# Generate tree.h using m4
add_custom_target(tree ALL DEPENDS tree.h)
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/tree.h
  COMMAND m4
  ARGS ${CMAKE_CURRENT_SOURCE_DIR}/tree.gen ${CMAKE_CURRENT_SOURCE_DIR}/x_array.gen ${CMAKE_CURRENT_SOURCE_DIR}/hash.gen ${CMAKE_CURRENT_SOURCE_DIR}/tree.int > ${CMAKE_CURRENT_SOURCE_DIR}/tree.h
)

# Generate gram.c using bison -y
FIND_PACKAGE(BISON REQUIRED)
add_custom_target(gram ALL DEPENDS gram.c)
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/gram.c
  COMMAND ${BISON_EXECUTABLE}
  ARGS -y ${CMAKE_CURRENT_SOURCE_DIR}/gram.y -o ${CMAKE_CURRENT_SOURCE_DIR}/gram.c
)


add_executable(olive ${CMAKE_CURRENT_SOURCE_DIR}/gram.c
                     ${CMAKE_CURRENT_SOURCE_DIR}/x_arrayc.c
                     ${CMAKE_CURRENT_SOURCE_DIR}/x_arrayp.c
                     ${CMAKE_CURRENT_SOURCE_DIR}/code.c
                     ${CMAKE_CURRENT_SOURCE_DIR}/iburg.c
)

add_dependencies(olive tree gram)