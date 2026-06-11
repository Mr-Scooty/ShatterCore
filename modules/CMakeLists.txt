# This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

message("")

# Make the module list available in the current scope
GetModuleSourceList(MODULES_MODULE_LIST)

# Make the native install offset available in this scope
GetInstallOffset(INSTALL_OFFSET)

# Sets the MODULE_${SOURCE_MODULE} variables
# when using predefined templates for module building
# like dynamic, static
# Sets MODULES_DEFAULT_LINKAGE
if(MODULES MATCHES "dynamic")
  set(MODULES_DEFAULT_LINKAGE "dynamic")
elseif(MODULES MATCHES "static")
  set(MODULES_DEFAULT_LINKAGE "static")
else()
  set(MODULES_DEFAULT_LINKAGE "disabled")
endif()

# Set the MODULE_${SOURCE_MODULE} variables from the
# variables set above
foreach(SOURCE_MODULE ${MODULES_MODULE_LIST})
  ModuleNameToVariable(${SOURCE_MODULE} MODULE_MODULE_VARIABLE)

  if(${MODULE_MODULE_VARIABLE} STREQUAL "default")
    set(${MODULE_MODULE_VARIABLE} ${MODULES_DEFAULT_LINKAGE})
  endif()

  # Build the Graph values
  if(${MODULE_MODULE_VARIABLE} MATCHES "dynamic")
    GetProjectNameOfModuleName(${SOURCE_MODULE} MODULE_SOURCE_PROJECT_NAME)
    GetNativeSharedLibraryName(${MODULE_SOURCE_PROJECT_NAME} MODULE_PROJECT_LIBRARY)
    list(APPEND MODULE_GRAPH_KEYS ${MODULE_SOURCE_PROJECT_NAME})
    set(MODULE_GRAPH_VALUE_DISPLAY_${MODULE_SOURCE_PROJECT_NAME} ${MODULE_PROJECT_LIBRARY})
    list(APPEND MODULE_GRAPH_VALUE_CONTAINS_MODULES_${MODULE_SOURCE_PROJECT_NAME} ${SOURCE_MODULE})
  elseif(${MODULE_MODULE_VARIABLE} MATCHES "static")
    list(APPEND MODULE_GRAPH_KEYS worldserver)
    set(MODULE_GRAPH_VALUE_DISPLAY_worldserver worldserver)
    list(APPEND MODULE_GRAPH_VALUE_CONTAINS_MODULES_worldserver ${SOURCE_MODULE})
  else()
    list(APPEND MODULE_GRAPH_KEYS disabled)
    set(MODULE_GRAPH_VALUE_DISPLAY_disabled disabled)
    list(APPEND MODULE_GRAPH_VALUE_CONTAINS_MODULES_disabled ${SOURCE_MODULE})
  endif()
endforeach()

list(SORT MODULE_GRAPH_KEYS)
list(REMOVE_DUPLICATES MODULE_GRAPH_KEYS)

# Display the module graph
message("* Modules configuration (${MODULES}):
    |")

foreach(MODULE_GRAPH_KEY ${MODULE_GRAPH_KEYS})
  if(NOT MODULE_GRAPH_KEY STREQUAL "disabled")
    message("    +- ${MODULE_GRAPH_VALUE_DISPLAY_${MODULE_GRAPH_KEY}}")
  else()
    message("    |  ${MODULE_GRAPH_VALUE_DISPLAY_${MODULE_GRAPH_KEY}}")
  endif()
  foreach(MODULE_GRAPH_PROJECT_ENTRY ${MODULE_GRAPH_VALUE_CONTAINS_MODULES_${MODULE_GRAPH_KEY}})
    message("    |   +- ${MODULE_GRAPH_PROJECT_ENTRY}")
  endforeach()
  message("    |")
endforeach()

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

# Configures the module loader with the given name and stores the output in the
# LOADER_OUT variable. It is possible to expose multiple modules from the same
# loader through passing them to the variable arguments.
# Deliberately not named ConfigureScriptLoader - that function is already
# defined globally by src/server/scripts/CMakeLists.txt.
function(ConfigureModulesLoader MODULELOADER_NAME LOADER_OUT IS_DYNAMIC_SCRIPTLOADER)
  # Deduces the following variables which are referenced by the template:
  # TRINITY_IS_DYNAMIC_SCRIPTLOADER
  # TRINITY_SCRIPTS_FORWARD_DECL
  # TRINITY_SCRIPTS_INVOKE
  # TRINITY_CURRENT_SCRIPT_PROJECT

  # To generate export macros
  set(TRINITY_IS_DYNAMIC_SCRIPTLOADER ${IS_DYNAMIC_SCRIPTLOADER})
  # To generate forward declarations of the loading functions
  unset(TRINITY_SCRIPTS_FORWARD_DECL)
  unset(TRINITY_SCRIPTS_INVOKE)
  # The current module project which is built in. This becomes the
  # script context name and the name used by the hotswap recompiler
  # ("scripts_" + name must equal the CMake project name).
  string(TOLOWER ${MODULELOADER_NAME} TRINITY_CURRENT_SCRIPT_PROJECT)
  string(REPLACE "-" "_" TRINITY_CURRENT_SCRIPT_PROJECT ${TRINITY_CURRENT_SCRIPT_PROJECT})

  foreach(LOCALE_MODULE ${ARGN})
    # Module names use hyphens (mod-example) which aren't valid in
    # C++ identifiers - the loader function uses underscores instead.
    string(REPLACE "-" "_" LOCALE_MODULE ${LOCALE_MODULE})

    # Determine the loader function ("Add##${NameOfModule}##Scripts()")
    set(LOADER_FUNCTION
      "Add${LOCALE_MODULE}Scripts()")
    # Generate the function call and the forward declarations
    set(TRINITY_SCRIPTS_FORWARD_DECL
      "${TRINITY_SCRIPTS_FORWARD_DECL}void ${LOADER_FUNCTION};\n")
    set(TRINITY_SCRIPTS_INVOKE
      "${TRINITY_SCRIPTS_INVOKE}    ${LOADER_FUNCTION};\n")
  endforeach()

  set(GENERATED_LOADER ${CMAKE_CURRENT_BINARY_DIR}/gen_moduleloader/${MODULELOADER_NAME}/ModulesLoader.cpp)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ModulesLoader.cpp.in.cmake ${GENERATED_LOADER})
  set(${LOADER_OUT} ${GENERATED_LOADER} PARENT_SCOPE)
endfunction()

# Generates the actual module projects
# Fills the STATIC_MODULES and DYNAMIC_MODULE_PROJECTS variables
# which contain the names which modules are linked statically/dynamically and
# adds the sources of the static modules to the PRIVATE_SOURCES_MODULES variable.
foreach(SOURCE_MODULE ${MODULES_MODULE_LIST})
  GetPathToModuleSource(${SOURCE_MODULE} MODULE_SOURCE_PATH)
  ModuleNameToVariable(${SOURCE_MODULE} MODULE_MODULE_VARIABLE)

  if(NOT ${MODULE_MODULE_VARIABLE} STREQUAL "disabled")
    list(APPEND ENABLED_MODULES ${SOURCE_MODULE})
  endif()

  if((${MODULE_MODULE_VARIABLE} STREQUAL "disabled") OR
      (${MODULE_MODULE_VARIABLE} STREQUAL "static"))
    # Uninstall disabled modules
    GetProjectNameOfModuleName(${SOURCE_MODULE} MODULE_SOURCE_PROJECT_NAME)
    GetNativeSharedLibraryName(${MODULE_SOURCE_PROJECT_NAME} MODULE_OUTPUT_NAME)
    list(APPEND DISABLED_MODULE_PROJECTS ${INSTALL_OFFSET}/${MODULE_OUTPUT_NAME})
    if(${MODULE_MODULE_VARIABLE} STREQUAL "static")
      # Add the module name to STATIC_MODULES
      list(APPEND STATIC_MODULES ${SOURCE_MODULE})
      # Add the module content to the whole static module
      CollectSourceFiles(${MODULE_SOURCE_PATH} PRIVATE_SOURCES_MODULES)
      CollectIncludeDirectories(${MODULE_SOURCE_PATH} PUBLIC_INCLUDES_MODULES)
    endif()
  elseif(${MODULE_MODULE_VARIABLE} STREQUAL "dynamic")
    # Generate an own dynamic module which is loadable on runtime
    unset(MODULE_SOURCE_PRIVATE_SOURCES)
    unset(MODULE_SOURCE_PUBLIC_INCLUDES)
    CollectSourceFiles(${MODULE_SOURCE_PATH} MODULE_SOURCE_PRIVATE_SOURCES)
    CollectIncludeDirectories(${MODULE_SOURCE_PATH} MODULE_SOURCE_PUBLIC_INCLUDES)

    # Configure the module loader
    ConfigureModulesLoader(${SOURCE_MODULE} MODULE_PRIVATE_SCRIPTLOADER ON ${SOURCE_MODULE})
    GetProjectNameOfModuleName(${SOURCE_MODULE} MODULE_SOURCE_PROJECT_NAME)

    # Add the module project to DYNAMIC_MODULE_PROJECTS
    list(APPEND DYNAMIC_MODULE_PROJECTS ${MODULE_SOURCE_PROJECT_NAME})

    # Create the module project
    add_library(${MODULE_SOURCE_PROJECT_NAME} SHARED
      ${MODULE_SOURCE_PRIVATE_SOURCES}
      ${MODULE_PRIVATE_SCRIPTLOADER})

    target_link_libraries(${MODULE_SOURCE_PROJECT_NAME}
      PRIVATE
        trinity-core-interface
      PUBLIC
        game)

    target_include_directories(${MODULE_SOURCE_PROJECT_NAME}
      PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${MODULE_SOURCE_PUBLIC_INCLUDES})

    set_target_properties(${MODULE_SOURCE_PROJECT_NAME}
      PROPERTIES
        FOLDER
          "modules")

    if(UNIX)
      install(TARGETS ${MODULE_SOURCE_PROJECT_NAME}
        DESTINATION ${INSTALL_OFFSET}
        COMPONENT ${MODULE_SOURCE_PROJECT_NAME})
    elseif(WIN32)
      install(TARGETS ${MODULE_SOURCE_PROJECT_NAME}
        RUNTIME DESTINATION ${INSTALL_OFFSET}
        COMPONENT ${MODULE_SOURCE_PROJECT_NAME})
      if(MSVC)
        # Place the module libraries in the script subdirectory
        # where the ScriptReloadMgr discovers them
        set_target_properties(${MODULE_SOURCE_PROJECT_NAME} PROPERTIES
          RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug/scripts
          RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release/scripts
          RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/bin/RelWithDebInfo/scripts
          RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/bin/MinSizeRel/scripts)
      endif()
    endif()
  else()
    message(FATAL_ERROR "Unknown value \"${${MODULE_MODULE_VARIABLE}}\" for module (${SOURCE_MODULE})!")
  endif()
endforeach()

# Add the dynamic module projects to the worldserver as dependency
set(WORLDSERVER_DYNAMIC_MODULE_DEPENDENCIES ${DYNAMIC_MODULE_PROJECTS} PARENT_SCOPE)

ConfigureModulesLoader("static" MODULES_PRIVATE_SCRIPTLOADER OFF ${STATIC_MODULES})

# The modules library always exists (even when empty or with MODULES=disabled)
# so the worldserver can link against it unconditionally.
add_library(modules STATIC
  ModulesScriptLoader.h
  ${MODULES_PRIVATE_SCRIPTLOADER}
  ${PRIVATE_SOURCES_MODULES})

target_link_libraries(modules
  PRIVATE
    trinity-core-interface
  PUBLIC
    game-interface)

target_include_directories(modules
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PUBLIC_INCLUDES_MODULES})

set_target_properties(modules
  PROPERTIES
    FOLDER
      "modules")

# Enables module authors to include a cmake file in their module
# that will get run inline with the configuration.
foreach(SOURCE_MODULE ${MODULES_MODULE_LIST})
  include("${CMAKE_SOURCE_DIR}/modules/${SOURCE_MODULE}/${SOURCE_MODULE}.cmake" OPTIONAL)
endforeach()

# Remove all shared libraries in the install directory which
# are contained in the static library already (or disabled).
if(DISABLED_MODULE_PROJECTS)
  install(CODE "
    foreach(MODULE_TO_UNINSTALL ${DISABLED_MODULE_PROJECTS})
      if(EXISTS \"\${MODULE_TO_UNINSTALL}\")
        message(STATUS \"Uninstalling: \${MODULE_TO_UNINSTALL}\")
        file(REMOVE \"\${MODULE_TO_UNINSTALL}\")
      endif()
    endforeach()
  ")
endif()

# Collect the configuration files (*.conf.dist) of all enabled modules,
# install them and build the comma separated lists which are baked into
# the modules target as TC_MODULES_LIST and CONFIG_FILE_LIST.
if(ENABLED_MODULES)
  message("* Modules config list:
    |")

  foreach(SOURCE_MODULE ${ENABLED_MODULES})
    GetPathToModuleConfig(${SOURCE_MODULE} MODULE_CONFIG_PATH)

    file(GLOB MODULE_CONFIG_LIST RELATIVE
      ${MODULE_CONFIG_PATH}
      ${MODULE_CONFIG_PATH}/*.conf.dist)

    message("    +- ${SOURCE_MODULE}")

    foreach(configFileName ${MODULE_CONFIG_LIST})
      CopyModuleConfig("${MODULE_CONFIG_PATH}/${configFileName}")
      string(REGEX REPLACE "\\.dist$" "" configFileName ${configFileName})
      list(APPEND MODULES_CONFIG_FILE_LIST ${configFileName})
      message("    |   * ${configFileName}")
    endforeach()
    message("    |")
  endforeach()
endif()

string(JOIN "," MODULES_LIST_JOINED ${ENABLED_MODULES})
string(JOIN "," CONFIG_FILE_LIST_JOINED ${MODULES_CONFIG_FILE_LIST})

# Define the enabled modules list and their config file list
target_compile_definitions(modules
  INTERFACE
    TC_MODULES_LIST="${MODULES_LIST_JOINED}"
    CONFIG_FILE_LIST="${CONFIG_FILE_LIST_JOINED}")

message("")
