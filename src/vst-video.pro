#-------------------------------------------------
#
# Project created by QtCreator 2020-11-28T12:49:47
#
#-------------------------------------------------

QT       += core gui multimedia multimediawidgets xml concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = vst-video
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11
PKGCONFIG += libx11 libxtst

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    titlebar.cpp \
    playerpage.cpp \
    searchpage.cpp \
    resourcelist.cpp \
    webresource.cpp \
    framelesshelper.cpp \
    utils.cpp \
    trayicon.cpp \
    loading.cpp \
    app.cpp \
    detailpage.cpp \
    resourcesettings.cpp \
    browsepage.cpp \
    build-vst-video-Qt_5_11_3_in_PATH_qt5-Debug/moc_detialpage.cpp \
    build-vst-video-Qt_5_11_3_in_PATH_qt5-Debug/moc_framelesshelper.cpp \
    build-vst-video-Qt_5_11_3_in_PATH_qt5-Debug/moc_listpage.cpp \
    build-vst-video-Qt_5_11_3_in_PATH_qt5-Debug/moc_mainwindow.cpp \
    build-vst-video-Qt_5_11_3_in_PATH_qt5-Debug/moc_playerpage.cpp \
    build-vst-video-Qt_5_11_3_in_PATH_qt5-Debug/moc_resourcelist.cpp \
    build-vst-video-Qt_5_11_3_in_PATH_qt5-Debug/moc_searchpage.cpp \
    build-vst-video-Qt_5_11_3_in_PATH_qt5-Debug/moc_titlebar.cpp \
    build-vst-video-Qt_5_11_3_in_PATH_qt5-Debug/qrc_resources.cpp \
    app.cpp \
    browsepage.cpp \
    detailpage.cpp \
    framelesshelper.cpp \
    loading.cpp \
    main.cpp \
    mainwindow.cpp \
    playerpage.cpp \
    resourcelist.cpp \
    resourcesettings.cpp \
    searchpage.cpp \
    titlebar.cpp \
    trayicon.cpp \
    utils.cpp \
    webresource.cpp \
    playerconfig.cpp \
    shortcutsettings.cpp \
    favorites.cpp

HEADERS += \
        mainwindow.h \
    titlebar.h \
    playerpage.h \
    searchpage.h \
    resourcelist.h \
    webresource.h \
    framelesshelper.h \
    utils.h \
    trayicon.h \
    loading.h \
    app.h \
    detailpage.h \
    resourcesettings.h \
    browsepage.h \
    build-vst-video-Qt_5_11_3_in_PATH_qt5-Debug/moc_predefs.h \
    app.h \
    browsepage.h \
    detailpage.h \
    framelesshelper.h \
    loading.h \
    mainwindow.h \
    playerpage.h \
    resourcelist.h \
    resourcesettings.h \
    searchpage.h \
    titlebar.h \
    trayicon.h \
    utils.h \
    webresource.h \
    playerconfig.h \
    shortcutsettings.h \
    favorites.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    ../resources/resources.qrc
