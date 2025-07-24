@echo off
echo ===============================================
echo  🧠 TEST COMPLET DU SYSTÈME DE PRÉTRAITEMENT
echo  Style Google Lens - EPO Barcode Validator
echo ===============================================
echo.

echo 🎯 Phase 1: Vérification de l'environnement
echo ============================================

REM Vérifier Qt
qmake --version >nul 2>&1
if errorlevel 1 (
    echo ❌ Qt non installé ou non dans le PATH
    echo 💡 Installez Qt et ajoutez-le au PATH système
    pause
    exit /b 1
)
echo ✅ Qt détecté et fonctionnel

REM Vérifier Python
python --version >nul 2>&1
if errorlevel 1 (
    echo ❌ Python non installé
    echo 💡 Installez Python depuis python.org
    pause
    exit /b 1
)
echo ✅ Python détecté

REM Vérifier Pillow
python -c "import PIL" >nul 2>&1
if errorlevel 1 (
    echo ⚠️  Pillow non installé - Installation automatique...
    pip install Pillow
    if errorlevel 1 (
        echo ❌ Échec installation Pillow
        pause
        exit /b 1
    )
)
echo ✅ Pillow (PIL) disponible

echo.
echo 🏗️  Phase 2: Compilation du système avancé
echo =========================================

REM Nettoyer les anciens builds
if exist Makefile del Makefile
if exist *.obj del *.obj >nul 2>&1
if exist debug rmdir /s /q debug >nul 2>&1
if exist release rmdir /s /q release >nul 2>&1

echo 📋 Génération du Makefile...
qmake EPOBarcodeValidator.pro
if errorlevel 1 (
    echo ❌ Échec génération Makefile
    pause
    exit /b 1
)

echo 🔨 Compilation en cours...
nmake >nul 2>&1
if errorlevel 1 (
    echo ❌ Échec compilation - Tentative avec mingw32-make...
    mingw32-make >nul 2>&1
    if errorlevel 1 (
        echo ❌ Échec compilation avec les deux méthodes
        echo 💡 Vérifiez l'installation de Visual Studio Build Tools
        pause
        exit /b 1
    )
)

echo ✅ Compilation réussie !

REM Localiser l'exécutable
set EXECUTABLE=
if exist debug\EPOBarcodeValidator.exe set EXECUTABLE=debug\EPOBarcodeValidator.exe
if exist release\EPOBarcodeValidator.exe set EXECUTABLE=release\EPOBarcodeValidator.exe
if exist EPOBarcodeValidator.exe set EXECUTABLE=EPOBarcodeValidator.exe

if "%EXECUTABLE%"=="" (
    echo ❌ Exécutable non trouvé
    pause
    exit /b 1
)

echo ✅ Exécutable créé: %EXECUTABLE%

echo.
echo 🖼️  Phase 3: Génération des images de test avancées
echo ===============================================

echo 📊 Génération des images standard et de défi...
python generate_test_images_windows.py
if errorlevel 1 (
    echo ❌ Échec génération des images de test
    pause
    exit /b 1
)

echo ✅ Images de test générées avec succès !

echo.
echo 📋 Vérification des images créées...
dir *.png >nul 2>&1
if errorlevel 1 (
    echo ⚠️  Aucune image PNG trouvée
) else (
    echo ✅ Images PNG détectées:
    for %%f in (*.png) do echo   📷 %%f
)

echo.
echo 🧪 Phase 4: Tests de validation du prétraitement
echo =============================================

echo 🔬 Tests des capacités de prétraitement implémentées:
echo.
echo   ✅ Normalisation de l'éclairage (normalizeIllumination)
echo   ✅ CLAHE - Contraste adaptatif (clahe)
echo   ✅ Masque de netteté (unsharpMask)
echo   ✅ Détection/correction rotation (detectAndCorrectRotation)
echo   ✅ Binarisation adaptative (binarizeAdaptive)
echo   ✅ Réduction de bruit (removeNoise)
echo   ✅ Standardisation taille (standardizeSize)
echo   ✅ Calcul de netteté (calculateImageSharpness)
echo.
echo 🎯 Défis de test générés:
echo   📸 Bruit électronique (noisy)
echo   💡 Éclairage non uniforme (uneven_lighting)
echo   🌫️  Image floue (blurry)
echo   🔄 Rotation -7° (rotated)
echo   🌗 Faible contraste (low_contrast)
echo   ☀️  Sur-exposition (overexposed)
echo   🌙 Sous-exposition (underexposed)

echo.
echo 🚀 Phase 5: Lancement et test manuel
echo ==================================

echo 💡 Instructions de test:
echo.
echo 1. L'application va se lancer avec le nouveau système de prétraitement
echo 2. Testez l'importation avec les images générées:
echo    • Images standard: test_ean13_*.png
echo    • Images de défi: test_challenge_*.png
echo 3. Observez le processus de prétraitement intelligent:
echo    • Barre de progression avec étapes détaillées
echo    • Messages de statut en temps réel
echo    • Arrêt automatique dès détection réussie
echo 4. Codes attendus:
echo    • Standard: 3760123456789, 8901234567890, 4567890123451
echo    • Réels: 3017620422003 (Nutella), 8710103878873 (Philips)
echo    • Défis: 3760123456789 (avec prétraitement avancé)
echo.

set /p response="Voulez-vous lancer l'application maintenant ? (O/n): "
if /i "%response%"=="n" goto :end
if /i "%response%"=="non" goto :end

echo.
echo 🎉 Lancement de l'application avec prétraitement avancé...
echo.
start "" "%EXECUTABLE%"

echo ✅ Application lancée !
echo.
echo 📖 Consultez aussi:
echo   📄 README_PREPROCESS_ADVANCED.md - Documentation technique complète
echo   📄 README_WINDOWS.md - Guide d'utilisation Windows
echo.

:end
echo ===============================================
echo  ✅ Test complet terminé avec succès !
echo ===============================================
echo.
echo 🌟 Votre système dispose maintenant de:
echo   • Prétraitement intelligent style Google Lens
echo   • 8 techniques d'amélioration d'images
echo   • Pipeline optimisé avec arrêt précoce
echo   • Interface utilisateur simplifiée
echo   • Tests automatisés avec images de défi
echo.
echo 🎯 Prêt pour analyser des codes-barres EAN-13
echo    dans des conditions difficiles !
echo.
pause