@echo off
echo =====================================
echo  EPO Barcode Validator - Build Script
echo =====================================
echo.

REM Vérifier si Qt est disponible
qmake --version >nul 2>&1
if errorlevel 1 (
    echo [ERREUR] Qt n'est pas installé ou pas dans le PATH
    echo.
    echo Veuillez installer Qt et ajouter qmake au PATH système
    echo Exemple: C:\Qt\6.5.0\msvc2019_64\bin
    pause
    exit /b 1
)

echo [INFO] Qt détecté
qmake --version

echo.
echo [INFO] Nettoyage des fichiers de build précédents...
if exist Makefile del Makefile
if exist *.obj del *.obj
if exist EPOBarcodeValidator.exe del EPOBarcodeValidator.exe
if exist debug rmdir /s /q debug
if exist release rmdir /s /q release

echo [INFO] Génération du Makefile...
qmake EPOBarcodeValidator.pro
if errorlevel 1 (
    echo [ERREUR] Échec de la génération du Makefile
    pause
    exit /b 1
)

echo [INFO] Compilation en cours...
nmake
if errorlevel 1 (
    echo [ERREUR] Échec de la compilation
    echo.
    echo Vérifiez que Visual Studio Build Tools est installé
    pause
    exit /b 1
)

echo.
echo [SUCCESS] Compilation réussie !

REM Chercher l'exécutable dans debug ou release
if exist debug\EPOBarcodeValidator.exe (
    echo [INFO] Exécutable créé : debug\EPOBarcodeValidator.exe
    set EXECUTABLE=debug\EPOBarcodeValidator.exe
) else if exist release\EPOBarcodeValidator.exe (
    echo [INFO] Exécutable créé : release\EPOBarcodeValidator.exe
    set EXECUTABLE=release\EPOBarcodeValidator.exe
) else if exist EPOBarcodeValidator.exe (
    echo [INFO] Exécutable créé : EPOBarcodeValidator.exe
    set EXECUTABLE=EPOBarcodeValidator.exe
) else (
    echo [ATTENTION] Exécutable non trouvé aux emplacements habituels
)

echo.
echo =====================================
echo  Build terminé avec succès !
echo =====================================
echo.
echo Pour lancer l'application :
if defined EXECUTABLE echo   %EXECUTABLE%
echo.
pause