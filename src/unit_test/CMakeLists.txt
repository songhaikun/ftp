set(SOURCE_FILES
    ../task_worker.cpp
    ../task_manager.cpp
    ../command_handler.cpp
    ../data_handler.cpp
    ../user_info.cpp 
    user_info_test.cpp
    command_test.cpp
    data_test.cpp
    main.cpp
)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
add_executable(servertest ${SOURCE_FILES})
target_include_directories(servertest PUBLIC ../../include)
target_link_libraries(servertest ACE)
target_link_libraries(servertest gtest)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)