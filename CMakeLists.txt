cmake_minimum_required(VERSION 3.0)
project(scroll-and-sigil)

set(LUA ${CMAKE_PREFIX_PATH}/lua-5.4.3/src_embed)

file(GLOB SOURCE src/*.c src/*.h)
file(GLOB LUA_SOURCE ${LUA}/*.c ${LUA}/*.h)

set(SDL2_INCLUDE ${CMAKE_PREFIX_PATH}/sdl2/include)
set(SDL2_LIBRARIES ${CMAKE_PREFIX_PATH}/sdl2/lib/x64)

include_directories(src)
include_directories(${LUA})

include_directories(${SDL2_INCLUDE})
link_directories(${SDL2_LIBRARIES})

add_executable(${PROJECT_NAME} ${SOURCE} ${LUA_SOURCE})
target_link_libraries(${PROJECT_NAME} SDL2main SDL2)
