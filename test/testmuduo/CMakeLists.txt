#配置头文件搜索路径
#include_directories()

#配置库文件搜索路径
#link_directories()


#设置需要编译的源文件列表
set(SRC_LIST muduo_server.cpp)

#设置可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)


#生成可执行文件
add_executable(server ${SRC_LIST})

#指定可执行文件链接的库
target_link_libraries(server muduo_net muduo_base pthread)

