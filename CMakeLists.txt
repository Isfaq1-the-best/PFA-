cmake_minimum_required(VERSION 3.16)

project(EPOBarcodeValidator VERSION 1.0.0 LANGUAGES CXX)

# Configuration C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configuration pour Visual Studio
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    
    # Support UTF-8
    add_compile_options(/utf-8)
    
    # D�finir WIN32_LEAN_AND_MEAN pour �viter les conflits
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DNOMINMAX)
endif()

# Configuration Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Trouver Qt5 - Configuration pour Qt VS Tools
find_package(Qt5 REQUIRED COMPONENTS 
    Core 
    Widgets 
    Gui 
    Sql 
    Network 
)

# Trouver OpenCV
find_package(OpenCV REQUIRED)

# ZXing (optionnel - pour une meilleure d�tection de codes-barres)
# find_package(ZXing QUIET)
# if(ZXing_FOUND)
#     message(STATUS "ZXing trouv�: ${ZXing_VERSION}")
#     add_definitions(-DUSE_ZXING)
# else()
#     message(STATUS "ZXing non trouv� - utilisation d'OpenCV uniquement")
# endif()

# V�rifier que Qt a �t� trouv�
if(NOT Qt5_FOUND)
    message(FATAL_ERROR "Qt6 non trouv�. Assurez-vous que Qt VS Tools est correctement configur�.")
endif()

# V�rifier qu'OpenCV a �t� trouv�
if(NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV non trouv�. Installez OpenCV et configurez OpenCV_DIR.")
endif()

# Afficher les informations Qt pour debug
message(STATUS "Qt6 trouv�: ${Qt6_VERSION}")
message(STATUS "Qt6 Directory: ${Qt6_DIR}")
message(STATUS "OpenCV trouv�: ${OpenCV_VERSION}")
message(STATUS "OpenCV Directory: ${OpenCV_DIR}")

# D�finir les fichiers sources
set(SOURCES
    main.cpp
    SplashWindow.cpp
    MainWindow.cpp
    BarcodeValidator.cpp
    DatabaseManager.cpp
    CameraWidget.cpp
    ThemeManager.cpp
)

set(HEADERS
    SplashWindow.h
    MainWindow.h
    BarcodeValidator.h
    DatabaseManager.h
    CameraWidget.h
    ThemeManager.h
)

# Fichiers de ressources (optionnel)
set(RESOURCES
    # resources.qrc
)

# Cr�er l'ex�cutable Windows
add_executable(EPOBarcodeValidator WIN32 ${SOURCES} ${HEADERS} ${RESOURCES})

# Propri�t�s sp�cifiques � Windows
set_target_properties(EPOBarcodeValidator PROPERTIES
    WIN32_EXECUTABLE TRUE
    OUTPUT_NAME "EPOBarcodeValidator"
    DEBUG_POSTFIX "_d"
)

# Lier les bibliothèques Qt
target_link_libraries(EPOBarcodeValidator
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Qt5::Sql
    Qt5::Network
    ${OpenCV_LIBS}
    # zxing
)

# Rpertoires d'inclusion
target_include_directories(EPOBarcodeValidator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5Sql_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
)

# D�finitions de compilation
target_compile_definitions(EPOBarcodeValidator PRIVATE
    QT_WIDGETS_LIB
    QT_GUI_LIB
    QT_CORE_LIB
    QT_SQL_LIB
    QT_NETWORK_LIB
)

# Configuration Debug/Release
target_compile_definitions(EPOBarcodeValidator PRIVATE
    $<$<CONFIG:Debug>:QT_QML_DEBUG>
    $<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>
)

# D�ploiement automatique des DLLs Qt (windeployqt)
if(WIN32)
    # Trouver windeployqt
    get_target_property(QT5_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
    get_filename_component(QT5_WINDEPLOYQT_EXECUTABLE ${QT5_QMAKE_EXECUTABLE} PATH)
    set(QT5_WINDEPLOYQT_EXECUTABLE "${QT5_WINDEPLOYQT_EXECUTABLE}/windeployqt.exe")

    if(EXISTS ${QT6_WINDEPLOYQT_EXECUTABLE})
        # Commande post-build pour d�ployer les DLLs Qt
        add_custom_command(TARGET EPOBarcodeValidator POST_BUILD
            COMMAND ${QT6_WINDEPLOYQT_EXECUTABLE} 
                --verbose 2
                --no-translations
                --no-system-d3d-compiler
                --no-opengl-sw
                $<TARGET_FILE:EPOBarcodeValidator>
            COMMENT "D�ploiement des biblioth�ques Qt"
        )
        message(STATUS "windeployqt trouv�: ${QT6_WINDEPLOYQT_EXECUTABLE}")
    else()
        message(WARNING "windeployqt non trouv�. Les DLLs Qt devront �tre copi�es manuellement.")
    endif()
endif()

# Configuration pour diff�rents g�n�rateurs Visual Studio
if(CMAKE_GENERATOR MATCHES "Visual Studio")
    # D�finir le projet de d�marrage
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT EPOBarcodeValidator)
    
    # Organiser les fichiers dans l'explorateur de solutions
    source_group("Header Files" FILES ${HEADERS})
    source_group("Source Files" FILES ${SOURCES})
    if(RESOURCES)
        source_group("Resource Files" FILES ${RESOURCES})
    endif()
    
    # Configuration des r�pertoires de sortie
    set_target_properties(EPOBarcodeValidator PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release"
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/RelWithDebInfo"
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/MinSizeRel"
    )
endif()

# Installation
install(TARGETS EPOBarcodeValidator
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Installer les DLLs Qt n�cessaires
if(WIN32)
    install(CODE "
        include(BundleUtilities)
        fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/bin/EPOBarcodeValidator.exe\" \"\" \"\")
    ")
endif()

# Informations de build
message(STATUS "=== Configuration EPO Barcode Validator ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Compilateur: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Standard C++: ${CMAKE_CXX_STANDARD}")
message(STATUS "Type de build: ${CMAKE_BUILD_TYPE}")
message(STATUS "G�n�rateur: ${CMAKE_GENERATOR}")
message(STATUS "==========================================")

# Cibles personnalis�es pour le d�veloppement
add_custom_target(run
    COMMAND EPOBarcodeValidator
    DEPENDS EPOBarcodeValidator
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Lancement de l'application"
)

add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} --build . --target clean
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/Debug
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/Release
    COMMENT "Nettoyage complet"
)