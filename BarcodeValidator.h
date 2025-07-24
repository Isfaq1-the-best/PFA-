#ifndef BARCODEVALIDATOR_H
#define BARCODEVALIDATOR_H

#include <QObject>
#include <QString>
#include <QRegularExpression>
#include <QThread>
#include <QMutex>
#include <QFileInfo>
#include <QImageReader>
#include <QImage>
#include <QPixmap>
#include <QDateTime>
#include <QDebug>

// OpenCV includes pour l'analyse d'images
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core.hpp>

// ZXing includes (optionnel)
// #include <ZXing/ReadBarcode.h>
// #include <ZXing/BarcodeFormat.h>

class BarcodeValidator : public QObject
{
    Q_OBJECT

public:
    explicit BarcodeValidator(QObject* parent = nullptr);
    ~BarcodeValidator();

    // Méthodes publiques
    void validateBarcode(const QString& barcode);
    void analyzeImage(const QString& imagePath);

    // Méthodes statiques utilitaires
    static bool isValidEAN13(const QString& barcode);
    static int calculateCheckDigit(const QString& barcode12);
    static QString extractBarcodeFromImage(const QString& imagePath);

signals:
    void barcodeValidated(const QString& barcode, bool isValid);
    void imageAnalysisProgress(int percentage);
    void imageAnalysisCompleted(const QString& barcode, bool found);

private slots:
    void onImageAnalysisFinished();

private:
    QThread* analysisThread;
    QMutex validationMutex;

    bool validateBarcodeFormat(const QString& barcode);
    bool validateChecksum(const QString& barcode);
    void processImageInThread(const QString& imagePath);
};

// Classe pour l'analyse d'image dans un thread séparé
class ImageAnalysisWorker : public QObject
{
    Q_OBJECT

public:
    explicit ImageAnalysisWorker(const QString& imagePath, QObject* parent = nullptr);

public slots:
    void process();

signals:
    void finished();
    void progress(int percentage);
    void barcodeFound(const QString& barcode);
    void error(const QString& errorMessage);

private:
    QString m_imagePath;

    QString performOCRAnalysis(const QString& imagePath);
    QString tryManualExtraction(const QImage& image);
    QStringList findBarcodePatterns(const QString& text);
    bool isLikelyBarcode(const QString& text);
};

#endif // BARCODEVALIDATOR_H