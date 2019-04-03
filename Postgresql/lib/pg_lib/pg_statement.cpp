//
// Created by Administrator on 2019/3/25.
//

#include "pg_statement.h"
#include "exception/statement_exception.h"
#include "exception/sqlexecute_exception.h"
#include <iostream>

pg_statement::pg_statement(PGconn *conn) {
    if (conn == NULL){
        throw statement_exception("The PGconn is NULL");
    }
    this->conn = conn;
}

void pg_statement::execute_update(const std::string &sql) {
    this->sql = sql;
    PGresult *result_set = PQexec(this->conn, this->sql.c_str());
    this->verify_sql_executeresult(result_set);
    PQclear(result_set);
}

pg_resultset pg_statement::execute_query(const std::string &sql) {
    this->sql = sql;
    PGresult *result_set = PQexec(this->conn, this->sql.c_str());
    this->verify_sql_executeresult(result_set);
    return pg_resultset(result_set);
}

void pg_statement::verify_sql_executeresult(PGresult *res) {
    ExecStatusType type = PQresultStatus(res);
    switch (type){
        case PGRES_EMPTY_QUERY:
            std::cerr << PQresultErrorMessage(res) << std::endl;
            throw sqlexecute_exception(type);
        case PGRES_BAD_RESPONSE:
            std::cerr << PQresultErrorMessage(res) << std::endl;
            throw sqlexecute_exception(type);
        case PGRES_NONFATAL_ERROR:
            std::cerr << PQresultErrorMessage(res) << std::endl;
            throw sqlexecute_exception(type);
        case PGRES_FATAL_ERROR:
            std::cerr << PQresultErrorMessage(res) << std::endl;
            throw sqlexecute_exception(type);
        default:
            break;
    }
}

