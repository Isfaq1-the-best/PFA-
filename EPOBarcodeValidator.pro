QT += core widgets gui sql network

CONFIG += c++17

TARGET = EPOBarcodeValidator
TEMPLATE = app

# Définir la version de l'application
VERSION = 1.0.0
QMAKE_TARGET_COMPANY = "EPO Project"
QMAKE_TARGET_PRODUCT = "EPO Barcode Validator"
QMAKE_TARGET_DESCRIPTION = "Application de validation de codes-barres"
QMAKE_TARGET_COPYRIGHT = "Copyright (c) 2025 EPO Project"

# Sources
SOURCES += \
    main.cpp \
    SplashWindow.cpp \
    MainWindow.cpp \
    BarcodeValidator.cpp \
    DatabaseManager.cpp \
    CameraWidget.cpp \
    ThemeManager.cpp

# Headers
HEADERS += \
    SplashWindow.h \
    MainWindow.h \
    BarcodeValidator.h \
    DatabaseManager.h \
    CameraWidget.h \
    ThemeManager.h

# Ressources (si vous avez des icônes)
# RESOURCES += resources.qrc

# Configuration pour différentes plateformes
win32 {
    RC_ICONS = icon.ico
    QMAKE_TARGET_PRODUCT = "EPO Barcode Validator"
}

macx {
    ICON = icon.icns
    QMAKE_INFO_PLIST = Info.plist
}

unix:!macx {
    target.path = /usr/local/bin
    INSTALLS += target
}

# Optimisations
CONFIG(release, debug|release) {
    DEFINES += QT_NO_DEBUG_OUTPUT
}

# Avertissements
QMAKE_CXXFLAGS += -Wall -Wextra

# OpenCV
win32 {
    # Ajustez le chemin selon votre installation OpenCV
    OPENCV_PATH = C:/opencv/build
    INCLUDEPATH += $$OPENCV_PATH/include
    
    CONFIG(debug, debug|release) {
        LIBS += -L$$OPENCV_PATH/x64/vc16/lib \
                -lopencv_core4d \
                -lopencv_imgproc4d \
                -lopencv_imgcodecs4d \
                -lopencv_videoio4d
    } else {
        LIBS += -L$$OPENCV_PATH/x64/vc16/lib \
                -lopencv_core4 \
                -lopencv_imgproc4 \
                -lopencv_imgcodecs4 \
                -lopencv_videoio4
    }
}

unix {
    # Pour Linux/Mac avec OpenCV installé via package manager
    LIBS += -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_videoio
    INCLUDEPATH += /usr/local/include/opencv4
}
