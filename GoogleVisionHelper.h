#pragma once
#include <QImage>
#include <QString>

// Utilitaire pour appeler l'API Google Vision et extraire un code-barres EAN-13
namespace GoogleVisionHelper {
    // Retourne le code-barre EAN-13 détecté ou une QString vide si rien trouvé
    QString detectBarcodeWithGoogleVision(const QImage& image, const QString& apiKey);
}