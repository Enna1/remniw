# Build the phase specific libraries
add_subdirectory(frontend)
add_subdirectory(semantic)
add_subdirectory(codegen)
add_subdirectory(optimizer)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h @ONLY)

# compiler binary target
add_executable(remniw ${CMAKE_CURRENT_SOURCE_DIR}/driver.cpp)
target_link_libraries(remniw PRIVATE frontend
                                     semantic
                                     ircodegen
                                     asmcodegen
                                    #  optimizer
                                     antlr4_static
                                     ${llvm_libs}
)

# runtime
add_subdirectory(runtime)
