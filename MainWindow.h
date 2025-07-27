#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QTabWidget>
#include <QLabel>
#include <QComboBox>
#include <QStackedWidget>
#include <QTextEdit>
#include <QTableWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QProgressBar>
#include <QTimer>
#include <QDebug>

// ZXing includes
// #include <ZXing/ReadBarcode.h>
// #include <ZXing/Result.h>
// #include <ZXing/BarcodeFormat.h>
// #include <ZXing/ImageView.h>
// #include <ZXing/ReaderOptions.h>

#include "BarcodeValidator.h"
#include "DatabaseManager.h"
#include "CameraWidget.h"
#include "ThemeManager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void addNewTab();
    void changeTheme();
    void importImage();
    void startScanner();
    void manualEntry();
    void showFAQ();
    void showHistory();
    void onBarcodeValidated(const QString& barcode, bool isValid);
    void redirectToGoogleLens(const QString& imagePath);
    void handleGoogleLensResult();

private:
    // UI Components
    QWidget* centralWidget;
    QVBoxLayout* mainLayout;
    QFrame* topFrame;
    QFrame* contentFrame;
    QHBoxLayout* topLayout;
    QHBoxLayout* contentLayout;

    // Top frame buttons
    QPushButton* addTabButton;
    QPushButton* themeButton;

    // Content frame buttons
    QPushButton* importButton;
    QPushButton* scannerButton;
    QPushButton* manualButton;
    QPushButton* faqButton;
    QPushButton* historyButton;

    // Stacked widget for different views
    QStackedWidget* stackedWidget;
    QWidget* mainView;
    QWidget* scannerView;
    QWidget* faqView;
    QWidget* historyView;

    // Components
    BarcodeValidator* barcodeValidator;
    DatabaseManager* databaseManager;
    CameraWidget* cameraWidget;
    ThemeManager* themeManager;
    QTableWidget* historyTable;
    QTextEdit* faqText;
    QProgressBar* progressBar;
    QLabel* statusLabel;

    // Tab management
    QTabWidget* tabWidget;
    static int tabCounter;

    // Image processing
    QImage currentImage;
    QLabel* imagePreview;
    QPushButton* googleLensResultButton;
    QPushButton* deleteImageButton;

    void setupUI();
    void setupConnections();
    void setupStackedWidget();
    void setupHistoryView();
    void setupFAQView();
    void setupScannerView();
    void applyCurrentTheme();
    void updateHistoryTable();
    QString getButtonStyle() const;
    QString getFrameStyle() const;
    QString calculerCleChiffrement(const QString& douzeChiffres);
    void setupImageProcessingView();
    QImage enhanceImageForBarcode(const QImage& originalImage);
    QString detectBarcodeFromImage(const QImage& image);
    QImage createBinaryImage(const QImage& grayImage);
};

#endif // MAINWINDOW_H