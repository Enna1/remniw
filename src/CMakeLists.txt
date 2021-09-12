# Build the phase specific libraries
add_subdirectory(frontend)
add_subdirectory(semantic)
add_subdirectory(codegen)
add_subdirectory(optimizer)

# compiler binary target
add_executable(remniw ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
target_link_libraries(remniw PRIVATE frontend
                                     semantic
                                     ircodegen
                                    #  optimizer
                                     antlr4_static
                                     ${llvm_libs}
)
target_include_directories(remniw PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/frontend
                                          ${CMAKE_CURRENT_SOURCE_DIR}/semantic
                                          ${CMAKE_CURRENT_SOURCE_DIR}/codegen
                                          ${CMAKE_CURRENT_SOURCE_DIR}/optimizer
)