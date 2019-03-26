//
// Created by Administrator on 2019/3/26.
//

#include "pg_prepared_statement.h"
#include "exception/statement_exception.h"
#include <regex>
#include <time.h>
#include <random>
#include <iostream>
#include "exception/sqlexecute_exception.h"

pg_prepared_statement::pg_prepared_statement(PGconn *conn,
        std::string &sql): pg_statement(conn) {
    int count = 0;
    std::regex pattern(R"pattern(\$[0-9]*)pattern");
    this->sql = sql;
    std::sregex_iterator iter(sql.begin(), sql.end(), pattern);
    std::sregex_iterator end;
    for (; iter != end; iter ++, count ++);
    if (count <= 0){
        throw statement_exception("The parameter count is invaild");
    }
    this->parameters_count = count;
    this->parameters = std::vector<char *>(count);
    this->param_type = std::vector<std::string>(count);
    srand((unsigned) time(NULL));
    long current_time = time(NULL) + rand();
    std::vector<int> tmp;
    while(true){
        int tmp_idx = current_time % 10;
        current_time /= 10;
        tmp.push_back(tmp_idx);
        if (current_time == 0){
            break;
        }
    }
    char *name = new char[tmp.size() + 1];
    int i = 0;
    for (std::vector<int>::reverse_iterator iter = tmp.rbegin(); iter != tmp.rend(); iter ++){
        name[i ++] = *iter + 65;
    }
    name[tmp.size()] = 0;
    this->prepared_name = std::string(name);
    this->prepare_prefix = std::string("prepare ") + this->prepared_name + std::string("(");
    delete []name;
}

void pg_prepared_statement::set_value(char *parameter,
        int idx, parameter_type type) {
    if (idx >= this->parameters_count){
        throw statement_exception("The idx is bigger than the biggest parameter count");
    }
    switch (type){
        case date_type:
            this->param_type[idx] = std::string("date");
            break;
        case int_type:
            this->param_type[idx] = std::string("int");
            break;
        case text_type:
            this->param_type[idx] = std::string("text");
            break;
        case numeric_type:
            this->param_type[idx] = std::string("numeric");
            break;
        case bool_type:
            this->param_type[idx] = std::string("bool");
            break;
        default:
            throw statement_exception("Invaild parameter type");
    }
    this->parameters[idx] = parameter;
}

void pg_prepared_statement::execute_update() {
    for (std::vector<std::string>::iterator iter = this->param_type.begin();
    iter != this->param_type.end(); iter ++){
        if ((iter + 1) == this->param_type.end()){
            this->prepare_prefix += (*iter);
        }else{
            this->prepare_prefix += ((*iter) + std::string(","));
        }
    }
    this->prepare_prefix += (std::string(") as ") + this->sql);
    PQexec(this->conn, this->prepare_prefix.c_str());
    std::string execute = std::string("execute ") + this->prepared_name + std::string("(");
    for (std::vector<char *>::iterator iter = this->parameters.begin();
         iter != this->parameters.end(); iter ++){
        if ((iter + 1) == this->parameters.end()){
            execute += std::string(*iter);
        }else{
            execute += (std::string(*iter) + std::string(","));
        }
    }
    execute += std::string(");");
    PGresult *res = PQexec(this->conn, execute.c_str());
    execute = std::string("deallocate prepare ") + this->prepared_name + std::string(";");
    PQexec(this->conn, execute.c_str());
    this->verify_sql_executeresult(PQresultStatus(res));
}

pg_resultset pg_prepared_statement::execute_query() {
    for (std::vector<std::string>::iterator iter = this->param_type.begin();
         iter != this->param_type.end(); iter ++){
        if ((iter + 1) == this->param_type.end()){
            this->prepare_prefix += (*iter);
        }else{
            this->prepare_prefix += ((*iter) + std::string(","));
        }
    }
    this->prepare_prefix += (std::string(") as ") + this->sql);
    PQexec(this->conn, this->prepare_prefix.c_str());
    std::string execute = std::string("execute ") + this->prepared_name + std::string("(");
    for (std::vector<char *>::iterator iter = this->parameters.begin();
         iter != this->parameters.end(); iter ++){
        if ((iter + 1) == this->parameters.end()){
            execute += std::string(*iter);
        }else{
            execute += (std::string(*iter) + std::string(","));
        }
    }
    execute += std::string(");");
    PGresult *result_set = PQexec(this->conn, execute.c_str());
    execute = std::string("deallocate prepare ") + this->prepared_name + std::string(";");
    PQexec(this->conn, execute.c_str());
    this->verify_sql_executeresult(PQresultStatus(result_set));
    return pg_resultset(result_set);
}


