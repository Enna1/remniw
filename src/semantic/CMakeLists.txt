add_library(semantic)

target_sources(
  semantic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/SymbolTable.h
                   ${CMAKE_CURRENT_SOURCE_DIR}/TypeAnalysis.h
                   ${CMAKE_CURRENT_SOURCE_DIR}/TypeAnalysis.cpp)

target_link_libraries(semantic PRIVATE frontend)
