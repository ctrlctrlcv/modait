cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

set(CMAKE_CXX_VISIBILITY_PRESET hidden)

# Cambia "OlkFPC" por el nombre de tu proyecto
project(OlkFPC VERSION 1.0.0)

# Añade todos los archivos fuente recursivamente desde src/
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

# Crea la librería del mod
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Verifica que la variable de entorno GEODE_SDK esté definida (la acción la configura por ti)
if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

# Añade el SDK de Geode como submódulo
add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

# Configura dependencias, recursos y enlaza Geode
setup_geode_mod(${PROJECT_NAME})
