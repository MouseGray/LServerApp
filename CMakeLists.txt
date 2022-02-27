cmake_minimum_required(VERSION 3.5)

project(ServeAppX LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Boost_INCLUDE_DIR D:/boost_1_75_0)
set(Boost_LIBRARY_DIR D:/boost_1_75_0/stage/lib)

set(PSQL_INCLUDE_DIR C:/Users/Admin/Documents/master_x64Unicode/include)
set(PSQL_LIBRARY_DIR D:/libpqcxx/libpqxx/src/Debug)

set(LConfig_INCLUDE_DIR C:/Users/Admin/Documents/LConfig)
set(LConfig_LIB_DIR C:/Users/Admin/Documents/build-LConfig-Desktop_Qt_5_15_1_MSVC2019_64bit-Debug)

set(LFormat_INCLUDE_DIR C:/Users/Admin/Documents/LFormat)
set(LFormat_LIB_DIR C:/Users/Admin/Documents/build-LFormat-Desktop_Qt_5_15_1_MSVC2019_64bit-Debug)

set(Brotli_INCLUDE_DIR C:/Users/Admin/Documents/ServerX/Brotli/include)

set(LVector_DIR		C:/Users/Admin/Documents/LTools/LVector)
set(LVector_INCLUDE_DIR ${LVector_DIR}/include)
set(LVector_LIB_DIR	${LVector_DIR}/lib)

include_directories(${Boost_INCLUDE_DIR} ${Brotli_INCLUDE_DIR} ${PSQL_INCLUDE_DIR}
    "C:/Program Files/PostgreSQL/12/include"
    C:/Users/Admin/Documents/NodeLib
    C:/Users/Admin/Documents/LExpression D:/JSONParser/rapidjson/include
    C:/Users/Admin/Documents/LLoggerLib
    ${LConfig_INCLUDE_DIR}
    ${LFormat_INCLUDE_DIR}
    ${LVector_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR} ${PSQL_LIBRARY_DIR}
    "C:/Program Files/PostgreSQL/12/lib"
    #C:/Users/Admin/Desktop/brotli
    C:/Users/Admin/Documents/NodeLib
    ${LConfig_LIB_DIR}
    ${LFormat_LIB_DIR}
    ${LVector_LIB_DIR})

add_executable(ServeAppX
    main.cpp
    server.cpp
    server.h
    worker.cpp
    worker.h
    service.cpp
    service.h
    ltask.cpp
    ltask.h
    database.cpp
    database.h
    jsonutils.cpp
    jsonutils.h
    usertask.cpp
    usertask.h
    utils.cpp
    utils.h
)

target_link_libraries(ServeAppX
    ${Boost_LIBRARIES}
    "C:/Program Files/PostgreSQL/12/lib/libpq.lib"
    D:/libpqcxx/libpqxx/src/Debug/pqxx.lib
    #C:/Users/Admin/Desktop/brotli/brotlienc.lib
    C:/Users/Admin/Documents/build-NodeLib-Clangxx-Debug/NodeLib.lib
    C:/Users/Admin/Documents/LExpression/LExpression.lib
    ${LConfig_LIB_DIR}/LConfig.lib
    ${LFormat_LIB_DIR}/LFormat.lib
    ${LVector_LIB_DIR}/LVector.lib)
