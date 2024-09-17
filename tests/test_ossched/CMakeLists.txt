# Imposta la versione minima di CMake richiesta
cmake_minimum_required(VERSION 3.10)

# Nome del progetto
project(test_ossched)

# Specifica lo standard C++ da utilizzare
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specifica la directory contenente i file sorgente
set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(BUILD_DIR ${CMAKE_SOURCE_DIR}/build)

# Aggiungi la directory dei file sorgente
include_directories(${SOURCE_DIR})

# Trova tutti i file sorgente nella directory src
file(GLOB SOURCES ${SOURCE_DIR}/*.cpp)

# Aggiungi l'eseguibile da compilare
add_executable(test_ossched ${SOURCES})

# Specifica dove posizionare l'eseguibile
set_target_properties(test_ossched PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR}
)

# Assicurati che la directory di output esista
file(MAKE_DIRECTORY ${BUILD_DIR})
