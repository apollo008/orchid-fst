add_subdirectory(test)

add_library(
        common_util_lib SHARED
        file_util.cpp
        hash_util.cpp
        string_util.cpp
        time_util.cpp
        utf8_util.cpp
        output_stream_util.cpp
)

install(TARGETS
        common_util_lib
        DESTINATION lib)

install(FILES
        file_util.h
        hash_util.h
        lru_cache.h
        lfu_cache.h
        string_util.h
        time_util.h
        CLI11.hpp
        utf8_util.h
        output_stream_util.h
        DESTINATION include/common/util)
