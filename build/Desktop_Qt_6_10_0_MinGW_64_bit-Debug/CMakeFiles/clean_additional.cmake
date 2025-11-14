# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\canvasDesign_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\canvasDesign_autogen.dir\\ParseCache.txt"
  "canvasDesign_autogen"
  )
endif()
