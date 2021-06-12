cmake_minimum_required(VERSION 3.14.5)

project(Cargo-bot)

#add_subdirectory(sfxr120)
set(LUA_INC $ENV{HOME}/usr/local/include)
set(LUA_LIB $ENV{HOME}/usr/local/lib)
add_library(phys STATIC phys.cpp)

target_include_directories(phys PUBLIC ${LUA_INC})
target_link_directories(phys PUBLIC ${LUA_LIB})
target_include_directories(phys PRIVATE /sdcard/_ws/box2d_231/box2d/Box2D)
target_link_directories(phys PUBLIC /sdcard/_ws/box2d_231/box2d/Box2D/c.build/Box2D)
#target_link_options(phys PUBLIC -pie -Wl,--no-undefined)
#target_link_libraries(phys PRIVATE lua_static box2d)

add_library(cargo_a STATIC font.cpp sfxr.cpp loadpng.cpp pngtest.cpp b64/cdecode.c)
target_link_options(cargo_a PUBLIC -Wl,--no-undefined)
target_link_libraries(cargo_a PUBLIC native_app_glue EGL GLESv1_CM log android png z phys box2d openal OpenSLES)


add_library(cargo SHARED cCode.c)
if(NOT CIDE)
target_compile_definitions(cargo PRIVATE RESOURCE_PATH="${PROJECT_SOURCE_DIR}")
endif()
target_include_directories(cargo PRIVATE ${LUA_INC})
target_link_directories(cargo PUBLIC ${LUA_LIB})
target_link_libraries(cargo PUBLIC cargo_a lua_static)
if(CIDE)
set(CMAKE_SHARED_LIBRARY_PREFIX)
set(CMAKE_SHARED_LIBRARY_SUFFIX .out)
set_target_properties(cargo PROPERTIES OUTPUT_NAME a)
else()
set_target_properties(cargo PROPERTIES OUTPUT_NAME box)
endif()

if(NOT CIDE)
add_library(cargo_rt SHARED cCode.c)
target_include_directories(cargo_rt PRIVATE ${LUA_INC})
target_link_directories(cargo_rt PUBLIC ${LUA_LIB})
target_link_libraries(cargo_rt PUBLIC cargo_a lua_static)
set_target_properties(cargo_rt PROPERTIES OUTPUT_NAME main)
endif()

add_executable(cargos test.cpp )
target_compile_options(cargos PUBLIC -DNDEBUG -std=c++17 -fPIC -fvisibility=hidden)
target_link_options(cargos PUBLIC -pie)

target_include_directories(cargos PUBLIC ${LUA_INC})
target_link_directories(cargos PUBLIC ${LUA_LIB})

target_link_libraries(cargos PUBLIC log lua_static)
if(CIDE)
set_target_properties(cargos PROPERTIES OUTPUT_NAME test.out)
install(TARGETS cargo DESTINATION $ENV{TMPDIR})
else()
set_target_properties(cargos PROPERTIES OUTPUT_NAME a.out)
#set(LIB true)
#if(LIB)
install(TARGETS cargo DESTINATION ${PROJECT_NAME})
#install(TARGETS cargo_rt DESTINATION ${PROJECT_NAME})
#else()
install(TARGETS cargos DESTINATION ${PROJECT_NAME})
#endif()
add_custom_command(TARGET cargo_rt POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory libs/arm64-v8a
	COMMAND ${CMAKE_STRIP} -s ${PROJECT_BINARY_DIR}/libmain.so -o libs/arm64-v8a/libmain.so
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	COMMENT "install Android library(s)")
endif()
