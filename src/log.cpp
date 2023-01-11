#include "log.h"

void mine_log::input_log(const std::string_view &_message,
         const std::string_view &_filename,
         const std::string_view &_func,
         const int &_line) {
    using OpenMode = std::ios_base::openmode;
    std::ofstream log("./d.log", OpenMode::_S_app);
    log << "info: " << _filename << ':' << _line << '\n'
        << _func << ": " << _message << '\n';
}