cmake_minimum_required(VERSION 3.15)
project(InteractiveOpticsSFML LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# Находим SFML
find_package(SFML 2.5 COMPONENTS system window graphics REQUIRED)

add_executable(interactive_optics src/main.cpp) # Пока файлы в корне src

# target_include_directories(interactive_optics PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}) # Убрано т.к. все в src

if(SFML_FOUND)
    target_link_libraries(interactive_optics PRIVATE sfml-graphics sfml-window sfml-system)
endif()