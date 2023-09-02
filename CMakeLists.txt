cmake_minimum_required(VERSION 3.12)
project(diptool)

# compiler standard and other settings

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

# command scripts to build the libraries before compiling project

set(CLEAR_LIBS "")
if (${CLEAR_SFML_LIBS})
	set(CLEAR_LIBS "clear")
endif()

execute_process(
                WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
		COMMAND bash ${PROJECT_SOURCE_DIR}/scripts/win-mingw/build-sfml.sh ${CLEAR_LIBS}
               )

execute_process(
                WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
		COMMAND bash ${PROJECT_SOURCE_DIR}/scripts/win-mingw/build-imgui-sfml.sh ${CLEAR_LIBS}
               )

# location of libraries

set(SFML_STATIC_LIBRARIES TRUE)
set(SFML_DIR "${PROJECT_SOURCE_DIR}/libs/sfml/sfml-release/lib/cmake/SFML")
set(ImGui-SFML_DIR "${PROJECT_SOURCE_DIR}/libs/imgui-sfml/imgui-sfml-release/lib/cmake/ImGui-SFML")

find_package(Threads REQUIRED)
find_package(SFML 2 COMPONENTS system window graphics REQUIRED)
find_package(ImGui-SFML REQUIRED)

add_subdirectory(libs/cli11)
add_subdirectory(libs/spdlog)

# files needed to build project

add_executable(diptool
               main.cpp
              )

target_link_libraries(diptool
                      sfml-window
                      sfml-graphics
                      ImGui-SFML::ImGui-SFML
                      Threads::Threads
                      CLI11::CLI11
                      spdlog
                     )



