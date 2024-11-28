cmake_minimum_required(VERSION 3.16)

project(inertiaengineplugin VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_QML_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(CMAKE_BUILD_TYPE STREQUAL RELEASE)
    set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")
endif()

find_package(Qt6 6.2 COMPONENTS Quick REQUIRED)

qt6_policy(SET QTP0001 NEW)
qt_standard_project_setup()

# Uncomment the next line and replace "..." with the extension plugins root path of your choice
#set(PLUGINS_PATH ...)

set(PLUGINS_PATH_PREFIX "/MyModules/Inertia/Engine")

qt_add_qml_module(inertiaengineplugin
                  URI "MyModules.Inertia.Engine"
                  VERSION 1.0

                  PLUGIN_TARGET inertiaengineplugin

                  SOURCES
                  model/game-model.cpp
                  model/game-model.hpp
                  state/game-state-maintainer.hpp
                  state/game-state-maintainer.cpp
                  state/state-wrapper.hpp
                  game-generator/game-generator.hpp
                  game-generator/game-generator.cpp
                  service/move-handler.hpp
                  service/move-handler.cpp
                  service/hint-handler.hpp
                  service/hint-handler.cpp
                  common-definitions.hpp
                  utility.hpp
                  utility.cpp
                  constants.hpp
                  movement-result.hpp
                  movement-result.cpp)

target_link_libraries(inertiaengineplugin PRIVATE Qt6::Quick)

target_include_directories(inertiaengineplugin PUBLIC
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           ${CMAKE_CURRENT_SOURCE_DIR}/model
                           ${CMAKE_CURRENT_SOURCE_DIR}/state)

install(TARGETS inertiaengineplugin
        RUNTIME DESTINATION "${PLUGINS_PATH}${PLUGINS_PATH_PREFIX}"
        LIBRARY DESTINATION "${PLUGINS_PATH}${PLUGINS_PATH_PREFIX}")

 install(FILES
         ${CMAKE_CURRENT_BINARY_DIR}${PLUGINS_PATH_PREFIX}/qmldir
         ${CMAKE_CURRENT_BINARY_DIR}${PLUGINS_PATH_PREFIX}/inertiaengineplugin.qmltypes
         ${CMAKE_CURRENT_BINARY_DIR}${PLUGINS_PATH_PREFIX}/inertiaengineplugin_qml_module_dir_map.qrc
         DESTINATION "${PLUGINS_PATH}${PLUGINS_PATH_PREFIX}")


