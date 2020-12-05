cmake_minimum_required (VERSION 3.5)

EXECUTE_PROCESS(COMMAND git describe --tags
    TIMEOUT 5
    OUTPUT_VARIABLE GIT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)

string(REGEX REPLACE "^[A-Za-z]" "" GIT_VERSION ${GIT_VERSION})

string(FIND ${GIT_VERSION} "-" index REVERSE)
string(SUBSTRING ${GIT_VERSION} 0 ${index} GIT_VERSION_WITH_REVISION)
message(STATUS "building from git tag ${GIT_VERSION_WITH_REVISION}")

string(FIND ${GIT_VERSION} "-" index)
string(SUBSTRING ${GIT_VERSION} 0 ${index} GIT_VERSION)
message(STATUS "building from git tag ${GIT_VERSION}")

add_definitions(-DBUILD_VERSION=\"${GIT_VERSION}\")

project(vst-video 
    VERSION ${GIT_VERSION}
    DESCRIPTION "基于 QMultiMedia 组件构建，目标是搭建 deepin v20 云播放平台。"
    HOMEPAGE_URL "https://github.com/xymov/vst-video"
    LANGUAGES "CXX")

# build targets

find_package(Qt5 COMPONENTS Core Gui Multimedia MultimediaWidgets Xml Concurrent)

aux_source_directory(src SRC)

file(GLOB RCC_FILES resources/*.qrc)

add_executable(${PROJECT_NAME} ${SRC} ${RCC_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES 
    OUTPUT_NAME ${PROJECT_NAME}
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON)

target_link_libraries(${PROJECT_NAME} 
    Qt5::Widgets Qt5::Core Qt5::Gui 
    Qt5::Multimedia Qt5::MultimediaWidgets Qt5::Xml Qt5::Concurrent
    X11 Xtst)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories(src)

add_definitions(${QT_DEFINITIONS})

# export targets

set(RESOURCE_FILES
    LICENSE
    README.md
)
set_target_properties(${PROJECT_NAME} PROPERTIES
  RESOURCE "${RESOURCE_FILES}")

if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")

    set(CMAKE_INSTALL_PREFIX "/usr")
    
    install(TARGETS ${PROJECT_NAME}
        #CONFIGURATIONS Release
        RUNTIME DESTINATION bin 
        RESOURCE DESTINATION share/doc/${PROJECT_NAME})
    
    file(GLOB DESKTOP_FILES resources/*.desktop)
    
    install(FILES ${DESKTOP_FILES} 
        DESTINATION share/applications)
    
    install(FILES resources/img/ico.svg
        DESTINATION share/icons/hicolor/scalable/apps
        RENAME ${PROJECT_NAME}.svg)

endif()

# build packages

set(CPACK_PACKAGE_DESCRIPTION 
    "基于 QMultiMedia 组件构建，目标是搭建 deepin v20 云播放平台。")

set(CPACK_PACKAGE_HOMEPAGE_URL ${PROJECT_HOMEPAGE_URL})
set(CPACK_PACKAGE_CONTACT "lidanger <lidanger@outlook.com>")

set(CPACK_PACKAGE_VERSION ${GIT_VERSION_WITH_REVISION})

set(CPACK_RESOURCE_FILE_LICENSE, ${PROJECT_SOURCE_DIR}/LICENSE)
set(CPACK_RESOURCE_FILE_README, ${PROJECT_SOURCE_DIR}/README.md)

set(CPACK_PACKAGE_CHECKSUM "MD5")

set(CPACK_SET_DESTDIR true)

include(InstallRequiredSystemLibraries)

# source package
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES "/CVS/;/.svn/;/.git/;.swp$;.#;/#;/build/;~") 

# targets package
set(GENERATORS)

# deb packages 
find_program(DPKG "dpkg")
if(DPKG)
    list(APPEND GENERATORS "DEB")
    
    set(CPACK_GENERATOR "DEB")
    
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
    
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_SECTION "text")
    
    set(CPACK_DEBIAN_PACKAGE_HOMEPAGE ${PROJECT_HOMEPAGE_URL})

    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5multimedia5-plugins, gstreamer1.0-plugins-bad, gstreamer1.0-libav")
    
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
endif()

# tgz package
if(NOT GENERATORS)
    list(APPEND GENERATORS "TGZ")
endif()

set(CPACK_GENERATOR ${GENERATORS})

include(CPack)
