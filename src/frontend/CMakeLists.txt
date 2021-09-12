# call macro to add lexer and grammar to your build dependencies.
antlr_target(RemniwGrammar ${CMAKE_SOURCE_DIR}/grammar/Remniw.g4 LEXER PARSER VISITOR)

# include antlr generated files
include_directories(${ANTLR_RemniwGrammar_OUTPUT_DIR})

# define a library for the front end
add_library(frontend)
target_sources(frontend PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Type.h
                                ${CMAKE_CURRENT_SOURCE_DIR}/Type.cpp
                                ${CMAKE_CURRENT_SOURCE_DIR}/AST.h
                                ${CMAKE_CURRENT_SOURCE_DIR}/ASTBuilder.h
                                ${CMAKE_CURRENT_SOURCE_DIR}/ASTBuilder.cpp
                                ${CMAKE_CURRENT_SOURCE_DIR}/RecursiveASTVisitor.h
                                ${CMAKE_CURRENT_SOURCE_DIR}/ASTPrinter.h
                                ${CMAKE_CURRENT_SOURCE_DIR}/FrontEnd.h
                                ${CMAKE_CURRENT_SOURCE_DIR}/FrontEnd.cpp
                                ${ANTLR_RemniwGrammar_CXX_OUTPUTS}
)
target_include_directories(frontend PRIVATE ${ANTLR_RemniwGrammar_OUTPUT_DIR})
target_link_libraries(frontend PRIVATE antlr4_static)