add_library(ircodegen)

target_sources(
    ircodegen PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/IRCodeGenerator.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/IRCodeGenerator.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/IRCodeGeneratorImpl.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/IRCodeGeneratorImpl.cpp)

target_include_directories(
    ircodegen PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}/src/frontend)

target_link_libraries(ircodegen PRIVATE frontend)