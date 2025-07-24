#!/usr/bin/env python3
"""
Script pour générer des images de test avec codes-barres EAN-13
Compatible Windows - Version simplifiée utilisant PIL uniquement
"""

from PIL import Image, ImageDraw, ImageFont
import os

def create_simple_ean13_barcode(code):
    """Crée une représentation simple d'un code-barre EAN-13"""
    
    # Motifs pour les chiffres EAN-13 (version simplifiée)
    # Partie gauche (groupe A et B)
    left_patterns = {
        'A': {
            '0': '0001101', '1': '0011001', '2': '0010011', '3': '0111101', '4': '0100011',
            '5': '0110001', '6': '0101111', '7': '0111011', '8': '0110111', '9': '0001011'
        },
        'B': {
            '0': '0100111', '1': '0110011', '2': '0011011', '3': '0100001', '4': '0011101',
            '5': '0111001', '6': '0000101', '7': '0010001', '8': '0001001', '9': '0010111'
        }
    }
    
    # Partie droite (groupe C)
    right_patterns = {
        '0': '1110010', '1': '1100110', '2': '1101100', '3': '1000010', '4': '1011100',
        '5': '1001110', '6': '1010000', '7': '1000100', '8': '1001000', '9': '1110100'
    }
    
    # Motifs pour déterminer le groupe (A ou B) pour la partie gauche
    first_digit_patterns = {
        '0': 'AAAAAA', '1': 'AABABB', '2': 'AABBAB', '3': 'AABBBA', '4': 'ABAABB',
        '5': 'ABBAAB', '6': 'ABBBAA', '7': 'ABABAB', '8': 'ABABBA', '9': 'ABBABA'
    }
    
    if len(code) != 13:
        raise ValueError("Le code EAN-13 doit contenir exactement 13 chiffres")
    
    # Construire le motif binaire
    binary_code = ""
    
    # Garde de début
    binary_code += "101"
    
    # Premier chiffre détermine le motif pour les 6 suivants
    first_digit = code[0]
    pattern = first_digit_patterns[first_digit]
    
    # 6 chiffres de gauche (positions 1-6)
    for i in range(6):
        digit = code[i + 1]
        group = pattern[i]
        binary_code += left_patterns[group][digit]
    
    # Séparateur central
    binary_code += "01010"
    
    # 6 chiffres de droite (positions 7-12)
    for i in range(6):
        digit = code[i + 7]
        binary_code += right_patterns[digit]
    
    # Garde de fin
    binary_code += "101"
    
    return binary_code

def create_barcode_image(code, width=600, height=200):
    """Crée une image de code-barre à partir du motif binaire"""
    
    binary_pattern = create_simple_ean13_barcode(code)
    
    # Calculer la largeur des barres
    bar_width = width // len(binary_pattern)
    actual_width = bar_width * len(binary_pattern)
    
    # Créer l'image
    image = Image.new('RGB', (actual_width, height), 'white')
    draw = ImageDraw.Draw(image)
    
    # Dessiner les barres
    for i, bit in enumerate(binary_pattern):
        if bit == '1':  # Barre noire
            x = i * bar_width
            draw.rectangle([x, 0, x + bar_width - 1, height - 30], fill='black')
    
    # Ajouter le texte du code
    try:
        # Essayer d'utiliser une police système
        font = ImageFont.truetype("arial.ttf", 20)
    except:
        # Police par défaut si arial n'est pas disponible
        font = ImageFont.load_default()
    
    # Formater le code pour l'affichage (groupes de chiffres)
    formatted_code = f"{code[0]} {code[1:7]} {code[7:13]}"
    
    # Calculer la position du texte (centré)
    text_bbox = draw.textbbox((0, 0), formatted_code, font=font)
    text_width = text_bbox[2] - text_bbox[0]
    text_x = (actual_width - text_width) // 2
    text_y = height - 25
    
    draw.text((text_x, text_y), formatted_code, fill='black', font=font)
    
    return image

def create_test_images():
    """Crée plusieurs images de test avec différents codes EAN-13"""
    
    test_codes = [
        ("3760123456789", "Code EAN-13 standard"),
        ("8901234567890", "Code avec différent préfixe"),
        ("4567890123451", "Code de test boutique")
    ]
    
    print("Génération des images de test pour Windows...")
    
    for i, (code, description) in enumerate(test_codes, 1):
        print(f"  Création de l'image {i}: {code} ({description})")
        
        # Créer l'image de base
        barcode_img = create_barcode_image(code)
        
        # Créer une image plus grande avec du contexte
        context_img = Image.new('RGB', (800, 400), 'white')
        
        # Coller le code-barre au centre
        barcode_width, barcode_height = barcode_img.size
        x = (800 - barcode_width) // 2
        y = (400 - barcode_height) // 2
        context_img.paste(barcode_img, (x, y))
        
        # Ajouter du texte descriptif
        draw = ImageDraw.Draw(context_img)
        try:
            title_font = ImageFont.truetype("arial.ttf", 24)
            desc_font = ImageFont.truetype("arial.ttf", 16)
        except:
            title_font = ImageFont.load_default()
            desc_font = ImageFont.load_default()
        
        # Titre
        title = f"Test EAN-13 #{i}"
        title_bbox = draw.textbbox((0, 0), title, font=title_font)
        title_width = title_bbox[2] - title_bbox[0]
        draw.text(((800 - title_width) // 2, 50), title, fill='black', font=title_font)
        
        # Description
        desc_bbox = draw.textbbox((0, 0), description, font=desc_font)
        desc_width = desc_bbox[2] - desc_bbox[0]
        draw.text(((800 - desc_width) // 2, 80), description, fill='gray', font=desc_font)
        
        # Sauvegarder
        filename = f"test_ean13_{i}_{code}.png"
        context_img.save(filename)
        print(f"    ✓ Sauvegardé: {filename}")
    
    # Créer aussi une image avec plusieurs codes-barres
    print("  Création d'une image multi-codes...")
    multi_img = Image.new('RGB', (1000, 800), 'white')
    draw = ImageDraw.Draw(multi_img)
    
    # Titre principal
    try:
        main_font = ImageFont.truetype("arial.ttf", 28)
    except:
        main_font = ImageFont.load_default()
    
    title = "Tests d'Importation - Codes EAN-13"
    title_bbox = draw.textbbox((0, 0), title, font=main_font)
    title_width = title_bbox[2] - title_bbox[0]
    draw.text(((1000 - title_width) // 2, 30), title, fill='black', font=main_font)
    
    # Placer les codes-barres
    y_positions = [150, 350, 550]
    for i, (code, desc) in enumerate(test_codes):
        if i < len(y_positions):
            barcode_img = create_barcode_image(code, width=400, height=120)
            x = (1000 - 400) // 2
            y = y_positions[i]
            multi_img.paste(barcode_img, (x, y))
    
    multi_img.save("test_multi_ean13.png")
    print("    ✓ Sauvegardé: test_multi_ean13.png")
    
    print("\n✅ Images de test créées avec succès !")
    print("\nImages générées :")
    for i, (code, _) in enumerate(test_codes, 1):
        print(f"  • test_ean13_{i}_{code}.png")
    print("  • test_multi_ean13.png")
    
    print("\n💡 Utilisez ces images pour tester l'importation dans l'application EPO Barcode Validator")

if __name__ == "__main__":
    try:
        create_test_images()
    except Exception as e:
        print(f"❌ Erreur lors de la génération: {e}")
        input("Appuyez sur Entrée pour continuer...")