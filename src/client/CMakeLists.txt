#定义一个变量，保存当前目录下的所有源文件
aux_source_directory(. SRC_LIST)

#指定生成可执行文件
add_executable(chatClient ${SRC_LIST})

#指定可执行文件所依赖的库
target_link_libraries(chatClient  pthread)