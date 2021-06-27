cmake_minimum_required(VERSION 3.0)
project(scroll-and-sigil)

file(GLOB SOURCE src/*.c src/*.h)

set(SDL2_INCLUDE ${CMAKE_PREFIX_PATH}/sdl2/include)
set(SDL2_LIBRARIES ${CMAKE_PREFIX_PATH}/sdl2/lib/x64)

include_directories(src)

include_directories(${SDL2_INCLUDE})
link_directories(${SDL2_LIBRARIES})

if (MSVC)
  add_compile_options(/W4 /WX /wd4996)
else()
  add_compile_options(-Wall -Wextra -Werror -pedantic -std=c11)
endif()

add_executable(${PROJECT_NAME} ${SOURCE})
target_link_libraries(${PROJECT_NAME} SDL2main SDL2)
