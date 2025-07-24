QT += core widgets gui sql network

CONFIG += c++17

TARGET = EPOBarcodeValidator
TEMPLATE = app

# D�finir la version de l'application
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

# Ressources (si vous avez des ic�nes)
# RESOURCES += resources.qrc

# Configuration pour diff�rentes plateformes
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

# OpenCV et ZXing
win32 {
    # Configuration simplifiée pour Windows - Qt uniquement
    # Si ZXing est disponible, décommentez les lignes suivantes :
    # ZXING_PATH = C:/vcpkg/installed/x64-windows
    # INCLUDEPATH += $$ZXING_PATH/include
    # LIBS += -L$$ZXING_PATH/lib -lZXing
    
    # Sinon, utilisation de Qt uniquement pour le traitement d'images
    DEFINES += QT_ONLY_IMAGE_PROCESSING
}

unix {
    # Pour Linux/Mac avec OpenCV installé via package manager
    LIBS += -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_videoio -lZXing
    INCLUDEPATH += /usr/include/opencv4
}
