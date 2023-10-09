#include "CSqliteHandler.h"

#define CHECK_DB_VALID \
    if(nullptr == m_pDB){ return false; } \
    std::lock_guard<std::mutex> lck(m_mtx);

CSqliteHandler::CSqliteHandler(const std::string& dbPath)
    : m_pDB(nullptr)
{
    OpenSqlDB(dbPath);
}

CSqliteHandler::~CSqliteHandler()
{
    Destory();
}

void CSqliteHandler::Destory()
{
    if (nullptr != m_pDB)
    {
        sqlite3_close(m_pDB);
        m_pDB = nullptr;
    }
}

bool CSqliteHandler::OpenSqlDB(const std::string& dbPath)
{
    return SQLITE_OK == sqlite3_open(dbPath.c_str(), &m_pDB);
}

bool CSqliteHandler::CreateTable(const std::string& sql)
{
    CHECK_DB_VALID
    char* errMsg = nullptr;
    int ret = sqlite3_exec(m_pDB, sql.c_str(), nullptr, nullptr, &errMsg);
    if (errMsg)
    {
        sqlite3_free(errMsg);
    }
    return ret == SQLITE_OK;
}

//开启事务
bool CSqliteHandler::BeginTransaction()
{
    char* errMsg = nullptr;
    int ret = sqlite3_exec(m_pDB, "begin", nullptr, nullptr, &errMsg);
    if (errMsg)
    {
        sqlite3_free(errMsg);
    }
    return ret == SQLITE_OK;
}

//提交事务
bool CSqliteHandler::Commit()
{
    char* errMsg = nullptr;
    int ret = sqlite3_exec(m_pDB, "commit", nullptr, nullptr, &errMsg);
    if (errMsg)
    {
        sqlite3_free(errMsg);
    }
    return ret == SQLITE_OK;
}

bool CSqliteHandler::Insert(const std::string& sql)
{
    CHECK_DB_VALID
    if (sql.empty()) return false;

    char* errMsg = nullptr;
    int ret = sqlite3_exec(m_pDB, sql.c_str(), nullptr, nullptr, &errMsg);
    if (errMsg)
    {
        sqlite3_free(errMsg);
    }
    return ret == SQLITE_OK;
}

bool CSqliteHandler::Delete(const std::string& sql)
{
    CHECK_DB_VALID
    int nCols = 0;
    int nRows = 0;
    char** azResult = nullptr;
    char* errMsg = nullptr;
    int result = sqlite3_get_table(m_pDB, sql.c_str(), &azResult, &nRows, &nCols, &errMsg);
    if (result != SQLITE_OK)
    {
        return false;
    }
    if (azResult)
    {
        sqlite3_free_table(azResult);
    }
    if (errMsg)
    {
        sqlite3_free(errMsg);
    }
    return true;
}

bool CSqliteHandler::Update(const std::string& sql)
{
    CHECK_DB_VALID
    char* errMsg = nullptr;

    int ret = sqlite3_exec(m_pDB, sql.c_str(), nullptr, nullptr, &errMsg);
    if (errMsg)
    {
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
    const int result = sqlite3_get_table(m_pDB, sql.c_str(), &azResult, &nRows, &nCols, &errMsg);

    index = nCols;

    values.clear();
    for (int i = 0; i < nRows; ++i)
    {
        SQL_ROW_VALUE temp;
        for (int j = 0; j < nCols; ++j)
        {
            if (azResult[index] == nullptr)// maybe the result is null
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
        sqlite3_free(errMsg);
    }
    return result == SQLITE_OK;
}
