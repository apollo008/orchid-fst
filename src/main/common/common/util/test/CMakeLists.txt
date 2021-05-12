add_executable(
        CLI11_unittest_exe
        CLI11_unittest_exe.cpp
)

add_executable(
        cache_unittest_exe
        cache_unittest_exe.cpp
)

add_executable(
        cache_unittest
        cache_unittest.cpp
        ${DOTEST_CPP}
)

add_executable(
        common_util_unittest
        CLI11_unittest.cpp
        ${DOTEST_CPP}
        )

target_link_libraries(
        common_util_unittest
        common_util_lib
        tulip
        cppunit
        dl
        )

target_link_libraries(
        CLI11_unittest_exe
        cppunit
        dl
)

target_link_libraries(
        cache_unittest
        common_util_lib
        tulip
        cppunit
        dl
        pthread
)


target_link_libraries(
        cache_unittest_exe
        common_util_lib
        tulip
        cppunit
        dl
        pthread
)

install( TARGETS
        CLI11_unittest_exe
        cache_unittest_exe
        DESTINATION bin )


add_test(
        common_util_unittest
        common_util_unittest
)

add_test(
        cache_unittest
        cache_unittest
)

