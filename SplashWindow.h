#pragma once
#ifndef SPLASHWINDOW_H
#define SPLASHWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QFont>

class MainWindow;

class SplashWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SplashWindow(QWidget* parent = nullptr);
    ~SplashWindow();

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void startAnimation();
    void onAnimationFinished();
    void showMainWindow();

private:
    QLabel* titleLabel;
    QPropertyAnimation* fadeInAnimation;
    QPropertyAnimation* scaleAnimation;
    QGraphicsOpacityEffect* opacityEffect;
    QTimer* delayTimer;
    MainWindow* mainWindow;

    void setupUI();
    void setupAnimations();
};

#endif // SPLASHWINDOW_H
