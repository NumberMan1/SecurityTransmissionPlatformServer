#ifndef MYSQL_SECKEY_H
#define MYSQL_SECKEY_H

#include <mysqlx/xdevapi.h>

namespace platform {

struct DBInfo {
    std::string host_name;
    std::string user_name;
    std::string password;
    std::string data_base;
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
    void Insert(std::string_view table_name);
private:
    inline mysqlx::Table InitTableImpl(std::string_view table_name) {
        mysqlx::Session sess{url_};
        return sess.getSchema(schema_name_).getTable(table_name.data());
    }
};
    
} // namespace mysql_seckey

#endif