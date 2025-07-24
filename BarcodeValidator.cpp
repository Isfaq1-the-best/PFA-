#include "BarcodeValidator.h"
#include <QDebug>
#include <QPixmap>
#include <QImageReader>
#include <QRegularExpressionMatch>
#include <QTimer>

BarcodeValidator::BarcodeValidator(QObject* parent)
    : QObject(parent), analysisThread(nullptr)
{
}

BarcodeValidator::~BarcodeValidator()
{
    if (analysisThread && analysisThread->isRunning()) {
        analysisThread->quit();
        analysisThread->wait();
    }
}

void BarcodeValidator::validateBarcode(const QString& barcode)
{
    QMutexLocker locker(&validationMutex);

    bool isValid = isValidEAN13(barcode);
    emit barcodeValidated(barcode, isValid);
}

void BarcodeValidator::analyzeImage(const QString& imagePath)
{
    if (imagePath.isEmpty()) {
        emit imageAnalysisCompleted("", false);
        return;
    }

    // Vérifier si l'image existe et est lisible
    QFileInfo fileInfo(imagePath);
    if (!fileInfo.exists()) {
        emit barcodeValidated("", false);
        return;
    }

    QImageReader reader(imagePath);
    if (!reader.canRead()) {
        emit barcodeValidated("", false);
        return;
    }

    // Charger l'image pour vérification
    QImage image = reader.read();
    if (image.isNull()) {
        emit barcodeValidated("", false);
        return;
    }

    emit imageAnalysisProgress(25);

    // Créer un thread pour l'analyse si nécessaire
    if (analysisThread && analysisThread->isRunning()) {
        analysisThread->quit();
        analysisThread->wait(3000);
        if (analysisThread->isRunning()) {
            analysisThread->terminate();
            analysisThread->wait(1000);
        }
    }

    analysisThread = new QThread();
    ImageAnalysisWorker* worker = new ImageAnalysisWorker(imagePath);
    worker->moveToThread(analysisThread);

    // Connexions pour le thread
    connect(analysisThread, &QThread::started, worker, &ImageAnalysisWorker::process);
    connect(worker, &ImageAnalysisWorker::finished, this, &BarcodeValidator::onImageAnalysisFinished);
    connect(worker, &ImageAnalysisWorker::finished, analysisThread, &QThread::quit);
    connect(worker, &ImageAnalysisWorker::finished, worker, &ImageAnalysisWorker::deleteLater);
    connect(analysisThread, &QThread::finished, [this]() {
        if (analysisThread) {
            analysisThread->deleteLater();
            analysisThread = nullptr;
        }
        });

    connect(worker, &ImageAnalysisWorker::progress,
        this, &BarcodeValidator::imageAnalysisProgress);
    connect(worker, &ImageAnalysisWorker::barcodeFound,
        [this](const QString& barcode) {
            if (!barcode.isEmpty()) {
                bool isValid = isValidEAN13(barcode);
                emit barcodeValidated(barcode, isValid);
                emit imageAnalysisCompleted(barcode, true);
            }
            else {
                emit barcodeValidated("", false);
                emit imageAnalysisCompleted("", false);
            }
        });
    connect(worker, &ImageAnalysisWorker::error,
        [this](const QString& error) {
            qDebug() << "Erreur d'analyse d'image:" << error;
            emit barcodeValidated("", false);
            emit imageAnalysisCompleted("", false);
        });

    emit imageAnalysisProgress(50);
    analysisThread->start();
}

bool BarcodeValidator::isValidEAN13(const QString& barcode)
{
    // Vérifier le format
    if (barcode.length() != 13) {
        return false;
    }

    // Vérifier que tous les caractères sont des chiffres
    static const QRegularExpression digitRegex("^\\d{13}$");
    QRegularExpressionMatch match = digitRegex.match(barcode);
    if (!match.hasMatch()) {
        return false;
    }

    // Vérifier la clé de contrôle
    QString first12 = barcode.left(12);
    int calculatedCheckDigit = calculateCheckDigit(first12);
    int providedCheckDigit = barcode.right(1).toInt();

    return calculatedCheckDigit == providedCheckDigit;
}

int BarcodeValidator::calculateCheckDigit(const QString& barcode12)
{
    if (barcode12.length() != 12) {
        return -1;
    }

    int sum = 0;
    for (int i = 0; i < 12; ++i) {
        int digit = barcode12.at(i).digitValue();
        if (i % 2 == 0) {
            sum += digit;  // Position impaire (1, 3, 5, ...)
        }
        else {
            sum += digit * 3;  // Position paire (2, 4, 6, ...)
        }
    }

    int remainder = sum % 10;
    return (remainder == 0) ? 0 : (10 - remainder);
}

QString BarcodeValidator::extractBarcodeFromImage(const QString& imagePath)
{
    // Cette méthode devrait utiliser une bibliothèque OCR comme Tesseract
    // Pour la démonstration, nous simulons l'extraction
    Q_UNUSED(imagePath)

        // Simulation d'un code-barre extrait
        // Dans une vraie implémentation, vous utiliseriez Tesseract ou ZBar
        return "1234567890123";  // Code-barre de test
}

void BarcodeValidator::onImageAnalysisFinished()
{
    emit imageAnalysisCompleted("", false);
}

bool BarcodeValidator::validateBarcodeFormat(const QString& barcode)
{
    return barcode.length() == 13 &&
        QRegularExpression("^\\d{13}$").match(barcode).hasMatch();
}

bool BarcodeValidator::validateChecksum(const QString& barcode)
{
    return isValidEAN13(barcode);
}

// Implémentation de ImageAnalysisWorker
ImageAnalysisWorker::ImageAnalysisWorker(const QString& imagePath, QObject* parent)
    : QObject(parent), m_imagePath(imagePath)
{
}

void ImageAnalysisWorker::process()
{
    emit progress(20);

    // Vérifier que le fichier existe
    QFileInfo fileInfo(m_imagePath);
    if (!fileInfo.exists()) {
        emit error("Fichier image introuvable: " + m_imagePath);
        emit finished();
        return;
    }

    emit progress(40);

    // Charger l'image
    QImage image(m_imagePath);
    if (image.isNull()) {
        emit error("Impossible de charger l'image. Format non supporté.");
        emit finished();
        return;
    }

    emit progress(60);

    // Analyser l'image pour trouver des codes-barres
    QString extractedText = performOCRAnalysis(m_imagePath);
    emit progress(75);

    // Rechercher des motifs de codes-barres
    QStringList possibleBarcodes = findBarcodePatterns(extractedText);
    emit progress(90);

    // Valider les codes-barres trouvés
    for (const QString& barcode : possibleBarcodes) {
        if (BarcodeValidator::isValidEAN13(barcode)) {
            emit progress(100);
            emit barcodeFound(barcode);
            emit finished();
            return;
        }
    }

    // Si aucun code-barre valide trouvé, essayer une analyse plus approfondie
    QString manualBarcode = tryManualExtraction(image);
    if (!manualBarcode.isEmpty() && BarcodeValidator::isValidEAN13(manualBarcode)) {
        emit progress(100);
        emit barcodeFound(manualBarcode);
        emit finished();
        return;
    }

    emit progress(100);
    emit error("Aucun code-barre EAN-13 valide détecté dans l'image.\n"
        "Assurez-vous que :\n"
        "• L'image contient un code-barre visible\n"
        "• Le code-barre est net et bien éclairé\n"
        "• Il s'agit d'un code-barre EAN-13 (13 chiffres)");
    emit finished();
}

QString ImageAnalysisWorker::performOCRAnalysis(const QString& imagePath)
{
    // Charger l'image avec OpenCV pour l'analyse
    cv::Mat image = cv::imread(imagePath.toStdString());
    if (image.empty()) {
        return QString();
    }

    // Convertir en niveaux de gris
    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

    // Améliorer le contraste
    cv::Mat enhanced;
    cv::equalizeHist(gray, enhanced);

    // Appliquer un seuillage adaptatif
    cv::Mat binary;
    cv::adaptiveThreshold(enhanced, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 11, 2);

    // Dans une vraie implémentation, vous utiliseriez Tesseract OCR ici
    // Pour la démonstration, on simule l'extraction de texte

    // Simuler différents résultats possibles basés sur l'analyse de l'image
    QStringList simulatedResults = {
        "Produit: 1234567890128 Prix: 15.99€",  // Code-barre valide dans du texte
        "EAN: 3141592653589 Marque: Test",      // Autre code-barre valide
        "Code: 9876543210987 Description",       // Code-barre invalide
        "Référence 8901234567890 Stock: 25",     // Code-barre valide
        "texte sans code-barre visible"          // Pas de code-barre
    };

    // Choisir un résultat basé sur une caractéristique de l'image (taille par exemple)
    int index = (image.rows + image.cols) % simulatedResults.size();
    return simulatedResults[index];
}

QString ImageAnalysisWorker::tryManualExtraction(const QImage& image)
{
    // Méthode alternative pour essayer d'extraire un code-barre
    // En analysant les patterns de l'image

    if (image.isNull()) {
        return QString();
    }

    // Convertir en OpenCV Mat
    cv::Mat mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
    cv::Mat gray;
    cv::cvtColor(mat, gray, cv::COLOR_BGRA2GRAY);

    // Rechercher des patterns de barres (lignes verticales répétitives)
    cv::Mat edges;
    cv::Canny(gray, edges, 50, 150);

    // Dans une vraie implémentation, vous analyseriez les contours
    // pour détecter des patterns de codes-barres

    // Pour la démonstration, retourner un code-barre de test si l'image semble contenir des barres
    cv::Scalar meanValue = cv::mean(edges);
    if (meanValue[0] > 10) { // Si suffisamment de contours détectés
        return "1234567890128"; // Code-barre de test valide
    }

    return QString();
}

QStringList ImageAnalysisWorker::findBarcodePatterns(const QString& text)
{
    QStringList barcodes;

    // Rechercher des séquences de 13 chiffres
    QRegularExpression barcodeRegex("\\b\\d{13}\\b");
    QRegularExpressionMatchIterator iterator = barcodeRegex.globalMatch(text);

    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        QString barcode = match.captured(0);
        if (isLikelyBarcode(barcode)) {
            barcodes.append(barcode);
        }
    }

    return barcodes;
}

bool ImageAnalysisWorker::isLikelyBarcode(const QString& text)
{
    // Vérifications basiques pour déterminer si c'est probablement un code-barre
    if (text.length() != 13) {
        return false;
    }

    // Vérifier que ce ne sont pas tous les mêmes chiffres
    QChar firstChar = text.at(0);
    bool allSame = true;
    for (int i = 1; i < text.length(); ++i) {
        if (text.at(i) != firstChar) {
            allSame = false;
            break;
        }
    }

    return !allSame;
}