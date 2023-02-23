cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE src ${PROJECT_SOURCE_DIR}/src "*.cpp" "*.cc")

add_library(mysrc STATIC ${src})

set(THREADS_PREFER_PTHREAD_FLAG ON)
# find_package(Threads REQUIRED)

# find_package(unofficial-mysql-connector-cpp CONFIG REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Sql) 

find_package(Boost 1.82 REQUIRED COMPONENTS )
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
message(Boost\ version: ${Boost_VERSION_STRING})
message(Boost\ include\ dir: ${Boost_INCLUDE_DIRS})
message(Boost\ library\ dir: ${Boost_LIBRARY_DIRS})

message("Found Boost Libraries:")
foreach(boost_lib IN LISTS Boost_LIBRARIES)
message(${boost_lib})
string(REGEX MATCH ".+/.*boost_([^-]+)-.+\.(lib|a)" boost_lib_name ${boost_lib})
set(boost_lib_name ${CMAKE_MATCH_1})
set(boost_target Boost::${boost_lib_name})
if(TARGET ${boost_target})
message(STATUS "Boost target found: " ${boost_target})
endif(TARGET ${boost_target})
endforeach(boost_lib)

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
include_directories(${OPENSSL_INCLUDE_DIRS})
message(STATUS "OpenSSL Found!")
endif()

message("----------- trying to find Jsoncpp-------------") 
find_package(jsoncpp CONFIG REQUIRED)

# find_library(Jsoncpp_LIBRARY 
# NAMES jsoncpp 
# PATHS ${Jsoncpp_PKGCONF_LIBRARY_DIRS}
# ) 
find_package(spdlog CONFIG REQUIRED)

set(DEP_LIBS
OpenSSL::Crypto OpenSSL::SSL  # modern
# ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY}  # old style
)
# link_libraries(openssl crypto)

target_link_libraries(mysrc PUBLIC Threads::Threads)
# target_link_libraries(mysrc PUBLIC event)
target_link_libraries(mysrc PUBLIC ${DEP_LIBS})
target_link_libraries(mysrc PUBLIC ${Boost_LIBRARIES})
target_link_libraries(mysrc PUBLIC rt) # for boost_interprocess
target_link_libraries(mysrc PUBLIC pthread) # for pthread
target_link_libraries(mysrc PUBLIC jsoncpp_object jsoncpp_static JsonCpp::JsonCpp)
target_link_libraries(mysrc PUBLIC spdlog::spdlog spdlog::spdlog_header_only)
target_link_libraries(mysrc PUBLIC protobuf protoc)
# target_link_libraries(mysrc PUBLIC unofficial::mysql-connector-cpp::connector)
target_link_libraries(mysrc PRIVATE Qt6::Sql)
target_link_libraries(mysrc PUBLIC resolv)
# target_link_libraries(mysrc PUBLIC mysqlcppconn8)
if(MSVC)
target_compile_options(mysrc PRIVATE /W4 /WX)
else()
target_compile_options(mysrc PRIVATE -Wall -Wextra -pedantic )#-Werror)
endif()