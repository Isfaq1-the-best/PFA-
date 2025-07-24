#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPixmap>
#include <QThread>
#include <QMutex>
#include <QDateTime>
#include <QFileInfo>
#include <QImageReader>
#include <QInputDialog>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

// OpenCV includes
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

// ZXing includes
#include <ZXing/ReadBarcode.h>
#include <ZXing/Result.h>
#include <ZXing/BarcodeFormat.h>
#include <ZXing/ImageView.h>
#include <ZXing/ReaderOptions.h>

class CameraWorker;

class CameraWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CameraWidget(QWidget* parent = nullptr);
    ~CameraWidget();

public slots:
    void startLocalCamera();
    void startIPCamera();
    void stopCamera();
    void captureImage();
    void connectToIPCamera(const QString& ipAddress);

signals:
    void barcodeDetected(const QString& barcode);
    void imageCapture(const QPixmap& image);

private slots:
    void updateIPCameraFrame();
    void onNetworkReplyFinished();
    void onFrameReady(const QPixmap& frame);
    void onCameraError(const QString& error);

private:
    // UI Components
    QVBoxLayout* mainLayout;
    QHBoxLayout* controlsLayout;
    QLabel* videoLabel;
    QPushButton* startLocalButton;
    QPushButton* startIPButton;
    QPushButton* stopButton;
    QPushButton* captureButton;
    QLineEdit* ipAddressEdit;
    QComboBox* connectionTypeCombo;
    QLabel* statusLabel;

    // OpenCV Camera components
    QThread* cameraThread;
    CameraWorker* cameraWorker;

    // IP Camera components
    QNetworkAccessManager* networkManager;
    QTimer* frameTimer;
    QString currentIPAddress;
    bool isIPCameraActive;
    bool isLocalCameraActive;
    bool scanned;

    void setupUI();
    void initializeDatabase();
    void processFrameForBarcodeZXing(const QPixmap& frame);
    QString calculerCleChiffrement(const QString& douzeChiffres);
    void saveBarcodeToDatabase(const QString& barcode, const QString& method);
    void showConnectionDialog();
    QPixmap matToQPixmap(const cv::Mat& mat);
};

// Worker class for OpenCV camera in separate thread
class CameraWorker : public QObject
{
    Q_OBJECT

public:
    explicit CameraWorker(QObject* parent = nullptr);
    ~CameraWorker();

public slots:
    void startCapture(int cameraIndex = 0);
    void stopCapture();

signals:
    void frameReady(const QPixmap& frame);
    void error(const QString& errorMessage);

private slots:
    void captureFrame();

private:
    cv::VideoCapture* capture;
    QTimer* captureTimer;
    QMutex captureMutex;
    bool isCapturing;

    QPixmap matToQPixmap(const cv::Mat& mat);
};

#endif // CAMERAWIDGET_H