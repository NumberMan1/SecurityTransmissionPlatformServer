#include "mysql_seckey.h"
#include <json/json.h>
#include <fstream>
#include <iostream>
#include <filesystem>

using namespace platform;

SeckeyMysql::SeckeyMysql(std::string_view json_file) {
    Json::Value root{};
    std::ifstream is(json_file.data());
    is >> root;
    url_.append(root["user"].asString() + ':' +
                root["password"].asString() + '@' +
                root["hostname"].asString());
    schema_name_ = root["schema"].asString();
    is.close();
}

SeckeyMysql::SeckeyMysql(const DBInfo &db_info)
    : schema_name_{db_info.data_base} {
    url_.append(db_info.user_name + ':' + db_info.password + '@' + db_info.host_name);
}

std::list<mysqlx::Row> SeckeyMysql::Select(std::string_view table_name) {
    auto table = InitTableImpl(table_name);
    mysqlx::TableSelect table_select = table.select("*");
    mysqlx::RowResult row_result = table_select.execute();
    return row_result.fetchAll();
}

void SeckeyMysql::Insert(std::string_view table_name) {
    auto table = InitTableImpl(table_name);
    mysqlx::TableInsert table_insert = table.insert();

}