#ifndef BARCODEIMAGEWIDGET_H
#define BARCODEIMAGEWIDGET_H

#include <QLabel>
#include <QPixmap>
#include <QImage>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QRect>
#include <QPropertyAnimation>
#include <QTimer>
#include <QMouseEvent>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QFont>
#include <QFontMetrics>

/**
 * Widget d'affichage d'images avec détection visuelle de codes-barres
 * Style Google Lens avec cadre dynamique et animations
 */
class BarcodeImageWidget : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(int scanLinePosition READ scanLinePosition WRITE setScanLinePosition)
    Q_PROPERTY(qreal highlightOpacity READ highlightOpacity WRITE setHighlightOpacity)

public:
    explicit BarcodeImageWidget(QWidget *parent = nullptr);
    ~BarcodeImageWidget();

    // Gestion des images
    void setImageWithBarcode(const QImage& image, const QRect& barcodeRect = QRect(), const QString& detectedCode = "");
    void clearImage();
    void setAnalysisMode(bool analyzing);
    
    // Gestion de la détection
    void showBarcodeDetection(const QRect& barcodeRect, const QString& code, bool success = true);
    void hideBarcodeDetection();
    void startScanAnimation();
    void stopScanAnimation();
    
    // État
    bool hasDetectedBarcode() const { return m_hasDetection; }
    QRect detectedBarcodeRect() const { return m_barcodeRect; }
    QString detectedCode() const { return m_detectedCode; }

public slots:
    void setScanLinePosition(int position);
    void setHighlightOpacity(qreal opacity);
    void onAnalysisProgress(int percentage, const QString& status);

signals:
    void barcodeClicked(const QRect& rect);
    void imageClicked(const QPoint& position);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateScanAnimation();
    void pulseHighlight();

private:
    void drawBarcodeFrame(QPainter& painter);
    void drawCornerMarkers(QPainter& painter, const QRect& rect, const QColor& color);
    void drawScanLine(QPainter& painter);
    void drawDetectionInfo(QPainter& painter);
    void drawAnalysisOverlay(QPainter& painter);
    void calculateBarcodeDisplayRect();
    QRect scaleBarcodeRect(const QRect& originalRect) const;
    
    // Image et détection
    QImage m_originalImage;
    QPixmap m_scaledPixmap;
    QRect m_imageDisplayRect;
    QRect m_barcodeRect;          // Rectangle du code-barre dans l'image originale
    QRect m_barcodeDisplayRect;   // Rectangle adapté à l'affichage
    QString m_detectedCode;
    bool m_hasDetection;
    bool m_detectionSuccess;
    
    // Animation et effets visuels
    QTimer* m_scanTimer;
    QPropertyAnimation* m_scanAnimation;
    QPropertyAnimation* m_highlightAnimation;
    int m_scanLinePosition;
    qreal m_highlightOpacity;
    bool m_isAnalyzing;
    
    // Analyse en cours
    int m_analysisProgress;
    QString m_analysisStatus;
    
    // Style et couleurs
    QColor m_successColor;
    QColor m_processingColor;
    QColor m_scanLineColor;
    QFont m_infoFont;
    
    // Constantes de style
    static const int FRAME_THICKNESS = 3;
    static const int CORNER_RADIUS = 8;
    static const int INFO_MARGIN = 10;
    static const int SCAN_LINE_HEIGHT = 2;
    static const int ANIMATION_DURATION = 2000;
};

#endif // BARCODEIMAGEWIDGET_H