# This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

# Returns the base path to the modules directory in the source directory
function(GetModulesBasePath variable)
  set(${variable} "${CMAKE_SOURCE_DIR}/modules" PARENT_SCOPE)
endfunction()

# Stores the absolute path to the source directory of the given module in the variable
function(GetPathToModuleSource module variable)
  GetModulesBasePath(MODULES_BASE_PATH)
  set(${variable} "${MODULES_BASE_PATH}/${module}/src" PARENT_SCOPE)
endfunction()

# Stores the absolute path to the conf directory of the given module in the variable
function(GetPathToModuleConfig module variable)
  GetModulesBasePath(MODULES_BASE_PATH)
  set(${variable} "${MODULES_BASE_PATH}/${module}/conf" PARENT_SCOPE)
endfunction()

# Stores the project name of the given module in the variable.
# Modules deliberately use the "scripts_" prefix so dynamically linked
# modules are discovered and hot-reloaded by the existing ScriptReloadMgr
# (which matches lib[sS]cripts_*.so and rebuilds the scripts_${module} target).
function(GetProjectNameOfModuleName module variable)
  string(TOLOWER "scripts_${module}" GENERATED_NAME)
  string(REPLACE "-" "_" GENERATED_NAME ${GENERATED_NAME})
  set(${variable} "${GENERATED_NAME}" PARENT_SCOPE)
endfunction()

# Creates a list of all modules (directories below modules/ containing a src/ dir)
# and stores it in the given variable.
function(GetModuleSourceList variable)
  GetModulesBasePath(BASE_PATH)
  file(GLOB LOCALE_MODULE_LIST RELATIVE
    ${BASE_PATH}
    ${BASE_PATH}/*)

  set(${variable})
  foreach(SOURCE_MODULE ${LOCALE_MODULE_LIST})
    GetPathToModuleSource(${SOURCE_MODULE} MODULE_SOURCE_PATH)
    if(IS_DIRECTORY ${MODULE_SOURCE_PATH})
      list(APPEND ${variable} ${SOURCE_MODULE})
    endif()
  endforeach()
  set(${variable} ${${variable}} PARENT_SCOPE)
endfunction()

# Converts the given module name into its
# variable name which holds the linkage type.
function(ModuleNameToVariable module variable)
  string(TOUPPER ${module} ${variable})
  string(REPLACE "-" "_" ${variable} ${${variable}})
  set(${variable} "MODULE_${${variable}}")
  set(${variable} ${${variable}} PARENT_SCOPE)
endfunction()

# Stores in the given variable whether dynamic linking is required for modules
function(IsDynamicLinkingModulesRequired variable)
  if(MODULES MATCHES "dynamic")
    set(IS_DEFAULT_VALUE_DYNAMIC_MODULE ON)
  endif()

  GetModuleSourceList(MODULES_MODULE_LIST)
  set(IS_REQUIRED OFF)
  foreach(SOURCE_MODULE ${MODULES_MODULE_LIST})
    ModuleNameToVariable(${SOURCE_MODULE} MODULE_MODULE_VARIABLE)
    if((${MODULE_MODULE_VARIABLE} STREQUAL "dynamic") OR
        (${MODULE_MODULE_VARIABLE} STREQUAL "default" AND IS_DEFAULT_VALUE_DYNAMIC_MODULE))
      set(IS_REQUIRED ON)
      break()
    endif()
  endforeach()
  set(${variable} ${IS_REQUIRED} PARENT_SCOPE)
endfunction()

# Installs the given module configuration file (*.conf.dist) into the
# module configuration directory and copies it next to the binaries on Windows.
function(CopyModuleConfig configFile)
  if(UNIX)
    install(FILES "${configFile}" DESTINATION "${CONF_DIR}/modules")
  elseif(WIN32)
    install(FILES "${configFile}" DESTINATION "${CMAKE_INSTALL_PREFIX}/modules")
    if("${CMAKE_MAKE_PROGRAM}" MATCHES "MSBuild")
      add_custom_command(TARGET modules
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/bin/$(ConfigurationName)/modules"
        COMMAND ${CMAKE_COMMAND} -E copy "${configFile}" "${CMAKE_BINARY_DIR}/bin/$(ConfigurationName)/modules")
    elseif(MINGW)
      add_custom_command(TARGET modules
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/bin/modules"
        COMMAND ${CMAKE_COMMAND} -E copy "${configFile}" "${CMAKE_BINARY_DIR}/bin/modules")
    endif()
  endif()
endfunction()
