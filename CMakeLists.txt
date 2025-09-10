cmake_minimum_required (VERSION 3.8)
project(Kida)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

add_executable(Kida src/Kida.cpp src/Kida.h)

add_subdirectory(external/miniupnp/miniupnpc EXCLUDE_FROM_ALL)
target_link_libraries(${PROJECT_NAME} PRIVATE libminiupnpc-static)
target_include_directories(${PROJECT_NAME} PRIVATE external/miniupnp/miniupnpc)

add_subdirectory(external/spdlog EXCLUDE_FROM_ALL)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog)
target_include_directories(${PROJECT_NAME} PRIVATE external/spdlog)

if(WIN32)
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif()

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Kida PROPERTY CXX_STANDARD 20)
endif()
