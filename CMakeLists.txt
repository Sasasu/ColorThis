cmake_minimum_required(VERSION 3.9)

project(ColorThis)

add_library(LibColorThis SHARED libcolorthis.c)
target_link_libraries(LibColorThis dl)

add_executable(${PROJECT_NAME}
    main.c
    hook.c hook.h
    pty.c pty.h
    epoll.c epoll.h
)

install(TARGETS ColorThis LibColorThis
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
)

