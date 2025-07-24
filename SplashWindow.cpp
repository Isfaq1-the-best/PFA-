#include "SplashWindow.h"
#include "MainWindow.h"
#include <QApplication>
#include <QScreen>

SplashWindow::SplashWindow(QWidget* parent)
    : QWidget(parent), mainWindow(nullptr)
{
    setupUI();
    setupAnimations();

    // Démarrer l'animation après un court délai
    QTimer::singleShot(500, this, &SplashWindow::startAnimation);
}

SplashWindow::~SplashWindow()
{
    if (mainWindow) {
        delete mainWindow;
    }
}

void SplashWindow::setupUI()
{
    setFixedSize(600, 400);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // Centrer la fenêtre
    QScreen* screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);

    // Configuration du layout
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);

    // Label du titre
    titleLabel = new QLabel("EPO PROJECT", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "QLabel {"
        "color: #2E86AB;"
        "font-size: 48px;"
        "font-weight: bold;"
        "font-family: 'Arial', sans-serif;"
        "}"
    );

    layout->addWidget(titleLabel);

    // Effet d'opacité
    opacityEffect = new QGraphicsOpacityEffect(this);
    opacityEffect->setOpacity(0.0);
    titleLabel->setGraphicsEffect(opacityEffect);
}

void SplashWindow::setupAnimations()
{
    // Animation de fondu
    fadeInAnimation = new QPropertyAnimation(opacityEffect, "opacity", this);
    fadeInAnimation->setDuration(2000);
    fadeInAnimation->setStartValue(0.0);
    fadeInAnimation->setEndValue(1.0);
    fadeInAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    // Timer pour la durée d'affichage
    delayTimer = new QTimer(this);
    delayTimer->setSingleShot(true);
    delayTimer->setInterval(3000);

    connect(fadeInAnimation, &QPropertyAnimation::finished,
        this, &SplashWindow::onAnimationFinished);
    connect(delayTimer, &QTimer::timeout,
        this, &SplashWindow::showMainWindow);
}

void SplashWindow::startAnimation()
{
    fadeInAnimation->start();
}

void SplashWindow::onAnimationFinished()
{
    delayTimer->start();
}

void SplashWindow::showMainWindow()
{
    // Créer et afficher la fenêtre principale
    mainWindow = new MainWindow();
    mainWindow->show();

    // Fermer l'écran de démarrage
    close();
}

void SplashWindow::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

        QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Fond avec gradient
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0, QColor(46, 134, 171, 200));
    gradient.setColorAt(1, QColor(112, 193, 207, 200));

    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), 15, 15);
}