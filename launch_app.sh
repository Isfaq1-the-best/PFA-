#!/bin/bash

echo "=== EPO Barcode Validator - Système d'Importation Amélioré ==="
echo ""

# Vérifier si l'application existe
if [ ! -f "./EPOBarcodeValidator" ]; then
    echo "🔨 Compilation de l'application..."
    make clean && qmake EPOBarcodeValidator.pro && make
    if [ $? -ne 0 ]; then
        echo "❌ Erreur lors de la compilation"
        exit 1
    fi
    echo "✅ Compilation réussie"
fi

# Générer les images de test si elles n'existent pas
if [ ! -f "test_barcode_ean13.png" ]; then
    echo "🖼️ Génération des images de test..."
    python3 test_image_import.py
    echo "✅ Images de test générées"
fi

echo ""
echo "🚀 Lancement de l'application..."
echo ""
echo "📋 Images de test disponibles :"
echo "   • test_barcode_ean13.png (Code: 3760123456789)"
echo "   • test_barcode_challenging.png (Code: 1234567890123)"
echo "   • test_barcode_rotated.png (tourné)"
echo "   • test_barcode_small.png (petite résolution)"
echo ""
echo "📖 Instructions :"
echo "   1. Allez dans l'onglet 'Traitement d'images'"
echo "   2. Cliquez sur 'Importer' et sélectionnez une image de test"
echo "   3. L'analyse se fait automatiquement avec feedback visuel"
echo "   4. Le résultat s'affiche instantanément"
echo ""

# Lancer l'application
./EPOBarcodeValidator

echo ""
echo "🏁 Application fermée"