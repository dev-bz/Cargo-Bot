cmake_minimum_required(VERSION 3.14.5)

project(Cargo)

#add_subdirectory(sfxr120)

add_library(phys STATIC phys.cpp)

target_include_directories(phys PUBLIC /sdcard/cargo-bot/lua-5.1.1/src)
target_link_directories(phys PUBLIC /sdcard/cargo-bot/lua-5.1.1/src)
target_include_directories(phys PRIVATE /sdcard/_ws/box2d_231/box2d/Box2D)
target_link_directories(phys PUBLIC /sdcard/_ws/box2d_231/box2d/Box2D/c.build/Box2D)
#target_link_options(phys PUBLIC -pie -Wl,--no-undefined)
target_link_libraries(phys PRIVATE lua box2d)


add_library(cargo SHARED cCode.c font.cpp sfxr.cpp loadpng.cpp pngtest.cpp b64/cdecode.c)
target_include_directories(cargo PRIVATE /sdcard/cargo-bot/lua-5.1.1/src)
target_link_directories(cargo PUBLIC /sdcard/cargo-bot/lua-5.1.1/src)
target_link_options(cargo PUBLIC -Wl,--no-undefined)
target_link_libraries(cargo PUBLIC native_app_glue EGL GLESv1_CM log android png z phys openal OpenSLES)
if(CIDE)
set(CMAKE_SHARED_LIBRARY_PREFIX)
set(CMAKE_SHARED_LIBRARY_SUFFIX .out)
set_target_properties(cargo PROPERTIES OUTPUT_NAME a)
else()
set_target_properties(cargo PROPERTIES OUTPUT_NAME box)
endif()


add_executable(cargos test.cpp )
target_link_options(cargos PUBLIC -pie)

target_include_directories(cargos PUBLIC /sdcard/cargo-bot/lua-5.1.1/src)
target_link_directories(cargos PUBLIC /sdcard/cargo-bot/lua-5.1.1/src)

target_link_libraries(cargos PUBLIC log lua)
if(CIDE)
set_target_properties(cargos PROPERTIES OUTPUT_NAME test.out)
else()
set_target_properties(cargos PROPERTIES OUTPUT_NAME a.out)
endif()
set(LIB true)
if(LIB)
install(TARGETS cargo DESTINATION $ENV{TMPDIR})
else()
install(TARGETS cargos DESTINATION $ENV{TMPDIR})
endif()
