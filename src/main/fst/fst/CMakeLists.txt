add_subdirectory(test)
add_subdirectory(fst_core)

add_executable(
        fst_core_main
        fst_core_main.cpp
        )
target_link_libraries(
        fst_core_main
        tulip
        )


install(FILES
        common.h
        DESTINATION include/fst)


install(TARGETS
        fst_core_main
        DESTINATION bin)



