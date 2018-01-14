add_executable(example001 example001.cpp)
target_link_libraries(example001 ${PROJECT_NAME})
target_compile_options(example001 PRIVATE -fno-limit-debug-info)

add_executable(iterator_test iterator_test.cpp)
target_link_libraries(iterator_test ${PROJECT_NAME})

#https://cmake.org/cmake/help/v3.2/manual/cmake.1.html#command-line-tool-mode
add_custom_command(TARGET example001 iterator_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_SOURCE_DIR}/msft.csv
                "$<TARGET_PROPERTY:example001,RUNTIME_OUTPUT_DIRECTORY>")
