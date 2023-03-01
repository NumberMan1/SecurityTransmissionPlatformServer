#ifndef MYSQL_SECKEY_H
#define MYSQL_SECKEY_H

#include <mysqlx/xdevapi.h>
#include "struct_seckey.h"

namespace platform {

struct SelectInventory {
    std::string where;
    std::uint16_t limit{0};
};

class SeckeyMysql {
private:
    std::string url_{"mysqlx://"};
    std::string schema_name_;
public:
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
    std::list<mysqlx::Row> Select(const TableName &table_name);
    // 返回指定内容内容, 都是返回select *
    std::list<mysqlx::Row> Select(std::string_view table_name,
                                  SelectInventory &inventory);
    std::list<mysqlx::Row> Select(const TableName &table_name,
                                  SelectInventory &inventory);
    // 将对应表的行(没有null)插入表中
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
                                       mysqlx::Session &sess) const {
        switch (table_name) {
        case TableName::kSeckey_info:
            return sess.getSchema(schema_name_).getTable("seckey_info");
        case TableName::kSeckey_node:
            return sess.getSchema(schema_name_).getTable("seckey_node");
        }
    }
    inline void SelectSetImpl(mysqlx::TableSelect &table_select,
                              SelectInventory &inventory) const {
        if (!inventory.where.empty()) {
            table_select.where(inventory.where);
        }
        if (inventory.limit != 0) {
            table_select.limit(inventory.limit);
        }
    }
};
    
} // namespace mysql_seckey

#endif