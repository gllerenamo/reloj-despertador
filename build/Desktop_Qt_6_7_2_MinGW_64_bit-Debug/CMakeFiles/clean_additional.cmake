# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\reloj-despertador_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\reloj-despertador_autogen.dir\\ParseCache.txt"
  "reloj-despertador_autogen"
  )
endif()
