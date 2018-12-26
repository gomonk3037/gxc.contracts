add_contract(gxc.game gxc.game ${CMAKE_CURRENT_SOURCE_DIR}/src/gxc.game.cpp)
target_include_directories(gxc.game.wasm
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/../libraries/include
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(gxc.game.wasm
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
