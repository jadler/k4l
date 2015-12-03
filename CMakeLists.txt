CMAKE_MINIMUM_REQUIRED (VERSION 3.0)

IF (NOT DEFINED BUILD_SHARED_LIBS)
  SET (BUILD_SHARED_LIBS ON CACHE BOOL "Default library build type")
  MESSAGE (AUTHOR_WARNING "BUILD_SHARED_LIBS not defined, using default value: ${BUILD_SHARED_LIBS}")
ENDIF()

IF (NOT DEFINED CMAKE_BUILD_TYPE)
  SET (CMAKE_BUILD_TYPE "Release" CACHE STRING "Default build type")
  SET_PROPERTY (CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
  MESSAGE (AUTHOR_WARNING "CMAKE_BUILD_TYPE not defined, using default value: ${CMAKE_BUILD_TYPE}")
ENDIF()

#######
# Validar o compilador instalado
SET (CMAKE_C_COMPILER /usr/bin/gcc CACHE STRING "Default C compiler")
SET (CMAKE_CXX_COMPILER /usr/bin/g++ CACHE STRING "Default CXX compiler")

SET (EXTRA_DEBUG_FLAGS "${EXTRA_DEBUG_FLAGS} -Wall -ansi -O0 -fprofile-arcs -ftest-coverage")

SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

SET (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${EXTRA_DEBUG_FLAGS}")
SET (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${EXTRA_DEBUG_FLAGS}")
SET (CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${EXTRA_DEBUG_FLAGS}")
SET (CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} ${EXTRA_DEBUG_FLAGS}")
SET (CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} ${EXTRA_DEBUG_FLAGS}")

FIND_PACKAGE (Git)
IF (GIT_FOUND)
  EXECUTE_PROCESS (COMMAND ${GIT_EXECUTABLE} describe --tags --long OUTPUT_VARIABLE VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
  STRING (REGEX REPLACE "^v([0-9]+)\\..*" "\\1" VERSION_MAJOR "${VERSION}")
  STRING (REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${VERSION}")
  STRING (REGEX REPLACE "^v[0-9]+\\.[0-9]+\\-([0-9]+).*" "\\1" VERSION_PATCH "${VERSION}")
  STRING (REGEX REPLACE "^v[0-9]+\\.[0-9]+\\-[0-9]-(.*)" "\\1" VERSION_TWEAK "${VERSION}")
ELSEIF()
  SET (VERSION_MAJOR 1 CACHE STRING "Default major version")
  SET (VERSION_MINOR 0 CACHE STRING "Default minor version")
  SET (VERSION_PATCH 0 CACHE STRING "Default patch version")
  SET (VERSION_TWEAK 0 CACHE STRING "Default tweak version")
ENDIF()

SET (VERSION_SHORT "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
SET (VERSION_FULL "${VERSION_SHORT}-${VERSION_TWEAK}")

# Atribui o nome do diretório à variável ${PROJECT_NAME}, esta variável
# é criada pala macro PROJECT().
GET_FILENAME_COMPONENT (PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
SET (PROJECT_NAME ${PROJECT_NAME} CACHE STRING "Default project name")

SET (RUNTIME_DIR bin CACHE PATH "Installation directory for binaries")
SET (LIBRARY_DIR lib CACHE PATH "Installation directory for libraries")
SET (INCLUDE_DIR include CACHE PATH "Installation directory for header files")

OPTION (CMAKE_INCLUDE_CURRENT_DIR "Add the source and build directories to the include path" ON)

# Carrega todos os arquivos do diretório src em SOURCE
FILE (GLOB SOURCES "src/*.c*")

# Carrega todos os arquivos do diretório include em HEADERS
FILE (GLOB HEADERS "include/*.h*")
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/include)

PROJECT (${PROJECT_NAME})

################################################################################
################################################################################
################################################################################
################################################################################

FIND_PACKAGE (Threads)
SET (EXTRA_ARGS ${EXTRA_ARGS} ${CMAKE_THREAD_LIBS_INIT})
FIND_PACKAGE (OpenCV REQUIRED)
SET (EXTRA_ARGS ${EXTRA_ARGS} ${OpenCV_LIBS})
FIND_PACKAGE (Lua51 REQUIRED)
SET (EXTRA_ARGS ${EXTRA_ARGS} ${LUA_LIBRARIES})

OPTION (LIBRARY "Define the default artifact type" TRUE)
IF (LIBRARY)
  ADD_LIBRARY (${PROJECT_NAME} "${SOURCES}")
  MESSAGE (STATUS "Building project as a library")
ELSE ()
  ADD_EXECUTABLE (${PROJECT_NAME} "${SOURCES}")
  MESSAGE (STATUS "Building project as a binary")
ENDIF ()

TARGET_LINK_LIBRARIES (${PROJECT_NAME} ${EXTRA_ARGS})
SET_TARGET_PROPERTIES (${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${HEADERS}")

################################################################################
################################################################################
################################################################################
################################################################################

INSTALL (
  TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}Targets
  # RUNTIME refers to the runtime piece of the target. Runtime piece only applies to executable targets and DLL (Windows) style shared libraries
  RUNTIME DESTINATION ${RUNTIME_DIR}
  # LIBRARY refers to all the other (non DLL) shared libraries and modules. 
  LIBRARY DESTINATION ${LIBRARY_DIR}
  # ARCHIVE refers to the static libraries and the import parts of DLL libraries (LIB files). 
  ARCHIVE DESTINATION ${LIBRARY_DIR}
  # Specify public header files in a FRAMEWORK shared library target
  PUBLIC_HEADER DESTINATION ${INCLUDE_DIR})

INSTALL (
  EXPORT ${PROJECT_NAME}Targets
  FILE "${PROJECT_NAME}Targets.cmake"
  DESTINATION lib/${PROJECT_NAME})

INCLUDE (${CMAKE_SOURCE_DIR}/Configure.cmake)
CONFIGURE_PACKAGE_CONFIG_FILE(
  "${CMAKE_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION ${LIB_INSTALL_DIR}/${PROJECT_NAME}
  PATH_VARS
    INCLUDE_DIR
    LIBRARY_DIR
    RUNTIME_DIR)

INSTALL (
  FILES "${CMAKE_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  DESTINATION lib/${PROJECT_NAME})

WRITE_BASIC_PACKAGE_VERSION_FILE(
  ${CMAKE_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  VERSION ${VERSION_SHORT}
  COMPATIBILITY SameMajorVersion)

INSTALL (
  FILES "${CMAKE_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  DESTINATION lib/${PROJECT_NAME})

# Configurações de testes
OPTION (BUILD_TESTS "Enable build tests" OFF)
IF (BUILD_TESTS AND NOT BS_BUILD MATCHES "EXECUTABLE")
  #################
  #			CTEST			#
  #################
  SET (PROJECT_TEST_NAME ${PROJECT_NAME}Test CACHE STRING "Default project test name")
  SET_PROPERTY (DIRECTORY PROPERTY COMPILE_DEFINITIONS __TEST_CLASS__=${PROJECT_TEST_NAME})

  ENABLE_TESTING ()
  FIND_PACKAGE (GTest REQUIRED)
  INCLUDE_DIRECTORIES (${GTEST_INCLUDE_DIRS})
  SET (EXTRA_TEST_ARGS ${EXTRA_TEST_ARGS} ${GTEST_BOTH_LIBRARIES})

  #Carrega todos os arquivos de código fonte em SOURCE
  FILE (GLOB TESTS_SOURCES "test/src/*.c*")

  ADD_EXECUTABLE (${PROJECT_TEST_NAME} ${TESTS_SOURCES})
  TARGET_LINK_LIBRARIES (${PROJECT_TEST_NAME} ${EXTRA_TEST_ARGS} ${PROJECT_NAME})

  #Cria uma entrada com o mesmo nome ${PROJECT_TEST_NAME} para ser executado pelo comando make test
  ADD_TEST (${PROJECT_TEST_NAME} ${PROJECT_TEST_NAME})

  INCLUDE (CTest)
ENDIF()

# Configurações de empacotamento do projeto
OPTION (PACKAGE "Enable generate installer" OFF)
IF (PACKAGE)
  #################
  #			CPACK			#
  #################
  IF (NOT DEFINED CPACK_GENERATOR)

    MESSAGE (AUTHOR_WARNING "No generator specified, package option will be disabled")

  ELSE ()

    SET (CPACK_PACKAGE_VERSION "${VERSION_FULL}")
    SET (CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
    SET (CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
    SET (CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")
    SET (CPACK_PACKAGE_NAME "${PROJECT_NAME}" CACHE STRING "Default package name")
    SET (CPACK_PACKAGE_VENDOR "BioLogica Sistemas S.A" CACHE STRING "Default package vendor name")
    SET (CPACK_PACKAGE_DESCRIPTION_SUMMARY "" CACHE STRING "Short description summary")
    SET (CPACK_PACKAGE_INSTALL_DIRECTORY "BioLogica Sistemas" CACHE STRING "Default installation directory")
    SET (CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${VERSION_FULL}" CACHE STRING "Package file name without extension")

    MESSAGE (STATUS "cpack generator: ${CPACK_GENERATOR}")

    ##############################################################################
    #  - Bloco de configurações para empacotamento NSIS                          #
    #  - Não usado no momento                                                    #
    ##############################################################################
    #IF (CPACK_GENERATOR MATCHES "NSIS")
    #  INCLUDE (UseBSNSIS)
    #ENDIF()

    ##############################################################################
    #  - Deve ser chamado sempre após as configurações terem sido carregadas     #
    ##############################################################################
    INCLUDE (CPack)

    # Instalation types
    CPACK_ADD_INSTALL_TYPE (Developer)

    # Component groups
    CPACK_ADD_COMPONENT_GROUP (
      Runtime)

    CPACK_ADD_COMPONENT_GROUP (
      Development
      EXPANDED
      DESCRIPTION "All of the tools you'll ever need to develop software")

    # Components
    CPACK_ADD_COMPONENT (applications
      DISPLAY_NAME "${PROJECT_NAME} Application"
      DESCRIPTION "An extremely useful application that makes use of ${PROJECT_NAME}"
      GROUP Runtime
      INSTALL_TYPES Full)

    CPACK_ADD_COMPONENT (documentation
      DISPLAY_NAME "${PROJECT_NAME} Documentation"
      DESCRIPTION "The extensive suite of ${PROJECT_NAME} Application documentation files"
      GROUP Runtime
      INSTALL_TYPES Full)

    CPACK_ADD_COMPONENT (libraries
      DISPLAY_NAME "Libraries"
      DESCRIPTION "Static libraries used to build programs with ${PROJECT_NAME}"
      GROUP Development
      INSTALL_TYPES Developer Full)

    CPACK_ADD_COMPONENT (headers
      DISPLAY_NAME "C++ Headers"
      DESCRIPTION "C/C++ header files for use with ${PROJECT_NAME}"
      GROUP Development
      DEPENDS libraries
      INSTALL_TYPES Developer Full)

  ENDIF()
ENDIF()
