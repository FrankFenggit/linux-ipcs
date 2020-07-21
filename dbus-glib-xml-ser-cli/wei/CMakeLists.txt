cmake_minimum_required (VERSION 2.8)

#项目名称
project(test)
set(CLI_NAME "client")
set(SER_NAME "server")
message(STATUS "CMAKE_PROJECT_NAME = ${CMAKE_PROJECT_NAME}")


#编译器参数
set(CMAKE_CXX_COMPILER g++ CACHE STRING "C++ compiler" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_compile_options(-fPIE -pie -fstack-protector-strong -Wl,-z,relro,-z,now,-z,noexecstack)

SET(CMAKE_BUILD_TYPE Debug)
add_definitions(-DDEBUG)

IF(CMAKE_SYSTEM_PROCESSOR MATCHES  x86 )  #平台适配宏
	add_definitions(-DPLATFORM_PC_X86)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES  aarch)
	add_definitions(-DPLATFORM_PC_AARCH)
endif()
message(STATUS "CMAKE_SYSTEM_PROCESSOR = ${CMAKE_SYSTEM_PROCESSOR}")

# 包匹配
find_package(Threads)
find_package(PkgConfig)
pkg_check_modules(DBUS_GLIB  dbus-glib-1)
pkg_check_modules(GLIB  glib-2.0)

find_package(DBus1) 

set(DBUS_GLIB_DENPEND_LIBS
    ${DBUS_GLIB_LIBRARIES} 
    ${GLIB_LIBRARIES} 
    ${DBus1_LIBRARY}
)
set(DBUS_GLIB_DENPEND_INCLUDE
    ${DBUS_GLIB_INCLUDE_DIRS} 
    ${GLIB_INCLUDE_DIRS} 
    ${DBus1_INCLUDE_DIRS}
)
message(STATUS "DBUS_GLIB_DENPEND_INCLUDE = ${DBUS_GLIB_DENPEND_INCLUDE}")
message(STATUS "DBUS_GLIB_DENPEND_LIBS = ${DBUS_GLIB_DENPEND_LIBS}")

# 头文件
include_directories(${DBUS_GLIB_DENPEND_INCLUDE})
include_directories("${PROJECT_SOURCE_DIR}/")


# 源文件
aux_source_directory(client CLIENT_SRCS)
aux_source_directory(server SERVER_SRCS)
aux_source_directory(server/object SERVER_OBJECT_SRCS)

# 指定库文件连接目录
link_directories(
    ${CMAKE_BINARY_DIR}
)

# 项目生成
add_executable(${CLI_NAME} 
    ${CLIENT_SRCS}
)
add_executable(${SER_NAME} 
    ${SERVER_SRCS}
    ${SERVER_OBJECT_SRCS}
)

target_link_libraries (${CLI_NAME}  
    ${DBUS_GLIB_DENPEND_LIBS}
)
target_link_libraries (${SER_NAME}  
    ${DBUS_GLIB_DENPEND_LIBS}
)

