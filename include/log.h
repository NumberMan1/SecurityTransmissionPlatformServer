#ifndef LOG_HPP_
#define LOG_HPP_

#include <fstream>
#include <string_view>

#define LOG(_message) mine_log::input_log((_message), __FILE__, __func__, __LINE__)

namespace mine_log {
// 可以直接使用LOG(消息)来给日志文件输入信息
void input_log(const std::string_view &_message,
         const std::string_view &_filename,
         const std::string_view &_func,
         const int &_line);

}

// inline void LOG(const std::string_view &_message) {
//     mine_log::input_log(_message, __FILE__, __func__, __LINE__);
// }
#endif // ! Log_HPP_