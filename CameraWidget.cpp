#include "CameraWidget.h"
#include "BarcodeValidator.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QNetworkRequest>
#include <QUrl>
#include <QPixmap>
#include <QDebug>
#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

// ZXing includes
#include <ZXing/ReadBarcode.h>
#include <ZXing/Result.h>
#include <ZXing/BarcodeFormat.h>
#include <ZXing/ImageView.h>
#include <ZXing/ReaderOptions.h>

CameraWidget::CameraWidget(QWidget* parent)
    : QWidget(parent), cameraThread(nullptr), cameraWorker(nullptr),
    networkManager(nullptr), frameTimer(nullptr),
    isIPCameraActive(false), isLocalCameraActive(false), scanned(false)
{
    setupUI();
    networkManager = new QNetworkAccessManager(this);
    frameTimer = new QTimer(this);
    frameTimer->setInterval(100); // 10 FPS

    connect(frameTimer, &QTimer::timeout, this, &CameraWidget::updateIPCameraFrame);

    // Initialiser la base de données
    initializeDatabase();
}

CameraWidget::~CameraWidget()
{
    stopCamera();
}

void CameraWidget::setupUI()
{
    mainLayout = new QVBoxLayout(this);

    // Zone d'affichage vidéo
    videoLabel = new QLabel();
    videoLabel->setMinimumSize(640, 480);
    videoLabel->setMaximumSize(800, 600);
    videoLabel->setStyleSheet(
        "QLabel {"
        "border: 2px solid #2E86AB;"
        "border-radius: 8px;"
        "background-color: #f0f0f0;"
        "padding: 5px;"
        "}"
    );
    videoLabel->setAlignment(Qt::AlignCenter);
    videoLabel->setScaledContents(false);
    videoLabel->setText("Aucune caméra active\n\nL'utilisateur peut :\n- Importer des images via le bouton 'Importer'\n- Scanner avec la caméra locale ou IP\n- Saisir manuellement un code-barre");

    // Contrôles
    controlsLayout = new QHBoxLayout();

    startLocalButton = new QPushButton("Caméra Locale");
    startIPButton = new QPushButton("Caméra IP");
    stopButton = new QPushButton("Arrêter");
    captureButton = new QPushButton("Capturer");

    startLocalButton->setFixedSize(120, 35);
    startIPButton->setFixedSize(120, 35);
    stopButton->setFixedSize(80, 35);
    captureButton->setFixedSize(80, 35);

    // Configuration IP
    QHBoxLayout* ipLayout = new QHBoxLayout();
    ipAddressEdit = new QLineEdit();
    ipAddressEdit->setPlaceholderText("192.168.1.100:8080");
    ipAddressEdit->setFixedWidth(150);

    connectionTypeCombo = new QComboBox();
    connectionTypeCombo->addItem("WiFi");
    connectionTypeCombo->addItem("Hotspot");
    connectionTypeCombo->setFixedWidth(100);

    ipLayout->addWidget(new QLabel("Type:"));
    ipLayout->addWidget(connectionTypeCombo);
    ipLayout->addWidget(new QLabel("IP:"));
    ipLayout->addWidget(ipAddressEdit);

    controlsLayout->addWidget(startLocalButton);
    controlsLayout->addWidget(startIPButton);
    controlsLayout->addWidget(stopButton);
    controlsLayout->addWidget(captureButton);
    controlsLayout->addStretch();
    controlsLayout->addLayout(ipLayout);

    // Label de statut
    statusLabel = new QLabel("Prêt - En attente d'images de l'utilisateur");
    statusLabel->setStyleSheet("color: #2E86AB; font-weight: bold;");

    // Assemblage
    mainLayout->addWidget(videoLabel, 1);
    mainLayout->addLayout(controlsLayout);
    mainLayout->addWidget(statusLabel);

    // Connexions
    connect(startLocalButton, &QPushButton::clicked, this, &CameraWidget::startLocalCamera);
    connect(startIPButton, &QPushButton::clicked, this, &CameraWidget::startIPCamera);
    connect(stopButton, &QPushButton::clicked, this, &CameraWidget::stopCamera);
    connect(captureButton, &QPushButton::clicked, this, &CameraWidget::captureImage);

    // État initial
    stopButton->setEnabled(false);
    captureButton->setEnabled(false);
}

void CameraWidget::initializeDatabase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "camera_connection");
    db.setDatabaseName("codes_barcodes_camera.db");

    if (!db.open()) {
        qDebug() << "Erreur de connexion à la base de données caméra:" << db.lastError().text();
        return;
    }

    QSqlQuery query(db);
    if (!query.exec("CREATE TABLE IF NOT EXISTS barcodes_camera (id INTEGER PRIMARY KEY AUTOINCREMENT, code TEXT, date DATETIME DEFAULT CURRENT_TIMESTAMP, method TEXT)")) {
        qDebug() << "Erreur création table:" << query.lastError().text();
    }
}

void CameraWidget::startLocalCamera()
{
    stopCamera();
    scanned = false;

    // Créer le thread et le worker pour OpenCV
    cameraThread = new QThread(this);
    cameraWorker = new CameraWorker();
    cameraWorker->moveToThread(cameraThread);

    // Connexions
    connect(cameraThread, &QThread::started, [this]() {
        cameraWorker->startCapture(0); // Caméra par défaut
        });
    connect(cameraWorker, &CameraWorker::frameReady, this, &CameraWidget::onFrameReady);
    connect(cameraWorker, &CameraWorker::error, this, &CameraWidget::onCameraError);
    connect(cameraThread, &QThread::finished, cameraWorker, &CameraWorker::deleteLater);

    cameraThread->start();

    isLocalCameraActive = true;
    statusLabel->setText("Caméra locale active - Détection automatique de codes-barres EAN-13");
    startLocalButton->setEnabled(false);
    startIPButton->setEnabled(false);
    stopButton->setEnabled(true);
    captureButton->setEnabled(true);
}

void CameraWidget::startIPCamera()
{
    stopCamera();
    scanned = false;
    showConnectionDialog();
}

void CameraWidget::stopCamera()
{
    scanned = false;

    // Arrêter la caméra locale OpenCV
    if (cameraWorker) {
        cameraWorker->stopCapture();
    }

    if (cameraThread && cameraThread->isRunning()) {
        cameraThread->quit();
        cameraThread->wait(3000);
        if (cameraThread->isRunning()) {
            cameraThread->terminate();
            cameraThread->wait(1000);
        }
    }

    cameraThread = nullptr;
    cameraWorker = nullptr;
    isLocalCameraActive = false;

    // Arrêter la caméra IP
    if (frameTimer && frameTimer->isActive()) {
        frameTimer->stop();
    }
    isIPCameraActive = false;

    // Réinitialiser l'interface
    videoLabel->setText("Aucune caméra active\n\nL'utilisateur peut :\n- Importer des images via le bouton 'Importer'\n- Scanner avec la caméra locale ou IP\n- Saisir manuellement un code-barre");
    statusLabel->setText("Arrêté - En attente d'images de l'utilisateur");

    startLocalButton->setEnabled(true);
    startIPButton->setEnabled(true);
    stopButton->setEnabled(false);
    captureButton->setEnabled(false);
}

void CameraWidget::captureImage()
{
    if (isLocalCameraActive || isIPCameraActive) {
        // Capturer l'image actuelle
        QPixmap currentFrame = videoLabel->pixmap(Qt::ReturnByValue);
        if (!currentFrame.isNull()) {
            emit imageCapture(currentFrame);
            processFrameForBarcodeZXing(currentFrame);
            statusLabel->setText("Image capturée et analysée avec ZXing");
        }
    }
}

void CameraWidget::connectToIPCamera(const QString& ipAddress)
{
    currentIPAddress = ipAddress;

    // Vérifier le format de l'adresse IP
    if (!ipAddress.contains(":")) {
        QMessageBox::warning(this, "Erreur",
            "Format d'adresse incorrect.\n"
            "Utilisez le format: 192.168.1.100:8080");
        return;
    }

    // Construire l'URL complète pour IP Webcam
    QString fullUrl = QString("http://%1").arg(ipAddress);
    if (!fullUrl.endsWith("/shot.jpg", Qt::CaseInsensitive)) {
        if (!fullUrl.endsWith("/"))
            fullUrl += "/";
        fullUrl += "shot.jpg";
    }
    currentIPAddress = fullUrl;

    statusLabel->setText("Test de connexion à la caméra IP...");

    // Tester d'abord avec une requête simple
    
    QNetworkRequest request(QUrl(fullUrl));

    QNetworkReply* reply = networkManager->get(QNetworkRequest(QUrl(fullUrl)));
    reply->setParent(this);

    // Timeout après 10 secondes
    QTimer::singleShot(10000, reply, &QNetworkReply::abort);

    connect(reply, &QNetworkReply::finished, [this, reply, ipAddress]() {
        if (reply->error() == QNetworkReply::NoError) {
            // La connexion fonctionne, démarrer le flux
            isIPCameraActive = true;
            frameTimer->start();
            statusLabel->setText(QString("Caméra IP connectée (%1) - Détection automatique EAN-13").arg(ipAddress));

            startLocalButton->setEnabled(false);
            startIPButton->setEnabled(false);
            stopButton->setEnabled(true);
            captureButton->setEnabled(true);

            QMessageBox::information(this, "Succès",
                "Connexion établie avec la caméra IP.\n"
                "Détection automatique de codes-barres EAN-13 activée.");
        }
        else {
            statusLabel->setText("Échec de connexion IP");
            QString errorMsg = QString("Impossible de se connecter à %1\n\n"
                "Vérifiez que :\n"
                "• L'application IP Webcam est active sur votre smartphone\n"
                "• L'adresse IP est correcte (format: 192.168.1.100:8080)\n"
                "• Votre PC et smartphone sont sur le même réseau\n"
                "• Le pare-feu ne bloque pas la connexion\n\n"
                "Erreur: %2").arg(ipAddress, reply->errorString());
            QMessageBox::warning(this, "Erreur de connexion", errorMsg);
        }
        reply->deleteLater();
        });
}

void CameraWidget::updateIPCameraFrame()
{
    if (!isIPCameraActive || scanned) {
        if (scanned) {
            frameTimer->stop();
        }
        return;
    }

    QNetworkRequest request(QUrl(currentIPAddress));
    QUrl url(currentIPAddress);
    url.setQuery(QString("ts=%1").arg(QDateTime::currentMSecsSinceEpoch()));
    QNetworkRequest request(QUrl(currentIPAddress));

    QNetworkReply* reply = networkManager->get(QNetworkRequest(QUrl(currentIPAddress)));
    reply->setParent(this);

    // Timeout pour éviter les blocages
    QTimer::singleShot(5000, reply, &QNetworkReply::abort);

    connect(reply, &QNetworkReply::finished, this, &CameraWidget::onNetworkReplyFinished);
}

void CameraWidget::onNetworkReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply || scanned) {
        if (reply) reply->deleteLater();
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray imageData = reply->readAll();

        if (!imageData.isEmpty()) {
            QImage image;
            if (image.loadFromData(imageData)) {
                // Redimensionner l'image pour s'adapter au label
                QSize labelSize = videoLabel->size();
                if (labelSize.width() < 320) labelSize.setWidth(320);
                if (labelSize.height() < 240) labelSize.setHeight(240);

                QPixmap scaledPixmap = QPixmap::fromImage(image).scaled(labelSize,
                    Qt::KeepAspectRatio,
                    Qt::SmoothTransformation);
                videoLabel->setPixmap(scaledPixmap);
                videoLabel->setScaledContents(false);
                videoLabel->setAlignment(Qt::AlignCenter);

                // Analyser l'image pour détecter des codes-barres avec ZXing
                processFrameForBarcodeZXing(scaledPixmap);

                // Mettre à jour le statut
                static int frameCount = 0;
                frameCount++;
                if (frameCount % 10 == 0) { // Toutes les 10 frames (~1 seconde)
                    statusLabel->setText(QString("Flux IP actif - Frame %1 - Détection EAN-13 active").arg(frameCount));
                }
            }
            else {
                statusLabel->setText("Erreur: Format d'image invalide");
            }
        }
        else {
            statusLabel->setText("Erreur: Données d'image vides");
        }
    }
    else if (reply->error() != QNetworkReply::OperationCanceledError) {
        statusLabel->setText(QString("Erreur réseau: %1").arg(reply->errorString()));

        // Si trop d'erreurs consécutives, arrêter le flux
        static int errorCount = 0;
        errorCount++;
        if (errorCount > 5) {
            stopCamera();
            QMessageBox::warning(this, "Connexion perdue",
                "Connexion avec la caméra IP perdue.\n"
                "Vérifiez votre réseau et reconnectez-vous.");
            errorCount = 0;
        }
    }
    else {
        // Reset du compteur d'erreurs en cas de succès
        static int errorCount = 0;
        errorCount = 0;
    }

    reply->deleteLater();
}

void CameraWidget::processFrameForBarcodeZXing(const QPixmap& frame)
{
    if (scanned || frame.isNull()) {
        return;
    }

    // Convertir QPixmap en QImage
    QImage image = frame.toImage();
    if (image.isNull()) {
        return;
    }

    // Convertir au format RGB pour ZXing
    QImage rgbImage = image.convertToFormat(QImage::Format_RGB888);

    // Créer l'ImageView pour ZXing
    ZXing::ImageView zxingImage(
        rgbImage.constBits(),
        rgbImage.width(),
        rgbImage.height(),
        ZXing::ImageFormat::RGB
    );

    // Configuration ZXing pour EAN-13
    ZXing::ReaderOptions options;
    options.setTryHarder(true);
    options.setFormats({ ZXing::BarcodeFormat::EAN13 });

    // Lecture du code-barre
    ZXing::Result result = ZXing::ReadBarcode(zxingImage, options);

    if (result.isValid()) {
        QString codeBarres = QString::fromStdString(result.text());
        qDebug() << "Code-barres EAN-13 détecté :" << codeBarres;

        // Marquer comme scanné pour arrêter la détection
        scanned = true;

        // Arrêter les timers
        if (frameTimer) frameTimer->stop();

        // Calculer et vérifier la clé de contrôle
        QString cleCalculee = calculerCleChiffrement(codeBarres.left(12));
        int cleBarcode = codeBarres.right(1).toInt();

        bool isValid = (cleCalculee.toInt() == cleBarcode);

        if (isValid) {
            statusLabel->setText(QString("✅ Code-barre EAN-13 valide détecté: %1").arg(codeBarres));
            QMessageBox::information(this, "Code-barre détecté",
                QString("✅ Code-barre EAN-13 valide !\n\n"
                    "Code: %1\n"
                    "Clé de contrôle: Correcte (%2)")
                .arg(codeBarres, cleCalculee));
        }
        else {
            statusLabel->setText(QString("❌ Code-barre EAN-13 invalide détecté: %1").arg(codeBarres));
            QMessageBox::warning(this, "Code-barre détecté",
                QString("❌ Code-barre EAN-13 invalide !\n\n"
                    "Code: %1\n"
                    "Clé de contrôle: Incorrecte (attendu: %2, trouvé: %3)")
                .arg(codeBarres, cleCalculee, QString::number(cleBarcode)));
        }

        // Sauvegarder dans la base de données
        saveBarcodeToDatabase(codeBarres, isValid ? "Scanner-Valide" : "Scanner-Invalide");

        // Émettre le signal
        emit barcodeDetected(codeBarres);

        // Arrêter la caméra après détection
        QTimer::singleShot(3000, this, &CameraWidget::stopCamera);
    }
}

QString CameraWidget::calculerCleChiffrement(const QString& douzeChiffres)
{
    int somme_impair = 0;
    int somme_pair = 0;

    for (int i = 0; i < douzeChiffres.length(); ++i) {
        int digit = douzeChiffres[i].digitValue();
        if ((i % 2) == 0)  // positions impaires (indices pairs)
            somme_impair += digit;
        else
            somme_pair += digit;
    }

    int total = somme_impair + 3 * somme_pair;
    int cle = (10 - (total % 10)) % 10;
    return QString::number(cle);
}

void CameraWidget::saveBarcodeToDatabase(const QString& barcode, const QString& method)
{
    QSqlDatabase db = QSqlDatabase::database("camera_connection");
    if (!db.isOpen()) {
        qDebug() << "Base de données non ouverte";
        return;
    }

    QSqlQuery insertQuery(db);
    insertQuery.prepare("INSERT INTO barcodes_camera (code, method) VALUES (:code, :method)");
    insertQuery.bindValue(":code", barcode);
    insertQuery.bindValue(":method", method);

    if (!insertQuery.exec()) {
        qDebug() << "Erreur lors de l'insertion dans la base de données:" << insertQuery.lastError().text();
    }
    else {
        qDebug() << "Code-barre sauvegardé:" << barcode << "Méthode:" << method;
    }
}

void CameraWidget::onFrameReady(const QPixmap& frame)
{
    if (!frame.isNull() && !scanned) {
        // Redimensionner pour s'adapter au label avec taille minimum
        QSize labelSize = videoLabel->size();
        if (labelSize.width() < 320) labelSize.setWidth(320);
        if (labelSize.height() < 240) labelSize.setHeight(240);

        QPixmap scaledFrame = frame.scaled(labelSize,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation);

        // Qt 6 : Utiliser setPixmap correctement
        videoLabel->setPixmap(scaledFrame);
        videoLabel->setScaledContents(true);
        videoLabel->setAlignment(Qt::AlignCenter);

        // Analyser pour détecter des codes-barres avec ZXing
        processFrameForBarcodeZXing(scaledFrame);

        // Mettre à jour le statut
        static int localFrameCount = 0;
        localFrameCount++;
        if (localFrameCount % 30 == 0) { // Toutes les 30 frames (~1 seconde à 30fps)
            statusLabel->setText(QString("Caméra locale active - Frame %1 - Détection EAN-13 active").arg(localFrameCount));
        }
    }
}

void CameraWidget::onCameraError(const QString& error)
{
    statusLabel->setText("Erreur caméra: " + error);
    QMessageBox::warning(this, "Erreur Caméra", error);
    stopCamera();
}

void CameraWidget::showConnectionDialog()
{
    bool ok;
    QString ip = QInputDialog::getText(this,
        "Connexion Caméra IP",
        "Entrez l'adresse IP de votre smartphone\n"
        "(Format: 192.168.1.100:8080):\n\n"
        "Instructions:\n"
        "1. Installez 'IP Webcam' sur Android\n"
        "2. Lancez l'app et notez l'IP affichée\n"
        "3. Choisissez WiFi ou Hotspot selon votre réseau\n"
        "4. La détection EAN-13 sera automatique",
        QLineEdit::Normal,
        ipAddressEdit->text(),
        &ok);

    if (ok && !ip.isEmpty()) {
        ipAddressEdit->setText(ip);
        connectToIPCamera(ip);
    }
}

QPixmap CameraWidget::matToQPixmap(const cv::Mat& mat)
{
    if (mat.empty()) {
        return QPixmap();
    }

    cv::Mat rgbMat;
    if (mat.channels() == 3) {
        cv::cvtColor(mat, rgbMat, cv::COLOR_BGR2RGB);
    }
    else if (mat.channels() == 4) {
        cv::cvtColor(mat, rgbMat, cv::COLOR_BGRA2RGB);
    }
    else {
        cv::cvtColor(mat, rgbMat, cv::COLOR_GRAY2RGB);
    }

    QImage qimg(rgbMat.data, rgbMat.cols, rgbMat.rows, rgbMat.step, QImage::Format_RGB888);
    return QPixmap::fromImage(qimg);
}

// Implémentation de CameraWorker
CameraWorker::CameraWorker(QObject* parent)
    : QObject(parent), capture(nullptr), captureTimer(nullptr), isCapturing(false)
{
    captureTimer = new QTimer(this);
    connect(captureTimer, &QTimer::timeout, this, &CameraWorker::captureFrame);
}

CameraWorker::~CameraWorker()
{
    stopCapture();
}

void CameraWorker::startCapture(int cameraIndex)
{
    QMutexLocker locker(&captureMutex);

    if (isCapturing) {
        return;
    }

    capture = new cv::VideoCapture(cameraIndex);

    if (!capture->isOpened()) {
        emit error("Impossible d'ouvrir la caméra. Vérifiez qu'aucune autre application ne l'utilise.");
        delete capture;
        capture = nullptr;
        return;
    }

    // Configuration de la caméra
    capture->set(cv::CAP_PROP_FRAME_WIDTH, 640);
    capture->set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    capture->set(cv::CAP_PROP_FPS, 30);

    isCapturing = true;
    captureTimer->start(33); // ~30 FPS
}

void CameraWorker::stopCapture()
{
    QMutexLocker locker(&captureMutex);

    isCapturing = false;

    if (captureTimer) {
        captureTimer->stop();
    }

    if (capture) {
        capture->release();
        delete capture;
        capture = nullptr;
    }
}

void CameraWorker::captureFrame()
{
    QMutexLocker locker(&captureMutex);

    if (!isCapturing || !capture) {
        return;
    }

    cv::Mat frame;
    if (capture->read(frame) && !frame.empty()) {
        QPixmap pixmap = matToQPixmap(frame);
        if (!pixmap.isNull()) {
            emit frameReady(pixmap);
        }
    }
}

QPixmap CameraWorker::matToQPixmap(const cv::Mat& mat)
{
    if (mat.empty()) {
        return QPixmap();
    }

    cv::Mat rgbMat;
    if (mat.channels() == 3) {
        cv::cvtColor(mat, rgbMat, cv::COLOR_BGR2RGB);
    }
    else if (mat.channels() == 4) {
        cv::cvtColor(mat, rgbMat, cv::COLOR_BGRA2RGB);
    }
    else {
        cv::cvtColor(mat, rgbMat, cv::COLOR_GRAY2RGB);
    }

    QImage qimg(rgbMat.data, rgbMat.cols, rgbMat.rows, rgbMat.step, QImage::Format_RGB888);
    return QPixmap::fromImage(qimg);
}