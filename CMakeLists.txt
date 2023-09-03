cmake_minimum_required(VERSION 3.15)
project(diptool)

# compiler standard and other settings

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

# command scripts to build the libraries before compiling project

set(CLEAR_LIBS "")
if (${CLEAR_SFML_LIBS})
	set(CLEAR_LIBS "clear")
endif()

message(${WIN32})

if (DEFINED WIN32 AND (DEFINED MSYS OR DEFINED MINGW))
message("Using MSYS")

execute_process(
                WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
		COMMAND bash ${PROJECT_SOURCE_DIR}/scripts/win-mingw/build-sfml.sh ${CLEAR_LIBS}
               )

execute_process(
                WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
		COMMAND bash ${PROJECT_SOURCE_DIR}/scripts/win-mingw/build-imgui-sfml.sh ${CLEAR_LIBS}
               )

elseif(DEFINED WIN32 AND DEFINED MSVC)
message("Using Visual Studio")

execute_process(
                WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
		COMMAND powershell ${PROJECT_SOURCE_DIR}/scripts/win-vs/build-sfml.ps1 ${CLEAR_LIBS}
               )

execute_process(
                WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
		COMMAND powershell ${PROJECT_SOURCE_DIR}/scripts/win-vs/build-imgui-sfml.ps1 ${CLEAR_LIBS}
               )

else()

message("No supported platform available!")
return(-1)

endif()

# location of libraries

set(SFML_STATIC_LIBRARIES TRUE)

if (DEFINED WIN32 AND (DEFINED MSYS OR DEFINED MINGW))

set(SFML_DIR "${PROJECT_SOURCE_DIR}/libs/sfml/sfml-release/lib/cmake/SFML")
set(ImGui-SFML_DIR "${PROJECT_SOURCE_DIR}/libs/imgui-sfml/imgui-sfml-release/lib/cmake/ImGui-SFML")

elseif(DEFINED WIN32 AND DEFINED MSVC)

set(SFML_DIR "${PROJECT_SOURCE_DIR}/libs/sfml/sfml-vs-release/lib/cmake/SFML")
set(ImGui-SFML_DIR "${PROJECT_SOURCE_DIR}/libs/imgui-sfml/imgui-sfml-vs-release/lib/cmake/ImGui-SFML")

else()

message("No supported platform available!")
return(-1)

endif()


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



