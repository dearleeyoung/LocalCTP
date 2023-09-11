#pragma once

#include "sqlite/sqlite3.h"
#include <vector>
#include <map>
#include <string>
#include <mutex>

class CSqliteHandler
{
public:
    friend class CSqliteTransactionHandler;
    using SQL_ROW_VALUE = std::map<std::string, std::string>;
    using SQL_VALUES = std::vector<SQL_ROW_VALUE>;
    CSqliteHandler(const std::string& dbPath);
    ~CSqliteHandler();

    bool OpenSqlDB(const std::string& dbPath);
    bool CreateTable(const std::string& sql);

    bool Insert(const std::string& sql);
    bool Delete(const std::string& sql);
    bool Update(const std::string& sql);
    bool SelectData(const std::string& sql, SQL_VALUES& values);
private:
    bool BeginTransaction();
    bool Commit();
    void Destory();
    sqlite3* m_pDB;
    std::mutex m_mtx;
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