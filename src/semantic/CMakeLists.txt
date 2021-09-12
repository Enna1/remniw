add_library(semantic)

target_sources(
  semantic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/SymbolTable.h
                   ${CMAKE_CURRENT_SOURCE_DIR}/TypeAnalysis.h
                   ${CMAKE_CURRENT_SOURCE_DIR}/TypeAnalysis.cpp)

target_include_directories(
  semantic
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
          ${CMAKE_SOURCE_DIR}/src/frontend)

target_link_libraries(semantic PRIVATE frontend)