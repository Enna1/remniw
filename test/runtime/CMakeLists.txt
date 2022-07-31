configure_file(lit.site.cfg.py.in lit.site.cfg.py @ONLY)

add_custom_target(check-aphotic_shield
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/lit.driver.py "${CMAKE_CURRENT_BINARY_DIR}" -v
  DEPENDS aphotic_shield)
