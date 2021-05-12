add_subdirectory(test)
add_subdirectory(util)

add_executable(
        common_main
        common_main.cpp
        )
target_link_libraries(
        common_main
        tulip
        )

install(FILES
        common.h
        DESTINATION include/common)

install(TARGETS
        common_main
        DESTINATION bin)



