#include "BarcodeImageWidget.h"
#include <QPaintEvent>
#include <QDebug>
#include <QApplication>

BarcodeImageWidget::BarcodeImageWidget(QWidget *parent)
    : QLabel(parent)
    , m_hasDetection(false)
    , m_detectionSuccess(false)
    , m_scanLinePosition(0)
    , m_highlightOpacity(0.0)
    , m_isAnalyzing(false)
    , m_analysisProgress(0)
    , m_scanTimer(nullptr)
    , m_scanAnimation(nullptr)
    , m_highlightAnimation(nullptr)
{
    // Configuration de base
    setMinimumSize(400, 300);
    setMaximumSize(800, 600);
    setScaledContents(false);
    setAlignment(Qt::AlignCenter);
    
    // Style du widget
    setStyleSheet(
        "QLabel {"
        "border: 2px dashed #2E86AB;"
        "border-radius: 8px;"
        "background-color: #f8f9fa;"
        "color: #666;"
        "font-size: 14px;"
        "}"
    );
    
    // Texte par défaut
    setText("Aucune image chargée\n\nUtilisez le bouton 'Importer' pour charger une image");
    
    // Couleurs du thème Google Lens
    m_successColor = QColor(76, 175, 80);      // Vert Google
    m_processingColor = QColor(33, 150, 243);  // Bleu Google
    m_scanLineColor = QColor(255, 193, 7);     // Jaune/Orange Google
    
    // Police pour les informations
    m_infoFont = QFont("Arial", 10, QFont::Bold);
    
    // Timer pour l'animation de scan
    m_scanTimer = new QTimer(this);
    connect(m_scanTimer, &QTimer::timeout, this, &BarcodeImageWidget::updateScanAnimation);
    
    // Animation de la ligne de scan
    m_scanAnimation = new QPropertyAnimation(this, "scanLinePosition");
    m_scanAnimation->setDuration(ANIMATION_DURATION);
    m_scanAnimation->setLoopCount(-1); // Boucle infinie
    
    // Animation du highlight (pulsation)
    m_highlightAnimation = new QPropertyAnimation(this, "highlightOpacity");
    m_highlightAnimation->setDuration(1000);
    m_highlightAnimation->setKeyValueAt(0.0, 0.3);
    m_highlightAnimation->setKeyValueAt(0.5, 0.8);
    m_highlightAnimation->setKeyValueAt(1.0, 0.3);
    m_highlightAnimation->setLoopCount(-1);
}

BarcodeImageWidget::~BarcodeImageWidget()
{
    stopScanAnimation();
}

void BarcodeImageWidget::setImageWithBarcode(const QImage& image, const QRect& barcodeRect, const QString& detectedCode)
{
    if (image.isNull()) {
        clearImage();
        return;
    }
    
    m_originalImage = image;
    m_barcodeRect = barcodeRect;
    m_detectedCode = detectedCode;
    m_hasDetection = !barcodeRect.isEmpty();
    m_detectionSuccess = m_hasDetection && !detectedCode.isEmpty();
    
    // Créer le pixmap mis à l'échelle
    m_scaledPixmap = QPixmap::fromImage(image).scaled(
        size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    
    // Calculer la position d'affichage de l'image
    calculateBarcodeDisplayRect();
    
    // Cacher le texte par défaut
    clear();
    
    // Démarrer l'animation de highlight si détection réussie
    if (m_detectionSuccess) {
        m_highlightAnimation->start();
    }
    
    update();
}

void BarcodeImageWidget::clearImage()
{
    m_originalImage = QImage();
    m_scaledPixmap = QPixmap();
    m_barcodeRect = QRect();
    m_barcodeDisplayRect = QRect();
    m_detectedCode.clear();
    m_hasDetection = false;
    m_detectionSuccess = false;
    m_isAnalyzing = false;
    
    stopScanAnimation();
    
    // Remettre le texte par défaut
    setText("Aucune image chargée\n\nUtilisez le bouton 'Importer' pour charger une image");
    
    update();
}

void BarcodeImageWidget::setAnalysisMode(bool analyzing)
{
    m_isAnalyzing = analyzing;
    
    if (analyzing) {
        startScanAnimation();
    } else {
        stopScanAnimation();
    }
    
    update();
}

void BarcodeImageWidget::showBarcodeDetection(const QRect& barcodeRect, const QString& code, bool success)
{
    m_barcodeRect = barcodeRect;
    m_detectedCode = code;
    m_hasDetection = !barcodeRect.isEmpty();
    m_detectionSuccess = success && !code.isEmpty();
    
    calculateBarcodeDisplayRect();
    
    // Animation de succès
    if (m_detectionSuccess) {
        m_highlightAnimation->start();
    }
    
    update();
    
    qDebug() << "🎯 Détection affichée:" << code << "à" << barcodeRect;
}

void BarcodeImageWidget::hideBarcodeDetection()
{
    m_hasDetection = false;
    m_detectionSuccess = false;
    m_barcodeRect = QRect();
    m_barcodeDisplayRect = QRect();
    m_detectedCode.clear();
    
    if (m_highlightAnimation->state() == QAbstractAnimation::Running) {
        m_highlightAnimation->stop();
    }
    
    update();
}

void BarcodeImageWidget::startScanAnimation()
{
    if (!m_originalImage.isNull() && m_isAnalyzing) {
        // Configurer l'animation de scan sur toute la hauteur de l'image
        m_scanAnimation->setStartValue(0);
        m_scanAnimation->setEndValue(height());
        m_scanAnimation->start();
        
        qDebug() << "🔍 Animation de scan démarrée";
    }
}

void BarcodeImageWidget::stopScanAnimation()
{
    if (m_scanAnimation->state() == QAbstractAnimation::Running) {
        m_scanAnimation->stop();
    }
    
    if (m_highlightAnimation->state() == QAbstractAnimation::Running) {
        m_highlightAnimation->stop();
    }
    
    m_scanLinePosition = 0;
    m_highlightOpacity = 0.0;
    
    update();
}

void BarcodeImageWidget::setScanLinePosition(int position)
{
    m_scanLinePosition = position;
    update();
}

void BarcodeImageWidget::setHighlightOpacity(qreal opacity)
{
    m_highlightOpacity = opacity;
    update();
}

void BarcodeImageWidget::onAnalysisProgress(int percentage, const QString& status)
{
    m_analysisProgress = percentage;
    m_analysisStatus = status;
    update();
}

void BarcodeImageWidget::paintEvent(QPaintEvent *event)
{
    if (m_originalImage.isNull()) {
        // Affichage par défaut
        QLabel::paintEvent(event);
        return;
    }
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Dessiner l'image
    if (!m_scaledPixmap.isNull()) {
        QRect imageRect = QRect(
            (width() - m_scaledPixmap.width()) / 2,
            (height() - m_scaledPixmap.height()) / 2,
            m_scaledPixmap.width(),
            m_scaledPixmap.height()
        );
        
        m_imageDisplayRect = imageRect;
        painter.drawPixmap(imageRect, m_scaledPixmap);
    }
    
    // Overlay d'analyse si en cours
    if (m_isAnalyzing) {
        drawAnalysisOverlay(painter);
        drawScanLine(painter);
    }
    
    // Cadre de détection du code-barre
    if (m_hasDetection) {
        drawBarcodeFrame(painter);
        drawDetectionInfo(painter);
    }
}

void BarcodeImageWidget::drawBarcodeFrame(QPainter& painter)
{
    if (m_barcodeDisplayRect.isEmpty()) return;
    
    painter.save();
    
    // Couleur selon le succès de la détection
    QColor frameColor = m_detectionSuccess ? m_successColor : m_processingColor;
    
    // Effet de pulsation si succès
    if (m_detectionSuccess && m_highlightOpacity > 0) {
        frameColor.setAlphaF(m_highlightOpacity);
    }
    
    // Cadre principal style Google Lens
    QPen framePen(frameColor, FRAME_THICKNESS);
    framePen.setStyle(Qt::SolidLine);
    painter.setPen(framePen);
    
    // Rectangle arrondi
    painter.drawRoundedRect(m_barcodeDisplayRect, CORNER_RADIUS, CORNER_RADIUS);
    
    // Coins caractéristiques Google Lens
    drawCornerMarkers(painter, m_barcodeDisplayRect, frameColor);
    
    // Fond semi-transparent
    if (m_detectionSuccess) {
        QColor bgColor = frameColor;
        bgColor.setAlpha(30);
        painter.fillRect(m_barcodeDisplayRect, bgColor);
    }
    
    painter.restore();
}

void BarcodeImageWidget::drawCornerMarkers(QPainter& painter, const QRect& rect, const QColor& color)
{
    painter.save();
    
    QPen markerPen(color, FRAME_THICKNESS + 1);
    painter.setPen(markerPen);
    
    int cornerLength = 20;
    
    // Coin supérieur gauche
    painter.drawLine(rect.left(), rect.top() + cornerLength, rect.left(), rect.top());
    painter.drawLine(rect.left(), rect.top(), rect.left() + cornerLength, rect.top());
    
    // Coin supérieur droit
    painter.drawLine(rect.right() - cornerLength, rect.top(), rect.right(), rect.top());
    painter.drawLine(rect.right(), rect.top(), rect.right(), rect.top() + cornerLength);
    
    // Coin inférieur gauche
    painter.drawLine(rect.left(), rect.bottom() - cornerLength, rect.left(), rect.bottom());
    painter.drawLine(rect.left(), rect.bottom(), rect.left() + cornerLength, rect.bottom());
    
    // Coin inférieur droit
    painter.drawLine(rect.right() - cornerLength, rect.bottom(), rect.right(), rect.bottom());
    painter.drawLine(rect.right(), rect.bottom(), rect.right(), rect.bottom() - cornerLength);
    
    painter.restore();
}

void BarcodeImageWidget::drawScanLine(QPainter& painter)
{
    if (!m_isAnalyzing || m_imageDisplayRect.isEmpty()) return;
    
    painter.save();
    
    // Ligne de scan animée
    QLinearGradient gradient(0, 0, width(), 0);
    gradient.setColorAt(0.0, QColor(m_scanLineColor.red(), m_scanLineColor.green(), m_scanLineColor.blue(), 0));
    gradient.setColorAt(0.4, QColor(m_scanLineColor.red(), m_scanLineColor.green(), m_scanLineColor.blue(), 180));
    gradient.setColorAt(0.6, QColor(m_scanLineColor.red(), m_scanLineColor.green(), m_scanLineColor.blue(), 180));
    gradient.setColorAt(1.0, QColor(m_scanLineColor.red(), m_scanLineColor.green(), m_scanLineColor.blue(), 0));
    
    painter.fillRect(
        m_imageDisplayRect.left(),
        m_scanLinePosition - SCAN_LINE_HEIGHT/2,
        m_imageDisplayRect.width(),
        SCAN_LINE_HEIGHT,
        gradient
    );
    
    painter.restore();
}

void BarcodeImageWidget::drawDetectionInfo(QPainter& painter)
{
    if (!m_hasDetection || m_detectedCode.isEmpty()) return;
    
    painter.save();
    painter.setFont(m_infoFont);
    
    // Texte d'information
    QString infoText = m_detectionSuccess ? 
        QString("✅ Code EAN-13: %1").arg(m_detectedCode) :
        QString("🔍 Analyse: %1").arg(m_detectedCode);
    
    QFontMetrics fm(m_infoFont);
    QRect textRect = fm.boundingRect(infoText);
    
    // Position du texte (au-dessus du cadre)
    int textX = m_barcodeDisplayRect.center().x() - textRect.width() / 2;
    int textY = m_barcodeDisplayRect.top() - INFO_MARGIN;
    
    // S'assurer que le texte reste visible
    textX = qMax(INFO_MARGIN, qMin(textX, width() - textRect.width() - INFO_MARGIN));
    textY = qMax(textRect.height() + INFO_MARGIN, textY);
    
    // Fond du texte
    QRect bgRect(textX - 8, textY - textRect.height() - 4, textRect.width() + 16, textRect.height() + 8);
    
    QColor bgColor = m_detectionSuccess ? m_successColor : m_processingColor;
    bgColor.setAlpha(200);
    painter.fillRect(bgRect, bgColor);
    
    // Texte
    painter.setPen(Qt::white);
    painter.drawText(textX, textY, infoText);
    
    painter.restore();
}

void BarcodeImageWidget::drawAnalysisOverlay(QPainter& painter)
{
    if (!m_isAnalyzing) return;
    
    painter.save();
    
    // Overlay semi-transparent
    QColor overlayColor(0, 0, 0, 50);
    painter.fillRect(rect(), overlayColor);
    
    // Texte de progression
    if (m_analysisProgress > 0) {
        painter.setFont(m_infoFont);
        painter.setPen(Qt::white);
        
        QString progressText = QString("%1 (%2%)").arg(m_analysisStatus).arg(m_analysisProgress);
        QFontMetrics fm(m_infoFont);
        QRect textRect = fm.boundingRect(progressText);
        
        int textX = (width() - textRect.width()) / 2;
        int textY = height() - 30;
        
        // Fond du texte de progression
        QRect bgRect(textX - 10, textY - textRect.height() - 5, textRect.width() + 20, textRect.height() + 10);
        painter.fillRect(bgRect, QColor(0, 0, 0, 150));
        
        painter.drawText(textX, textY, progressText);
    }
    
    painter.restore();
}

void BarcodeImageWidget::calculateBarcodeDisplayRect()
{
    if (m_barcodeRect.isEmpty() || m_originalImage.isNull() || m_imageDisplayRect.isEmpty()) {
        m_barcodeDisplayRect = QRect();
        return;
    }
    
    // Facteurs d'échelle
    double scaleX = (double)m_imageDisplayRect.width() / m_originalImage.width();
    double scaleY = (double)m_imageDisplayRect.height() / m_originalImage.height();
    
    // Rectangle mis à l'échelle
    m_barcodeDisplayRect = QRect(
        m_imageDisplayRect.x() + (int)(m_barcodeRect.x() * scaleX),
        m_imageDisplayRect.y() + (int)(m_barcodeRect.y() * scaleY),
        (int)(m_barcodeRect.width() * scaleX),
        (int)(m_barcodeRect.height() * scaleY)
    );
    
    qDebug() << "📏 Rectangle calculé:" << m_barcodeDisplayRect << "depuis" << m_barcodeRect;
}

QRect BarcodeImageWidget::scaleBarcodeRect(const QRect& originalRect) const
{
    if (originalRect.isEmpty() || m_originalImage.isNull()) {
        return QRect();
    }
    
    double scaleX = (double)width() / m_originalImage.width();
    double scaleY = (double)height() / m_originalImage.height();
    double scale = qMin(scaleX, scaleY);
    
    return QRect(
        (int)(originalRect.x() * scale),
        (int)(originalRect.y() * scale),
        (int)(originalRect.width() * scale),
        (int)(originalRect.height() * scale)
    );
}

void BarcodeImageWidget::mousePressEvent(QMouseEvent *event)
{
    if (m_hasDetection && m_barcodeDisplayRect.contains(event->pos())) {
        emit barcodeClicked(m_barcodeRect);
    } else {
        emit imageClicked(event->pos());
    }
    
    QLabel::mousePressEvent(event);
}

void BarcodeImageWidget::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);
    
    if (!m_originalImage.isNull()) {
        // Recalculer le pixmap et les rectangles
        m_scaledPixmap = QPixmap::fromImage(m_originalImage).scaled(
            size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        calculateBarcodeDisplayRect();
        update();
    }
}

void BarcodeImageWidget::updateScanAnimation()
{
    update();
}

void BarcodeImageWidget::pulseHighlight()
{
    update();
}

// Getters pour les propriétés animées
int BarcodeImageWidget::scanLinePosition() const
{
    return m_scanLinePosition;
}

qreal BarcodeImageWidget::highlightOpacity() const
{
    return m_highlightOpacity;
}