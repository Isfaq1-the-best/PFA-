# 🪟 EPO Barcode Validator - Guide Windows

## 🚀 Système d'Importation d'Images - Version Windows

Bienvenue dans le nouveau système d'importation d'images intelligent pour **EPO Barcode Validator**, spécialement adapté pour Windows ! Cette version offre une expérience similaire à **Google Lens** pour l'analyse automatique des codes-barres EAN-13.

## 📋 Prérequis

### 1. Qt Framework
Vous devez installer Qt sur votre système Windows :

**Option A - Qt Open Source (Gratuit)**
- Téléchargez Qt depuis : [https://www.qt.io/download](https://www.qt.io/download)
- Choisissez "Qt Online Installer"
- Installez Qt 6.5+ avec MSVC 2019 ou MinGW
- **Important** : Ajoutez le dossier `bin` de Qt au PATH système
  - Exemple : `C:\Qt\6.5.0\msvc2019_64\bin`

**Option B - Qt via vcpkg (Avancé)**
```cmd
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
.\vcpkg install qt5[core,widgets,gui]:x64-windows
```

### 2. Compilateur C++
**Option A - Visual Studio (Recommandé)**
- Installez Visual Studio 2019/2022 Community (gratuit)
- Ou installez uniquement "Build Tools for Visual Studio"

**Option B - MinGW**
- Généralement inclus avec Qt
- Ou téléchargez depuis [MinGW-w64](https://www.mingw-w64.org/)

### 3. Python (pour les images de test)
- Python 3.8+ depuis [python.org](https://www.python.org/downloads/)
- Installez Pillow : `pip install Pillow`

## 🔨 Compilation

### Méthode 1 : Script Automatique (Recommandé)
```cmd
# Clonez ou téléchargez le projet
cd EPOBarcodeValidator

# Lancez le script de build
build_windows.bat
```

### Méthode 2 : Compilation Manuelle
```cmd
# Ouvrez l'invite de commande Visual Studio
# (Recherchez "Developer Command Prompt" dans le menu Démarrer)

cd EPOBarcodeValidator
qmake EPOBarcodeValidator.pro
nmake
```

### Méthode 3 : Qt Creator
1. Ouvrez Qt Creator
2. Fichier → Ouvrir un fichier ou un projet
3. Sélectionnez `EPOBarcodeValidator.pro`
4. Configurez le kit de développement
5. Cliquez sur "Build" (Ctrl+B)

## 🖼️ Génération d'Images de Test

Avant de tester l'application, générez des images de codes-barres :

```cmd
# Générer des images de test EAN-13
python generate_test_images_windows.py
```

Cela créera :
- `test_ean13_1_3760123456789.png`
- `test_ean13_2_8901234567890.png`
- `test_ean13_3_4567890123451.png`
- `test_multi_ean13.png`

## 🎯 Utilisation du Nouveau Système

### Interface Simplifiée
1. **Bouton "Importer"** : Sélectionnez une image contenant un code-barre EAN-13
2. **Analyse Automatique** : Plus besoin de "Convertir" puis "Décoder" !
3. **Feedback Visuel** : Barre de progression et statut en temps réel
4. **Résultats Instantanés** : Le code est automatiquement détecté et validé

### Fonctionnalités Avancées
- **🧠 Intelligence Multi-Technique** : 5 méthodes d'amélioration d'image
- **⚡ Arrêt Intelligent** : S'arrête dès qu'un code valide est trouvé
- **🎯 Spécialisé EAN-13** : Optimisé pour les codes-barres européens
- **🔄 Retry Automatique** : Essaie plusieurs approches automatiquement

## ⚙️ Configuration Avancée

### Avec ZXing (Optionnel)
Pour de meilleures performances de détection, vous pouvez installer ZXing :

**Via vcpkg :**
```cmd
vcpkg install zxing:x64-windows
```

Puis décommentez dans `EPOBarcodeValidator.pro` :
```pro
win32 {
    ZXING_PATH = C:/vcpkg/installed/x64-windows
    INCLUDEPATH += $$ZXING_PATH/include
    LIBS += -L$$ZXING_PATH/lib -lZXing
}
```

### Sans ZXing
L'application fonctionne parfaitement avec Qt uniquement ! Le système utilise alors :
- Traitement d'image Qt natif
- Algorithmes optimisés pour Windows
- Détection basique mais efficace des motifs EAN-13

## 🐛 Résolution de Problèmes

### Erreur : "qmake n'est pas reconnu"
```cmd
# Ajoutez Qt au PATH système
set PATH=%PATH%;C:\Qt\6.5.0\msvc2019_64\bin
```

### Erreur : "nmake n'est pas reconnu"
- Utilisez "Developer Command Prompt for Visual Studio"
- Ou installez Visual Studio Build Tools

### Erreur : "MSVCR120.dll manquant"
- Installez Visual C++ Redistributable
- Téléchargeable depuis le site Microsoft

### L'application ne démarre pas
```cmd
# Vérifiez les DLL Qt requises
windeployqt.exe EPOBarcodeValidator.exe
```

## 📁 Structure du Projet

```
EPOBarcodeValidator/
│
├── 📄 EPOBarcodeValidator.pro      # Configuration Qt
├── 🔨 build_windows.bat           # Script de compilation
├── 🐍 generate_test_images_windows.py  # Générateur d'images
├── 📖 README_WINDOWS.md           # Ce guide
│
├── 📁 src/
│   ├── MainWindow.h/cpp           # Interface principale
│   ├── BarcodeValidator.h/cpp     # Validation EAN-13
│   └── ...                       # Autres sources
│
└── 📁 images_test/               # Images générées
    ├── test_ean13_1_*.png
    └── test_multi_ean13.png
```

## 🎉 Test de l'Application

1. **Lancez l'application** :
   ```cmd
   # Depuis le dossier de compilation
   debug\EPOBarcodeValidator.exe
   # ou
   release\EPOBarcodeValidator.exe
   ```

2. **Testez l'importation** :
   - Cliquez sur "Importer une image"
   - Sélectionnez une des images de test générées
   - Observez l'analyse automatique en action !

3. **Vérifiez les résultats** :
   - Le code EAN-13 devrait être détecté automatiquement
   - La validation s'effectue en temps réel
   - L'historique est mis à jour automatiquement

## 🌟 Nouvelles Fonctionnalités

### ✨ Par rapport à l'ancienne version :
- ❌ **Suppression** des étapes manuelles "Convertir" → "Décoder"
- ✅ **Ajout** de l'analyse automatique intelligente
- ✅ **Ajout** de la barre de progression
- ✅ **Amélioration** des algorithmes de détection
- ✅ **Optimisation** pour Windows

### 🎯 Spécial EAN-13 :
- Détection optimisée pour les codes européens
- Validation automatique de la clé de contrôle
- Support des différents préfixes de pays
- Nettoyage automatique du code (suppression des caractères non numériques)

## 💡 Conseils d'Utilisation

### 📸 Pour de Meilleurs Résultats :
- **Éclairage** : Évitez les reflets et les ombres
- **Angle** : Prenez la photo bien droite
- **Distance** : Ni trop près, ni trop loin
- **Résolution** : 800x600 minimum recommandé
- **Format** : PNG, JPG, JPEG supportés

### 🖼️ Types d'Images Supportées :
- Photos de produits
- Scans de codes-barres
- Captures d'écran
- Images web (avec codes EAN-13)

## 🛠️ Support et Développement

- **Problèmes** : Créez une issue sur GitHub
- **Suggestions** : Proposez des améliorations
- **Contributions** : Les pull requests sont bienvenues !

---

🎉 **Profitez du nouveau système d'importation intelligent !** 🎉