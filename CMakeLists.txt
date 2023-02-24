cmake_minimum_required(VERSION 3.2)

SET(CMAKE_TOOLCHAIN_FILE "~/vcpkg/scripts/buildsystems/vcpkg.cmake")

project(SecurityTransmissionPlatformServer VERSION 1.0 LANGUAGES CXX)

include(CTest)
enable_testing()


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pg -O2")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)


# if(OPENSSL_FOUND)
#   include_directories(${OPENSSL_INCLUDE_DIRS})
#   message(STATUS "OpenSSL Found!")
# endif()

# set(DEP_LIBS
#     OpenSSL::Crypto OpenSSL::SSL  # modern
#     # ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY}  # old style
# )


include_directories(${PROJECT_SOURCE_DIR}/include)
add_subdirectory(${PROJECT_SOURCE_DIR}/src)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(CPack)

add_executable(${PROJECT_NAME} main.cpp ${src})
target_link_libraries(${PROJECT_NAME} PRIVATE mysrc)
# target_link_libraries(${PROJECT_NAME} PRIVATE openssl)
# target_link_libraries(${PROJECT_NAME} PRIVATE event)
# target_link_libraries(${PROJECT_NAME} PRIVATE ${DEP_LIBS})
# # target_link_libraries(${PROJECT_NAME} PRIVATE jsoncpp_object jsoncpp_static JsonCpp::JsonCpp)
# target_link_libraries(${PROJECT_NAME} PRIVATE protobuf)
# target_link_libraries(${PROJECT_NAME} PRIVATE protoc)