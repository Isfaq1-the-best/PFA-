#!/usr/bin/env python3
"""
Script pour générer une image de test avec un code-barres EAN-13
pour tester le nouveau système d'importation amélioré
"""

import barcode
from barcode.writer import ImageWriter
from PIL import Image, ImageDraw, ImageFont
import os

def create_test_barcode_image():
    """Crée une image de test avec un code-barres EAN-13"""
    
    # Code EAN-13 valide pour test
    ean_code = "3760123456789"
    
    # Génération du code-barres
    ean = barcode.get_barcode_class('ean13')
    barcode_instance = ean(ean_code, writer=ImageWriter())
    
    # Options de rendu
    options = {
        'module_width': 0.5,
        'module_height': 15.0,
        'quiet_zone': 6.5,
        'font_size': 12,
        'text_distance': 5.0,
        'background': 'white',
        'foreground': 'black',
    }
    
    # Sauvegarde de l'image
    filename = 'test_barcode_ean13'
    barcode_instance.save(filename, options)
    
    print(f"Image de test créée: {filename}.png")
    print(f"Code EAN-13 utilisé: {ean_code}")
    print("")
    print("Instructions de test:")
    print("1. Lancez l'application EPOBarcodeValidator")
    print("2. Allez dans l'onglet 'Traitement d'images'")
    print("3. Cliquez sur 'Importer' et sélectionnez test_barcode_ean13.png")
    print("4. L'analyse devrait être automatique et détecter le code EAN-13")
    print("5. Le système utilisera plusieurs techniques d'amélioration d'image")
    print("6. Le résultat final devrait afficher le code détecté")

def create_challenging_test_image():
    """Crée une image plus difficile à analyser pour tester la robustesse"""
    
    # Code EAN-13 différent
    ean_code = "1234567890123"
    
    # Génération du code-barres
    ean = barcode.get_barcode_class('ean13')
    barcode_instance = ean(ean_code, writer=ImageWriter())
    
    # Options pour une image plus difficile
    options = {
        'module_width': 0.3,  # Plus fin
        'module_height': 10.0,  # Plus bas
        'quiet_zone': 3.0,  # Zone blanche réduite
        'font_size': 8,  # Police plus petite
        'text_distance': 2.0,
        'background': 'white',
        'foreground': 'black',
    }
    
    # Sauvegarde
    filename = 'test_barcode_challenging'
    barcode_instance.save(filename, options)
    
    # Ajout de bruit et de déformation pour tester la robustesse
    img = Image.open(f'{filename}.png')
    
    # Rotation légère
    img_rotated = img.rotate(2, fillcolor='white', expand=True)
    img_rotated.save('test_barcode_rotated.png')
    
    # Image plus petite (test de redimensionnement)
    img_small = img.resize((int(img.width * 0.5), int(img.height * 0.5)), Image.LANCZOS)
    img_small.save('test_barcode_small.png')
    
    print(f"Images de test difficiles créées:")
    print(f"- {filename}.png (code: {ean_code})")
    print(f"- test_barcode_rotated.png (légèrement tourné)")
    print(f"- test_barcode_small.png (résolution réduite)")

if __name__ == "__main__":
    try:
        # Vérification des dépendances
        import barcode
        from barcode.writer import ImageWriter
        from PIL import Image
        
        print("=== Génération d'images de test pour EPO Barcode Validator ===")
        print("")
        
        # Création des images de test
        create_test_barcode_image()
        print("")
        create_challenging_test_image()
        
        print("")
        print("=== Test du nouveau système d'importation ===")
        print("Nouvelles fonctionnalités implémentées:")
        print("✓ Processus automatique sans boutons séparés 'Convertir'/'Décoder'")
        print("✓ Techniques multiples d'amélioration d'image:")
        print("  - Flou gaussien pour réduire le bruit")
        print("  - Filtre de netteté pour améliorer les contours")
        print("  - Correction gamma pour l'exposition")
        print("  - Redimensionnement intelligent")
        print("✓ Barre de progression et indicateurs de statut")
        print("✓ Feedback visuel en temps réel")
        print("✓ Spécialisé pour les codes-barres EAN-13")
        
    except ImportError as e:
        print(f"Erreur: Module manquant - {e}")
        print("Installez les dépendances avec:")
        print("pip install python-barcode[images] pillow")