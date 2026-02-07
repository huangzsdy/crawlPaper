cmake_minimum_required(VERSION 3.15)
project(ScientificCrawler)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译器选项
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines-ts")
    endif()
endif()

# 查找依赖包
find_package(CURL REQUIRED)
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

# 查找或安装第三方库
# tomlplusplus
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/third_party/tomlplusplus)
    message(STATUS "Downloading tomlplusplus...")
    execute_process(
            COMMAND git clone https://github.com/marzer/tomlplusplus.git
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/third_party
    )
endif()

# pugixml
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/third_party/pugixml)
    message(STATUS "Downloading pugixml...")
    execute_process(
            COMMAND git clone https://github.com/zeux/pugixml.git
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/third_party
    )
endif()

# nlohmann_json
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/third_party/json)
    message(STATUS "Downloading nlohmann/json...")
    execute_process(
            COMMAND git clone https://github.com/nlohmann/json.git
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/third_party
    )
endif()

# 包含目录
include_directories(include)
include_directories(third_party/tomlplusplus/include)
include_directories(third_party/pugixml/src)
include_directories(third_party/json/include)

# 添加可执行文件
add_executable(scientific_crawler src/main.cpp)

# 添加源文件
file(GLOB_RECURSE SOURCES
        "src/*.cpp"
        "src/config/*.cpp"
        "src/network/*.cpp"
        "src/parser/*.cpp"
        "src/storage/*.cpp"
        "src/scheduler/*.cpp"
)

target_sources(scientific_crawler PRIVATE ${SOURCES})

# 链接库
target_link_libraries(scientific_crawler
        PRIVATE
        ${CURL_LIBRARIES}
        Threads::Threads
)

# 针对不同平台的特定设置
if(UNIX AND NOT APPLE)
    target_link_libraries(scientific_crawler PRIVATE dl)
endif()

if(WIN32)
    target_link_libraries(scientific_crawler PRIVATE ws2_32 winmm)
endif()

# 安装目标
install(TARGETS scientific_crawler
        RUNTIME DESTINATION bin
)

# 安装配置文件
install(DIRECTORY config/
        DESTINATION etc/scientific_crawler
        FILES_MATCHING PATTERN "*.toml"
)

# 测试配置（可选）
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

# 打包配置
set(CPACK_PACKAGE_NAME "scientific_crawler")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_CONTACT "developer@example.com")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libcurl4, libc6")
set(CPACK_RPM_PACKAGE_REQUIRES "libcurl, glibc")
include(CPack)