#include <iostream>
#include <regex>
#include <vector>
#include <iterator>
#include "lib/pg_lib/pg_connection.h"
#include "lib/pg_lib/exception/conn_info_nullpointer_exception.h"
#include "lib/pg_lib/pg_statement.h"
#include "lib/pg_lib/pg_resultset.h"
#include "lib/pg_lib/pg_prepared_statement.h"
#include <exception>


void regex_test(){
    using namespace std;
    regex pattern = regex(R"pattern(\w+(@)\w+\.com)pattern");
    string msg = "yh_swjtu@163.com是我一个人1193830957@qq.com";
    smatch match;
    cout << regex_match(msg, match, pattern) << endl;
    smatch search;
    cout << regex_search(msg, search, pattern) << endl;
    cout << search.str() << endl;
    cout << "=============================" << endl;
    sregex_iterator iter(msg.begin(), msg.end(), pattern);
    sregex_iterator end;
    for(; iter != end; iter ++){
        cout << iter->str() << endl;
    }
    cout << "=====================" << endl;
    sregex_token_iterator token_iter = sregex_token_iterator(msg.begin(), msg.end(), pattern);
    sregex_token_iterator token_end;
    for(; token_iter != token_end; token_iter ++){
        cout << token_iter->str() << " - " << *token_iter << endl;
    }
}

const char *parse(int num){
    using namespace std;
    vector<int> val;
    while(true){
        val.push_back(num % 10);
        num /= 10;
        if (!num){
            break;
        }
    }
    char *res = new char[val.size() + 1];
    for (int i = 0; i < val.size(); i ++){
        res[i] = val[val.size() - i - 1] + 48;
    }
    res[val.size()] = 0;
    return res;
}

void query_test(){
    using namespace std;
    pg_connection con("postgresql://ilmare@10.69.35.174/TPCD?connect_timeout=10&password=123456");
    cout << con << endl;
    try{
        con.set_auto_commit(false);
        string sql = "select l_orderkey,l_partkey,l_shipdate,l_commitdate,l_receiptdate,l_comment from lineitem where l_shipdate=$1 limit $2 offset $3";
        parameter_type types[] = {date_type, int_type, int_type};
        pg_prepared_statement st = con.prepared_statement(sql, types);
        st.set_value(0, "date '1994-10-24'");
        st.set_value(1, "10");
        st.set_value(2, "100");
        pg_resultset result = st.execute_query();
        con.commit();
        result.show_with_elegant_format();
    }catch(const exception &e){
        con.roll_back();
        cout << e.what() << endl;
    }
}

void update_test(){
    using namespace std;
    pg_connection con("postgresql://ilmare@10.69.35.174/TPCD?connect_timeout=10&password=123456");
    try{
        con.set_auto_commit(false);
//        std::string sql = "delete from nation where n_nationkey=$1";
//        parameter_type types[] = {int_type};
        std::string sql = "insert into nation(n_nationkey,n_name,n_regionkey,n_comment) values($1,$2,$3,$4)";
        parameter_type types[] = {int_type, text_type, int_type, text_type};
        pg_prepared_statement st = con.prepared_statement(sql, types);
        for(int i = 0; i < 10; i ++){
            const char *s = parse(25 + i);
            st.set_value(0, s);
            st.set_value(1, "'CANADA'");
            st.set_value(2, "0");
            st.set_value(3, "'eas hang ironic, silent packages. slyly regular packages are furiously over the tithes. fluffily bold'");
            st.execute_update();
            delete []s;
            if (i == 5){
                throw exception();
            }
        }
        con.commit();
    }catch(const exception &e){
        con.roll_back();
        cout << e.what() << endl;
    }
}

int main(int arg_n, char *arg_v[]) {
    using namespace std;
    query_test();
    return 0;
}