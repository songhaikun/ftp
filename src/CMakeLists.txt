set(SOURCE_FILES
    task_worker.cpp
    task_manager.cpp
    command_handler.cpp
    data_handler.cpp
    user_info.cpp
    main.cpp
)


add_executable(ftpserver ${SOURCE_FILES})

target_include_directories(ftpserver PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(ftpserver ACE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)