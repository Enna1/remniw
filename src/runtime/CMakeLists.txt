set(CMAKE_BUILD_TYPE Debug)

add_executable(test1
    test1.cpp
    aphotic_shield/aphotic_shield_interface.cpp
    aphotic_shield/aphotic_shield_allocator.cpp
    aphotic_shield/guarded_pool_allocator.cpp
    aphotic_shield/error_report.cpp
    aphotic_shield/options.cpp
    )

add_library(aphotic_shield SHARED
    aphotic_shield/aphotic_shield_interface.cpp
    aphotic_shield/aphotic_shield_allocator.cpp
    aphotic_shield/guarded_pool_allocator.cpp
    aphotic_shield/error_report.cpp
    aphotic_shield/options.cpp
)

add_executable(test2
    test2.cpp
)

target_link_libraries(test2 mylib)
