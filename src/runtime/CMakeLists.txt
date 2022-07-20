set(CMAKE_BUILD_TYPE Debug)
file(GLOB My_Source_Files *.cpp)
add_executable(test1
    test1.cpp
    aphotic_shield_allocator.cpp
    aphotic_shield_interface.cpp
    aphotic_shield_allocator.cpp
    guarded_pool_allocator.cpp
    error_report.cpp
    options.cpp
    )

add_library(mylib SHARED
    aphotic_shield_interface.cpp
    aphotic_shield_allocator.cpp
    guarded_pool_allocator.cpp
    error_report.cpp
    options.cpp
)

add_executable(test2
    test2.cpp
)
target_link_libraries(test2 mylib)