# Generate BrgTerm.cpp using preprocessor
add_custom_target(gen1 ALL DEPENDS BrgTerm.cpp)
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/BrgTerm.cpp
  COMMAND clang++ -xc++ -E -P ${CMAKE_CURRENT_SOURCE_DIR}/BrgTerm.gen -I${LLVM_INCLUDE_DIRS}  -o ${CMAKE_CURRENT_SOURCE_DIR}/BrgTerm.cpp
  COMMAND sed -i 's/_.*/\\U&/' ${CMAKE_CURRENT_SOURCE_DIR}/BrgTerm.cpp
)

# Generate AsmBuilderHelper.brg using sed
add_custom_target(gen2 ALL DEPENDS AsmBuilderHelper.brg)
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/AsmBuilderHelper.brg
  COMMAND sed "/# %term define begin/r ${CMAKE_CURRENT_SOURCE_DIR}/BrgTerm.cpp" ${CMAKE_CURRENT_SOURCE_DIR}/AsmBuilderHelper.gen > ${CMAKE_CURRENT_SOURCE_DIR}/AsmBuilderHelper.brg
  DEPENDS gen1
)

# Generate AsmBuilderHelper.cpp and AsmBuilderHelper.h using olive
add_custom_target(brg ALL DEPENDS AsmBuilderHelper.cpp AsmBuilderHelper.h)
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/AsmBuilderHelper.cpp ${CMAKE_CURRENT_SOURCE_DIR}/AsmBuilderHelper.h
  COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/olive
  ARGS ${CMAKE_CURRENT_SOURCE_DIR}/AsmBuilderHelper.brg
  DEPENDS olive gen2
)

add_library(asmcodegen)
target_sources(
    asmcodegen PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/AsmBuilderHelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AsmInstruction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AsmFunction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AsmContext.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/X86/X86AsmBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RISCV/RISCVAsmBuilder.cpp)

add_executable(remniw-llc ${CMAKE_CURRENT_SOURCE_DIR}/remniw-llc.cpp)
add_dependencies(remniw-llc brg)
target_link_libraries(remniw-llc PRIVATE asmcodegen ${llvm_libs})
