#pragma once

#include "sqlite/sqlite3.h"
#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <mutex>
#include <thread>

class CSqliteHandler
{
public:
    friend class CSqliteTransactionHandler;
    using SQL_ROW_VALUE = std::map<std::string, std::string>;
    using SQL_VALUES = std::vector<SQL_ROW_VALUE>;
    CSqliteHandler(const std::string& dbPath, const std::vector<std::string>& tableNames);
    ~CSqliteHandler();

    bool OpenSqlDB();
    bool CreateTable(const std::string& sql, const std::string& tableName);

    bool Insert(const std::string& sql);
    bool Delete(const std::string& sql);//Delete some record(s)
    bool Update(const std::string& sql);
    bool SelectData(const std::string& sql, SQL_VALUES& values);
private:
    bool DeleteImpl(const std::string& sql, sqlite3* pDB);
    bool BeginTransaction();
    bool Commit();
    void Destory();
    bool SyncMemoryAndFileDatabase(bool fromMemoryToFile = true);
    bool AttachMemoryAndFileDatabase();
    bool DetachMemoryAndFileDatabase();

    const std::string m_filedbStr;
    const std::string m_dbPath;
    sqlite3* m_pMemoryDB;//memory database
    const std::vector<std::string> m_tableNames;
    bool m_running;
    std::recursive_mutex m_mtx;
    std::thread m_syncThread;
};

class CSqliteTransactionHandler
{
    CSqliteHandler& m_handler;
public:
    CSqliteTransactionHandler(CSqliteHandler& h) : m_handler(h) {
        m_handler.BeginTransaction();
    }
    ~CSqliteTransactionHandler() {
        m_handler.Commit();
    }
};
