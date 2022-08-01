add_library(aphotic_shield STATIC
    aphotic_shield_interface.cpp
    aphotic_shield_allocator.cpp
    guarded_pool_allocator.cpp
    error_report.cpp
    options.cpp
)

target_compile_options(aphotic_shield PRIVATE -fno-exceptions -fno-sanitize=all)
