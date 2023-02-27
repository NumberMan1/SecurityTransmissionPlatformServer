#ifndef MYSQL_SECKEY_H
#define MYSQL_SECKEY_H

#include <mysqlx/xdevapi.h>
#include <iostream>
#include <tuple>
#include <chrono>

namespace platform {

// 形式为 xxxx-xx-xx
using Date = std::array<char, 11>;

struct DBInfo {
    std::string host_name;
    std::string user_name;
    std::string password;
    std::string data_base;
};

struct RowSeckeyInfo {
    std::array<char, 10> key_id;
    Date create_time;
    bool state;
    std::array<char, 5> client_id,
                        server_id;
    std::string seckey;
};

struct RowSeckeyNode {
    std::array<char, 5> id;
    std::string name;
    Date create_time;
    std::array<char, 13> authcode; // 授权码
    bool state;
    std::string node_desc; // 描述
};

class SeckeyMysql {
private:
    std::string url_{"mysqlx://"};
    std::string schema_name_;
public:
    enum class TableName {
        kSeckey_info,
        kSeckey_node,
    };
    explicit SeckeyMysql(std::string_view json_file);
    explicit SeckeyMysql(const DBInfo &db_info);
    inline std::string URL() const noexcept {
        return url_;
    }
    inline std::string SchemaName() const noexcept {
        return schema_name_;
    }
    inline void SetSchemaName(const std::string &name) noexcept {
        schema_name_ = name;
    }
    inline void SetUrl(const std::string &url) noexcept {
        url_ = url;
    }
    // 返回select * from table_name的内容
    std::list<mysqlx::Row> Select(std::string_view table_name);
    std::list<mysqlx::Row> Select(TableName table_name);
    template<TableName table_name, typename T>
    void Insert(T value) {
        mysqlx::Session sess{url_};
        mysqlx::Table table = InitTableImpl(table_name, sess);
        if constexpr(table_name == TableName::kSeckey_info) {
            static_assert(std::is_same_v<T, RowSeckeyInfo>,
                          "所尝试插入的类型不匹配");
            mysqlx::TableInsert insert = table.insert(
                "key_id", "create_time", "state",
                "client_id", "server_id",
                "seckey");
            insert.values(value.key_id.data(), value.create_time.data(),
                          value.state, value.client_id.data(),
                          value.server_id.data(), value.seckey);
            insert.execute();
        } else if constexpr(table_name == TableName::kSeckey_node) {
            static_assert(std::is_same_v<T, RowSeckeyNode>,
                          "所尝试插入的类型不匹配");
            mysqlx::TableInsert insert = table.insert(
                "id", "name", "create_time", "authcode",
                "state", "node_desc");
            insert.values(value.id.data(), value.name, value.create_time.data(),
                          value.authcode.data(), value.state,
                          value.node_desc);
            insert.execute();
        }
    }
private:
    inline mysqlx::Table InitTableImpl(const TableName &table_name,
                                       mysqlx::Session &sess) {
        switch (table_name) {
        case TableName::kSeckey_info:
            return sess.getSchema(schema_name_).getTable("seckey_info");
        case TableName::kSeckey_node:
            return sess.getSchema(schema_name_).getTable("seckey_node");
        }
    }
};
    
} // namespace mysql_seckey

#endif