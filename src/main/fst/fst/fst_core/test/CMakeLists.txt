add_executable(
        ofst
        fst_cmd.cpp
        )

add_executable(
        lfsort
        large_file_sort_cmd.cpp
)

add_executable(
        fst_unittest
        fst_unittest.cpp
        ${DOTEST_CPP}
        )

add_executable(
        large_file_sorter_unittest
        large_file_sorter_unittest.cpp
        ${DOTEST_CPP}
)

target_link_libraries(
        large_file_sorter_unittest
        common_util_lib
        fst_core_lib
        tulip
        cppunit
        pthread
        dl
)

target_link_libraries(
        fst_unittest
        common_util_lib
        fst_core_lib
        tulip
        cppunit
        pthread
        dl
        )

target_link_libraries(
        ofst
        common_util_lib
        fst_core_lib
        tulip
        cppunit
        dl
        )

target_link_libraries(
        lfsort
        common_util_lib
        fst_core_lib
        tulip
        dl
)

install( TARGETS
        lfsort
        ofst
        DESTINATION bin )

add_test(
        fst_unittest
        fst_unittest
)
add_test(
        large_file_sorter_unittest
        large_file_sorter_unittest
)
