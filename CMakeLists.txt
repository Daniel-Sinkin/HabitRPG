cmake_minimum_required(VERSION 3.24)

project(HabitRPG VERSION 0.1.0 LANGUAGES CXX)

option(HABITRPG_BUILD_UI "Build the SDL3 + OpenGL3 Dear ImGui shell" ON)
option(HABITRPG_BUILD_TESTS "Build test executable" ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SQLITE3 REQUIRED IMPORTED_TARGET sqlite3)

set(HABITRPG_CORE_SOURCES
  src/app/startup_smoke.cpp
  src/domain/entities.cpp
  src/domain/interaction_flow.cpp
  src/domain/reward_engine.cpp
  src/domain/today_queue.cpp
  src/data/migrations.cpp
  src/data/sqlite_repository.cpp
  src/ui/runtime_resources.cpp
)

add_library(habitrpg_core STATIC ${HABITRPG_CORE_SOURCES})
target_include_directories(habitrpg_core PUBLIC include)
target_link_libraries(habitrpg_core PUBLIC PkgConfig::SQLITE3)
target_compile_features(habitrpg_core PUBLIC cxx_std_23)

if(HABITRPG_BUILD_UI)
  set(HABITRPG_IMGUI_SOURCES
    third_party/imgui/imgui.cpp
    third_party/imgui/imgui_draw.cpp
    third_party/imgui/imgui_tables.cpp
    third_party/imgui/imgui_widgets.cpp
    third_party/imgui/backends/imgui_impl_sdl3.cpp
    third_party/imgui/backends/imgui_impl_opengl3.cpp
  )

  set(HABITRPG_IMGUI_HEADERS
    third_party/imgui/imgui.h
    third_party/imgui/imgui_internal.h
    third_party/imgui/imconfig.h
    third_party/imgui/imstb_rectpack.h
    third_party/imgui/imstb_textedit.h
    third_party/imgui/imstb_truetype.h
    third_party/imgui/backends/imgui_impl_sdl3.h
    third_party/imgui/backends/imgui_impl_opengl3.h
    third_party/imgui/backends/imgui_impl_opengl3_loader.h
  )

  set(HABITRPG_UI_READY ON)
  foreach(imgui_file IN LISTS HABITRPG_IMGUI_SOURCES HABITRPG_IMGUI_HEADERS)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${imgui_file}")
      set(HABITRPG_UI_READY OFF)
      break()
    endif()
  endforeach()

  if(HABITRPG_UI_READY)
    pkg_check_modules(SDL3 REQUIRED IMPORTED_TARGET sdl3)
    find_package(OpenGL REQUIRED)

    add_library(habitrpg_imgui STATIC ${HABITRPG_IMGUI_SOURCES})
    target_include_directories(
      habitrpg_imgui
      PUBLIC
        third_party/imgui
        third_party/imgui/backends
    )
    target_link_libraries(habitrpg_imgui PUBLIC PkgConfig::SDL3 OpenGL::GL)

    add_executable(
      habitrpg_app
      src/main.cpp
      src/app/application.cpp
      src/ui/dockspace_shell.cpp
    )
    target_include_directories(habitrpg_app PRIVATE include third_party/imgui third_party/imgui/backends)
    target_link_libraries(habitrpg_app PRIVATE habitrpg_core habitrpg_imgui PkgConfig::SDL3 OpenGL::GL)
  else()
    message(WARNING
      "HABITRPG_BUILD_UI is ON but Dear ImGui sources are incomplete in third_party/imgui. "
      "Run scripts/fetch_imgui.sh and re-configure to build habitrpg_app.")
  endif()
endif()

if(HABITRPG_BUILD_TESTS)
  enable_testing()

  add_executable(
    habitrpg_tests
    tests/test_main.cpp
    tests/migration_tests.cpp
    tests/queue_tests.cpp
    tests/round3_tests.cpp
    tests/smoke_tests.cpp
    tests/state_transition_tests.cpp
    tests/roundtrip_tests.cpp
  )
  target_include_directories(habitrpg_tests PRIVATE include)
  target_link_libraries(habitrpg_tests PRIVATE habitrpg_core)

  add_test(NAME habitrpg_tests COMMAND habitrpg_tests)
endif()
