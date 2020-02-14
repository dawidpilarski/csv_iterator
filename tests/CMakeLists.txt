project(csv_parser_tests)


add_executable(${PROJECT_NAME} test.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE third_party/)
target_link_libraries(${PROJECT_NAME} csv_parser)