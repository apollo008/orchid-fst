add_subdirectory(test)

add_library(
        fst_core_lib SHARED
        fst.cpp
        large_file_sorter.cpp
        automaton.cpp
)

install(TARGETS
        fst_core_lib
        DESTINATION lib)

install(FILES
        fst.h
        automaton.h
        large_file_sorter.h
        DESTINATION include/common/fst)
