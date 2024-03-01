#include "CSqliteHandler.h"
#include <iostream>

#define CHECK_DB_VALID \
    if(nullptr == m_pMemoryDB ){ return false; } \
    std::lock_guard<std::recursive_mutex> lck(m_mtx);

CSqliteHandler::CSqliteHandler(const std::string& dbPath,
    const std::vector<std::string>& tableNames)
    : m_filedbStr("filedb"), m_dbPath(dbPath)
    , m_pMemoryDB(nullptr)
    , m_tableNames(tableNames), m_running(true)
{
    OpenSqlDB();
    m_syncThread = std::thread([this]() {
        size_t count = 0;
        while (m_running)
        {
            if (++count >= 10)
            {
                count = 0;
                SyncMemoryAndFileDatabase();
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        // TODO: the below part will actually not be run in Windows, do you guys know why?
        std::cout << "Quit the CSqliteHandler, let's sync for the last time...!" << std::endl;
        SyncMemoryAndFileDatabase(); //sync once more before exit
    });
}

CSqliteHandler::~CSqliteHandler()
{
    m_running = false;
    if (m_syncThread.joinable())
    {
        m_syncThread.join();
    }
#ifdef _WIN32
    //sync once more before exit, because some code in the thread func is not run actually in Windows, so put it here
    std::cout << "Quit the CSqliteHandler in Windows, let's sync for the last time...!" << std::endl;
    SyncMemoryAndFileDatabase();
#endif
    Destory();
}

bool CSqliteHandler::SyncMemoryAndFileDatabase(bool fromMemoryToFile /*= true*/)
{
    CHECK_DB_VALID

    if (!AttachMemoryAndFileDatabase())
    {
        return false;
    }
    char* errMsg = nullptr;
    for (const auto& tableName : m_tableNames)
    {
        const std::string targetTable = fromMemoryToFile ? (m_filedbStr + "." + tableName) : tableName;
        const std::string srcTable = fromMemoryToFile ? tableName : (m_filedbStr + "." + tableName);

        const std::string syncSql = "INSERT OR REPLACE INTO " + targetTable
            + " SELECT * FROM " + srcTable + ";";
        int ret = sqlite3_exec(m_pMemoryDB, syncSql.c_str(), 0, 0, &errMsg);
        if (errMsg)
        {
            std::cerr << "Sync memory and file database failed! sql:" << syncSql
                << " errMsg:" << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
        if (SQLITE_OK != ret)
        {
            DetachMemoryAndFileDatabase();
            return false;
        }
    }
    if (!DetachMemoryAndFileDatabase())
    {
        return false;
    }

    return true;
}

void CSqliteHandler::Destory()
{
    if (nullptr != m_pMemoryDB)
    {
        sqlite3_close(m_pMemoryDB);
        m_pMemoryDB = nullptr;
    }
}

bool CSqliteHandler::OpenSqlDB()
{
    sqlite3* pDB(nullptr);//file database
    int ret = sqlite3_open(m_dbPath.c_str(), &pDB);// open file database, will create if not exist
    if (SQLITE_OK != ret)
    {
        std::cerr << "Open file database failed! dbPath:" << m_dbPath << std::endl;
        return false;
    }
    if (nullptr != pDB)
    {
        sqlite3_close(pDB);
        pDB = nullptr;
    }

    ret = sqlite3_open(":memory:", &m_pMemoryDB);// open memory database
    if (SQLITE_OK != ret)
    {
        std::cerr << "Open memory database failed! It shoule be attached with "
            << m_dbPath << std::endl;
        return false;
    }

    //const std::string pragmaSql = "PRAGMA cache_size = 10000;";//10 MB cache
    //char* errMsg = nullptr;
    //ret = sqlite3_exec(m_pMemoryDB, pragmaSql.c_str(), 0, 0, &errMsg);
    //if (errMsg)
    //{
    //    std::cerr << "PRAGMA something for file database " << m_dbPath << " failed! "
    //        << errMsg << std::endl;
    //    sqlite3_free(errMsg);
    //}
    //if (SQLITE_OK != ret)
    //{
    //    return false;
    //}

    std::cout << "OpenSqlDB done!~" << std::endl;
    return true;
}

bool CSqliteHandler::AttachMemoryAndFileDatabase()
{
    // attach the file database into the memory database connection,
    // otherwise we can not handle file database in the memory database connection,
    // because the sqlite forbids the cross-database operation if they are not attached,
    // and, the sql operation will be sync in these two database if they are attached
    char* errMsg = nullptr;
    const std::string attachSql = "ATTACH '" + m_dbPath + "' AS '" + m_filedbStr + "';";
    int ret = sqlite3_exec(m_pMemoryDB, attachSql.c_str(), 0, 0, &errMsg);
    if (errMsg)
    {
        std::cerr << "Attach memory and file database " << m_dbPath << " failed! "
            << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    if (SQLITE_OK != ret)
    {
        return false;
    }
    return true;
}

bool CSqliteHandler::DetachMemoryAndFileDatabase()
{
    // the sql operation will be sync in these two database if they are attached,
    // so we need detach them when we do not want to write to file database.
    char* errMsg = nullptr;
    const std::string detachSql = "DETACH DATABASE '" + m_filedbStr + "';";
    int ret = sqlite3_exec(m_pMemoryDB, detachSql.c_str(), 0, 0, &errMsg);
    if (errMsg)
    {
        std::cerr << "Detach memory and file database failed! " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    if (SQLITE_OK != ret)
    {
        return false;
    }
    return true;
}

bool CSqliteHandler::CreateTable(const std::string& sql, const std::string& tableName)
{
    CHECK_DB_VALID
    char* errMsg = nullptr;
    int ret = 0;

    // the first step: create table in the file database.
    // because if the table does not exist in file database,
    // it can not sync from file database to memory database.
    // of course, if the table does exist already, we do not need this step.
    sqlite3* pDB(nullptr);//file database
    ret = sqlite3_open(m_dbPath.c_str(), &pDB);
    if (SQLITE_OK != ret)
    {
        std::cerr << "Open file database failed! dbPath:" << m_dbPath << std::endl;
        return false;
    }
    ret = sqlite3_exec(pDB, sql.c_str(), nullptr, nullptr, &errMsg);
    if (errMsg)
    {
        char* ptr = ::strstr(errMsg, "already exists");
        if (ptr != NULL)
        {
            // table already exists, not deem it as an error
            ret = SQLITE_OK;
        }
        else
        {
            std::cerr << "Create table in file database failed! sql:" << sql
                << " errMsg:" << errMsg << std::endl;
        }
        sqlite3_free(errMsg);
    }
    if (nullptr != pDB)
    {
        sqlite3_close(pDB);
        pDB = nullptr;
    }
    if (SQLITE_OK != ret)
    {
        return false;
    }

    // the second step: create table in the memory database.
    ret = sqlite3_exec(m_pMemoryDB, sql.c_str(), nullptr, nullptr, &errMsg);
    if (errMsg)
    {
        std::cerr << "Create table in memory database failed! sql:" << sql
            << " errMsg:" << errMsg << std::endl;
        sqlite3_free(errMsg);
    }

    // the third step: attach the memory and file database,
    // and read table data from the existing table in file database
    // into memory database, finally we should detach them.
    if (!AttachMemoryAndFileDatabase())
    {
        return false;
    }
    const std::string targetTable = tableName;
    const std::string srcTable = m_filedbStr + "." + tableName;
    const std::string syncSql = "INSERT OR REPLACE INTO " + targetTable
        + " SELECT * FROM " + srcTable + ";";

    ret = sqlite3_exec(m_pMemoryDB, syncSql.c_str(), 0, 0, &errMsg);
    if (errMsg)
    {
        std::cerr << "Sync memory and file database on table " << tableName << " failed!"
            << " sql:" << syncSql
            << " errMsg:" << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    if (!DetachMemoryAndFileDatabase())
    {
        return false;
    }
    if (SQLITE_OK != ret)
    {
        return false;
    }

    return ret == SQLITE_OK;
}

// begin the transaction
bool CSqliteHandler::BeginTransaction()
{
    if (nullptr == m_pMemoryDB) { return false; }
    m_mtx.lock();
    char* errMsg = nullptr;
    int ret = sqlite3_exec(m_pMemoryDB, "begin", nullptr, nullptr, &errMsg);
    if (errMsg)
    {
        std::cerr << "Begin transaction in memory database failed!"
            << " errMsg:" << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    return ret == SQLITE_OK;
}

// commit the transaction
bool CSqliteHandler::Commit()
{
    char* errMsg = nullptr;
    int ret = sqlite3_exec(m_pMemoryDB, "commit", nullptr, nullptr, &errMsg);
    if (errMsg)
    {
        std::cerr << "Commit in memory database failed!"
            << " errMsg:" << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    m_mtx.unlock();
    return ret == SQLITE_OK;
}

bool CSqliteHandler::Insert(const std::string& sql)
{
    CHECK_DB_VALID
    if (sql.empty()) return false;

    char* errMsg = nullptr;
    int ret = sqlite3_exec(m_pMemoryDB, sql.c_str(), nullptr, nullptr, &errMsg);
    if (errMsg)
    {
        std::cerr << "Insert in memory database failed! sql:" << sql
            << " errMsg:" << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    return ret == SQLITE_OK;
}

bool CSqliteHandler::Delete(const std::string& sql)
{
    // How to handle deletion operations ?
    // First method : Synchronous operation, perform deletion in both the file databaseand the memory database.
    // Second method : Similar to the insertion operation, delete only in the memory database first,
    // and then in the timed task(SyncMemoryAndFileDatabase), clear all the data from the tables in the file database,
    // and then insert the content of each table in the memory database into the tables in the file database.
    // It is also possible to add a marker to the table to indicate that the table in the file database needs
    // to perform a clearand then insert operation.If there is no marker, it is not necessary to clear it first.
    //
    // This system uses the first method.

    // delete from table in file database, then delete from table in the memory database.
    sqlite3* pFileDB(nullptr);//file database
    int ret = sqlite3_open(m_dbPath.c_str(), &pFileDB);// open file database, will create if not exist
    if (SQLITE_OK != ret)
    {
        std::cerr << "Open file database in Delete() failed! dbPath:" << m_dbPath << std::endl;
        return false;
    }
    if (nullptr != pFileDB)
    {
        DeleteImpl(sql, pFileDB);

        sqlite3_close(pFileDB);
        pFileDB = nullptr;
    }

    return DeleteImpl(sql, m_pMemoryDB);
}

bool CSqliteHandler::DeleteImpl(const std::string& sql, sqlite3* pDB)
{
    CHECK_DB_VALID
    char* errMsg = nullptr;
    int result = sqlite3_exec(pDB, sql.c_str(), nullptr, nullptr, &errMsg);
    if (errMsg)
    {
        std::cerr << "Delete in " << (pDB == m_pMemoryDB ? "memory" : "file" )
            << " database failed! sql:" << sql
            << " errMsg:" << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    if (result != SQLITE_OK)
    {
        return false;
    }
    return true;
}


bool CSqliteHandler::Update(const std::string& sql)
{
    CHECK_DB_VALID
    char* errMsg = nullptr;

    int ret = sqlite3_exec(m_pMemoryDB, sql.c_str(), nullptr, nullptr, &errMsg);
    if (errMsg)
    {
        std::cerr << "Update in memory database failed! sql:" << sql
            << " errMsg:" << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    return ret == SQLITE_OK;
}

bool CSqliteHandler::SelectData(const std::string& sql, SQL_VALUES& values)
{
    CHECK_DB_VALID
    if (sql.empty()) return false;

    int nCols = 0;
    int nRows = 0;
    char** azResult = nullptr;
    char* errMsg = nullptr;
    int index = 0;
    const int result = sqlite3_get_table(m_pMemoryDB, sql.c_str(), &azResult, &nRows, &nCols, &errMsg);

    index = nCols;

    values.clear();
    for (int i = 0; i < nRows; ++i)
    {
        SQL_ROW_VALUE temp;
        for (int j = 0; j < nCols; ++j)
        {
            // maybe the result is null, TODO: maybe set as other value to present null?
            if (azResult[index] == nullptr)
            {
                temp[azResult[j]] = "";
            }
            else
            {
                temp[azResult[j]] = azResult[index];
            }
            index++;
        }
        values.push_back(temp);
    }

    if (azResult)
    {
        sqlite3_free_table(azResult);
    }
    if (errMsg)
    {
        std::cerr << "Select in memory database failed! sql:" << sql
            << " errMsg:" << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    return result == SQLITE_OK;
}
