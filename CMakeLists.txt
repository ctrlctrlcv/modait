cmake_minimum_required(VERSION 3.21)

# Definir el proyecto y el SDK de Geode
project(OlkFPC VERSION 1.0.0)

# Configurar Geode (esto descarga automáticamente el SDK si no está)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Incluir el sistema de build de Geode
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Geode.cmake)

# ============================================================
#  ARCHIVOS FUENTE DEL MOD
# ============================================================
set(SOURCES
    src/main.cpp
    src/OlkFPC.cpp
    src/FrameAnalyzer.cpp
    src/MacroParser.cpp
    src/PlaybackController.cpp
    src/RenderManager.cpp
    src/ui/OlkFPCUI.cpp
    src/ui/CirclesOverlay.cpp
    src/ui/RowsOverlay.cpp
    src/ui/InsightsUI.cpp
    src/utils/SoundManager.cpp
)

# Crear el target del mod
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Incluir directorios
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Enlazar con Geode
target_link_libraries(${PROJECT_NAME} PRIVATE Geode::Geode)

# ============================================================
#  CONFIGURACIÓN ESPECÍFICA POR PLATAFORMA
# ============================================================

if(WIN32)
    # Windows: no requiere nada especial
    message(STATUS "Building for Windows")
elseif(ANDROID)
    message(STATUS "Building for Android (${CMAKE_ANDROID_ARCH_ABI})")
elseif(APPLE)
    message(STATUS "Building for macOS/iOS")
endif()

# ============================================================
#  INSTALACIÓN Y EMPAQUETADO
# ============================================================
# Geode genera automáticamente el .geode al hacer build
geode_package()