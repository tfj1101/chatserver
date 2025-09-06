#
aux_source_directory(. SRCS)
aux_source_directory(./db DB_SRCS)
aux_source_directory(./model MODEL_SRCS)
aux_source_directory(./redis REDIS_SRCS)

#生成可执行文件
add_executable(ChatServer ${SRCS} ${DB_SRCS} ${MODEL_SRCS} ${REDIS_SRCS})


#指定可执行文件所依赖的库
target_link_libraries(ChatServer muduo_net muduo_base mysqlclient pthread hiredis)