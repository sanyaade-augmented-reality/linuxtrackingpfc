# GLC_Player 2.1 qmake configuration
TEMPLATE = app
CONFIG+= x86
CONFIG += warn_on debug
TARGET = glc_player
VERSION = 2.1.0

unix:OBJECTS_DIR = ./Build
unix:MOC_DIR = ./Build
unix:UI_DIR = ./Build
unix:RCC_DIR = ./Build

QT += core \
    gui \
    opengl \
    xml
    
win32 { 
    LIBS += -L"$$(GLC_LIB_DIR)/lib" \
        -lGLC_lib1
    INCLUDEPATH += "$$(GLC_LIB_DIR)/include"
    RC_FILE = ./ressources/glc_player.rc
}

unix { 
    LIBS += -lGLC_lib -lvrpnserver -lvrpnatmel -lpthread  -lquat -ltpfcc
    LIBS += -L/home/plaga/bin/vrpn_07_22/vrpn/pc_linux -L/home/plaga/bin/vrpn_07_22/vrpn/server_src/pc_linux
    LIBS += -L/home/plaga/bin/vrpn_07_22/vrpn/client_src/pc_linux -L/home/plaga/bin/vrpn_07_22/quat/pc_linux
    INCLUDEPATH += "/usr/include/GLC_lib"
    INCLUDEPATH += "/home/plaga/bin/vrpn_07_22/vrpn/server_src"
    INCLUDEPATH += "/home/plaga/bin/vrpn_07_22/vrpn/client_src"
    INCLUDEPATH += "/home/plaga/bin/vrpn_07_22/vrpn"
    INCLUDEPATH += "/home/plaga/bin/vrpn_07_22/quat"
}

TRANSLATIONS = ressources/glc_player_fr.ts

HEADERS_GLCPLAYER += 	FileEntry.h \
						OpenFileThread.h \
						glc_player.h \
						AlbumFile.h \
						FileOpenFilter.h \
						UserInterfaceSate.h \
						ExportToWeb.h
						
HEADERS_UICLASS +=		ui_class/AboutPlayer.h \		
						ui_class/SettingsDialog.h \
						ui_class/EditCamera.h \
						ui_class/EditLightDialog.h \
						ui_class/SelectionProperty.h \
						ui_class/AlbumManagerView.h \
						ui_class/InstanceProperty.h \
						ui_class/MaterialProperty.h \
						ui_class/ChooseShaderDialog.h \
						ui_class/ListOfMaterial.h \
						ui_class/ModelProperties.h \
						ui_class/OpenAlbumOption.h \
						ui_class/SendFilesDialog.h \
						ui_class/ScreenshotDialog.h \
						ui_class/MultiScreenshotsDialog.h \
						ui_class/ExportWebDialog.h \
						ui_class/LeftSideDock.h \
						ui_class/ModelManagerView.h \
						ui_class/ModelStructure.h

HEADERS_OPENGLVIEW +=	opengl_view/OpenglView.h \
						opengl_view/MaterialOpenglView.h \
						opengl_view/MultiShotsOpenglView.h
							
HEADERS += $${HEADERS_GLCPLAYER} $${HEADERS_UICLASS} $${HEADERS_OPENGLVIEW}

SOURCES_GLCPLAYER +=	main.cpp \
					 	FileEntry.cpp \
						OpenFileThread.cpp \
						glc_player.cpp \
						AlbumFile.cpp \
						FileOpenFilter.cpp \
						UserInterfaceSate.cpp \
						ExportToWeb.cpp
						
SOURCES_UICLASS +=		ui_class/AboutPlayer.cpp \		
						ui_class/SettingsDialog.cpp \
						ui_class/EditCamera.cpp \
						ui_class/EditLightDialog.cpp \
						ui_class/SelectionProperty.cpp \
						ui_class/AlbumManagerView.cpp \
						ui_class/InstanceProperty.cpp \
						ui_class/MaterialProperty.cpp \
						ui_class/ChooseShaderDialog.cpp \
						ui_class/ListOfMaterial.cpp \
						ui_class/ModelProperties.cpp \
						ui_class/OpenAlbumOption.cpp \
						ui_class/SendFilesDialog.cpp \
						ui_class/ScreenshotDialog.cpp \
						ui_class/MultiScreenshotsDialog.cpp \
						ui_class/ExportWebDialog.cpp \
						ui_class/LeftSideDock.cpp \
						ui_class/ModelManagerView.cpp \
						ui_class/ModelStructure.cpp

SOURCES_OPENGLVIEW +=	opengl_view/OpenglView.cpp \
						opengl_view/MaterialOpenglView.cpp \
						opengl_view/MultiShotsOpenglView.cpp
												
SOURCES += $${SOURCES_GLCPLAYER} $${SOURCES_UICLASS} $${SOURCES_OPENGLVIEW}

FORMS +=				designer/HelpBrowser.ui \
						designer/AboutPlayer.ui \
						designer/SettingsDialog.ui \
						designer/glc_player.ui \
						designer/EditCamera.ui \
						designer/SelectionProperty.ui \
						designer/EditLightDialog.ui \
						designer/AlbumManagerView.ui \
						designer/InstanceProperty.ui \
						designer/MaterialProperty.ui \
						designer/ChooseShaderDialog.ui \
						designer/ListOfMaterial.ui \
						designer/ModelProperties.ui \
						designer/OpenAlbumOption.ui \
						designer/SendFilesDialog.ui \
						designer/ScreenshotDialog.ui \
						designer/MultiScreenshotsDialog.ui \
						designer/ExportWebDialog.ui \
						designer/LeftSideDock.ui \
						designer/ModelManagerView.ui \
						designer/ModelStructure.ui
    
RESOURCES +=			ressources/lang.qrc \
    					designer/glc_player.qrc \
    					ressources/shaders.qrc

mac {
    ICON = ressources/images/Logo.icns
    QMAKE_INFO_PLIST = ressources/Info_mac.plist
    TARGET = glc_player
}
    