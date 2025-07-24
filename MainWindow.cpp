#include "MainWindow.h"
#include <QApplication>
#include <QHeaderView>
#include <QSplitter>
#include <QStatusBar>
#include <cmath>

#ifdef QT_ONLY_IMAGE_PROCESSING
// Utilisation de Qt uniquement pour le traitement d'images
#include <QImageReader>
#include <QBuffer>
#include <QTimer>
#define OPENCV_AVAILABLE false
#else
// OpenCV disponible
#define OPENCV_AVAILABLE true
#endif

int MainWindow::tabCounter = 1;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    // Initialisation des composants
    barcodeValidator = new BarcodeValidator(this);
    databaseManager = new DatabaseManager(this);
    themeManager = new ThemeManager(this);

    setupUI();
    setupConnections();
    applyCurrentTheme();

    // Initialiser la base de données
    databaseManager->initializeDatabase();

    setWindowTitle("EPO Barcode Validator");
    setMinimumSize(1000, 700);
    resize(1200, 800);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Frame supérieure
    topFrame = new QFrame();
    topFrame->setFixedHeight(60);
    topFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    topLayout = new QHBoxLayout(topFrame);
    topLayout->setContentsMargins(15, 10, 15, 10);

    // Boutons de la frame supérieure
    addTabButton = new QPushButton("Nouvel Onglet");
    addTabButton->setIcon(QIcon(":/icons/add_tab.png"));
    addTabButton->setFixedSize(150, 40);

    themeButton = new QPushButton("Thème");
    themeButton->setIcon(QIcon(":/icons/theme.png"));
    themeButton->setFixedSize(100, 40);

    topLayout->addWidget(addTabButton);
    topLayout->addWidget(themeButton);
    topLayout->addStretch();

    // Frame de contenu
    contentFrame = new QFrame();
    contentFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    contentLayout = new QHBoxLayout(contentFrame);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    contentLayout->setSpacing(15);

    // Boutons de la frame de contenu
    importButton = new QPushButton("Importer");
    importButton->setIcon(QIcon(":/icons/import.png"));
    importButton->setFixedSize(120, 80);

    scannerButton = new QPushButton("Scanner");
    scannerButton->setIcon(QIcon(":/icons/scanner.png"));
    scannerButton->setFixedSize(120, 80);

    manualButton = new QPushButton("Saisie\nManuelle");
    manualButton->setIcon(QIcon(":/icons/manual.png"));
    manualButton->setFixedSize(120, 80);

    faqButton = new QPushButton("FAQ");
    faqButton->setIcon(QIcon(":/icons/faq.png"));
    faqButton->setFixedSize(120, 80);

    historyButton = new QPushButton("Historique");
    historyButton->setIcon(QIcon(":/icons/history.png"));
    historyButton->setFixedSize(120, 80);

    // Layout des boutons
    contentLayout->addWidget(importButton);
    contentLayout->addWidget(scannerButton);
    contentLayout->addWidget(manualButton);
    contentLayout->addWidget(faqButton);
    contentLayout->addStretch();
    contentLayout->addWidget(historyButton);

    // Widget empilé pour les différentes vues
    setupStackedWidget();

    // Barre de statut
    statusLabel = new QLabel("Prêt");
    progressBar = new QProgressBar();
    progressBar->setVisible(false);

    statusBar()->addWidget(statusLabel);
    statusBar()->addPermanentWidget(progressBar);

    // Ajout à la layout principale
    mainLayout->addWidget(topFrame);
    mainLayout->addWidget(contentFrame);
    mainLayout->addWidget(stackedWidget, 1);
}

void MainWindow::setupStackedWidget()
{
    stackedWidget = new QStackedWidget();

    // Vue principale
    mainView = new QWidget();
    QVBoxLayout* mainViewLayout = new QVBoxLayout(mainView);
    QLabel* welcomeLabel = new QLabel("Bienvenue dans EPO Barcode Validator");
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2E86AB;");
    mainViewLayout->addWidget(welcomeLabel);

    // Zone de traitement d'images
    setupImageProcessingView();
    mainViewLayout->addWidget(imagePreview);
    mainViewLayout->addWidget(analysisStatusLabel);
    mainViewLayout->addWidget(imageProgressBar);

    QHBoxLayout* imageButtonsLayout = new QHBoxLayout();
    imageButtonsLayout->addStretch();
    imageButtonsLayout->addWidget(analyzeImageButton);
    imageButtonsLayout->addWidget(deleteImageButton);
    imageButtonsLayout->addStretch();
    mainViewLayout->addLayout(imageButtonsLayout);

    mainViewLayout->addStretch();

    setupScannerView();
    setupFAQView();
    setupHistoryView();

    stackedWidget->addWidget(mainView);
    stackedWidget->addWidget(scannerView);
    stackedWidget->addWidget(faqView);
    stackedWidget->addWidget(historyView);
}

void MainWindow::setupScannerView()
{
    scannerView = new QWidget();
    QVBoxLayout* scannerLayout = new QVBoxLayout(scannerView);

    QLabel* scannerTitle = new QLabel("Scanner de Codes-Barres");
    scannerTitle->setStyleSheet("font-size: 18px; font-weight: bold;");
    scannerTitle->setAlignment(Qt::AlignCenter);

    cameraWidget = new CameraWidget();

    QHBoxLayout* connectionLayout = new QHBoxLayout();
    QPushButton* wifiButton = new QPushButton("WiFi");
    QPushButton* hotspotButton = new QPushButton("Hotspot");
    QPushButton* backButton = new QPushButton("Retour");

    connectionLayout->addWidget(wifiButton);
    connectionLayout->addWidget(hotspotButton);
    connectionLayout->addStretch();
    connectionLayout->addWidget(backButton);

    scannerLayout->addWidget(scannerTitle);
    scannerLayout->addWidget(cameraWidget, 1);
    scannerLayout->addLayout(connectionLayout);

    connect(backButton, &QPushButton::clicked, [this]() {
        stackedWidget->setCurrentWidget(mainView);
        });
}

void MainWindow::setupFAQView()
{
    faqView = new QWidget();
    QVBoxLayout* faqLayout = new QVBoxLayout(faqView);

    QLabel* faqTitle = new QLabel("Foire Aux Questions");
    faqTitle->setStyleSheet("font-size: 18px; font-weight: bold;");
    faqTitle->setAlignment(Qt::AlignCenter);

    faqText = new QTextEdit();
    faqText->setReadOnly(true);
    faqText->setHtml(
        "<h3>Comment utiliser l'application ?</h3>"
        "<p><b>Images fournies par l'utilisateur :</b> L'application traite les images et codes-barres que vous importez ou scannez.</p>"
        "<p><b>Importer :</b> Permet d'importer des images (JPEG, JPG, PNG) contenant des codes-barres.</p>"
        "<p><b>Scanner :</b> Utilise la caméra ou une connexion IP pour scanner en temps réel.</p>"
        "<p><b>Saisie Manuelle :</b> Permet de saisir directement un code-barre.</p>"
        "<p><b>Historique :</b> Affiche tous les codes-barres validés et enregistrés.</p>"
        "<br>"
        "<h3>Sources des données</h3>"
        "<p>L'application ne génère pas de codes-barres. Elle traite uniquement :</p>"
        "<ul>"
        "<li>Les images que vous importez depuis votre ordinateur</li>"
        "<li>Les codes-barres que vous scannez avec la caméra</li>"
        "<li>Les codes-barres que vous saisissez manuellement</li>"
        "</ul>"
        "<br>"
        "<h3>Validation des codes-barres</h3>"
        "<p>L'application vérifie automatiquement la clé de chiffrement du code-barre en utilisant "
        "les 12 premiers chiffres pour calculer la clé de contrôle et la compare avec le 13ème chiffre.</p>"
        "<br>"
        "<h3>Configuration caméra IP</h3>"
        "<p>Pour utiliser votre smartphone comme caméra :</p>"
        "<ol>"
        "<li>Installez 'IP Webcam' sur Android</li>"
        "<li>Lancez l'application et notez l'adresse IP</li>"
        "<li>Choisissez WiFi (même réseau) ou Hotspot</li>"
        "<li>Saisissez l'IP dans l'application</li>"
        "</ol>"
        "<br>"
        "<h3>Base de données</h3>"
        "<p>Tous les codes-barres validés sont automatiquement enregistrés dans la base de données locale.</p>"
    );

    QPushButton* backButton = new QPushButton("Retour");
    connect(backButton, &QPushButton::clicked, [this]() {
        stackedWidget->setCurrentWidget(mainView);
        });

    faqLayout->addWidget(faqTitle);
    faqLayout->addWidget(faqText, 1);
    faqLayout->addWidget(backButton);
}

void MainWindow::setupHistoryView()
{
    historyView = new QWidget();
    QVBoxLayout* historyLayout = new QVBoxLayout(historyView);

    QLabel* historyTitle = new QLabel("Historique des Codes-Barres");
    historyTitle->setStyleSheet("font-size: 18px; font-weight: bold;");
    historyTitle->setAlignment(Qt::AlignCenter);

    historyTable = new QTableWidget();
    historyTable->setColumnCount(4);
    QStringList headers = { "Code-Barre", "Date/Heure", "Méthode", "Statut" };
    historyTable->setHorizontalHeaderLabels(headers);
    historyTable->horizontalHeader()->setStretchLastSection(true);
    historyTable->setAlternatingRowColors(true);
    historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    QHBoxLayout* historyButtonsLayout = new QHBoxLayout();
    QPushButton* refreshButton = new QPushButton("Actualiser");
    QPushButton* exportButton = new QPushButton("Exporter");
    QPushButton* backButton = new QPushButton("Retour");

    historyButtonsLayout->addWidget(refreshButton);
    historyButtonsLayout->addWidget(exportButton);
    historyButtonsLayout->addStretch();
    historyButtonsLayout->addWidget(backButton);

    historyLayout->addWidget(historyTitle);
    historyLayout->addWidget(historyTable, 1);
    historyLayout->addLayout(historyButtonsLayout);

    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::updateHistoryTable);
    connect(backButton, &QPushButton::clicked, [this]() {
        stackedWidget->setCurrentWidget(mainView);
        });
}

void MainWindow::setupConnections()
{
    connect(addTabButton, &QPushButton::clicked, this, &MainWindow::addNewTab);
    connect(themeButton, &QPushButton::clicked, this, &MainWindow::changeTheme);
    connect(importButton, &QPushButton::clicked, this, &MainWindow::importImage);
    connect(scannerButton, &QPushButton::clicked, this, &MainWindow::startScanner);
    connect(manualButton, &QPushButton::clicked, this, &MainWindow::manualEntry);
    connect(faqButton, &QPushButton::clicked, this, &MainWindow::showFAQ);
    connect(historyButton, &QPushButton::clicked, this, &MainWindow::showHistory);

    connect(barcodeValidator, &BarcodeValidator::barcodeValidated,
        this, &MainWindow::onBarcodeValidated);

    // Connexions pour le traitement d'images amélioré
    connect(analyzeImageButton, &QPushButton::clicked, this, &MainWindow::processImageAutomatically);

    connect(deleteImageButton, &QPushButton::clicked, this, [this]() {
        currentImage = QImage();
        imagePreview->clear();
        imagePreview->setText("Aucune image chargée\n\nUtilisez le bouton 'Importer' pour charger une image");
        imagePreview->setVisible(false);
        analyzeImageButton->setVisible(false);
        deleteImageButton->setVisible(false);
        analysisStatusLabel->setVisible(false);
        imageProgressBar->setVisible(false);
        statusLabel->setText("Image supprimée");
        });
}

void MainWindow::addNewTab()
{
    MainWindow* newWindow = new MainWindow();
    newWindow->setWindowTitle(QString("EPO Barcode Validator - Onglet %1").arg(++tabCounter));
    newWindow->show();
}

void MainWindow::changeTheme()
{
    themeManager->switchTheme();
    applyCurrentTheme();
}

void MainWindow::importImage()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Importer une image contenant un code-barre EAN-13",
        "",
        "Images (*.png *.jpg *.jpeg *.bmp *.tiff);;Tous les fichiers (*.*)"
    );

    if (fileName.isEmpty())
        return;

    QImage image(fileName);
    if (image.isNull()) {
        QMessageBox::critical(this, "Erreur", "Impossible de charger l'image.\n\n"
            "Vérifiez que :\n"
            "• Le fichier existe\n"
            "• Le format est supporté (PNG, JPG, JPEG, BMP, TIFF)\n"
            "• Le fichier n'est pas corrompu");
        return;
    }

    // Afficher l'image originale
    imagePreview->setPixmap(QPixmap::fromImage(image).scaled(400, 400, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    imagePreview->setVisible(true);

    // Afficher les nouveaux contrôles
    analyzeImageButton->setVisible(true);
    deleteImageButton->setVisible(true);
    analysisStatusLabel->setVisible(true);
    imageProgressBar->setVisible(false);

    // Stocker l'image importée pour réutilisation
    currentImage = image;

    analysisStatusLabel->setText(QString("Image chargée (%1x%2) - Cliquez sur 'Analyser' pour détecter le code-barre")
        .arg(image.width()).arg(image.height()));

    qDebug() << "Image importée - prête pour l'analyse automatique";
}

void MainWindow::startScanner()
{
    stackedWidget->setCurrentWidget(scannerView);
}

void MainWindow::manualEntry()
{
    bool ok;
    QString barcode = QInputDialog::getText(
        this,
        "Saisie Manuelle",
        "Entrez le code-barre EAN-13 (13 chiffres):\n\n"
        "Exemple: 1234567890128\n"
        "Le 13ème chiffre est la clé de contrôle qui sera vérifiée.",
        QLineEdit::Normal,
        "",
        &ok
    );

    if (ok && !barcode.isEmpty()) {
        // Nettoyer la saisie (supprimer espaces, tirets, etc.)
        static const QRegularExpression nonDigitRegex("[^0-9]");
        barcode = barcode.remove(nonDigitRegex);

        if (barcode.length() != 13) {
            QMessageBox::warning(this, "Erreur de saisie",
                QString("Le code-barre doit contenir exactement 13 chiffres.\n"
                    "Vous avez saisi %1 chiffres: %2")
                .arg(barcode.length()).arg(barcode));
            return;
        }

        statusLabel->setText("Validation du code-barre saisi...");

        // Marquer comme saisie manuelle pour la sauvegarde
        QObject* sender = new QObject();
        sender->setObjectName("manualEntry");

        barcodeValidator->validateBarcode(barcode);

        sender->deleteLater();
    }
}

void MainWindow::showFAQ()
{
    stackedWidget->setCurrentWidget(faqView);
}

void MainWindow::showHistory()
{
    updateHistoryTable();
    stackedWidget->setCurrentWidget(historyView);
}

void MainWindow::onBarcodeValidated(const QString& barcode, bool isValid)
{
    progressBar->setVisible(false);

    if (isValid) {
        statusLabel->setText(QString("Code-barre validé: %1").arg(barcode));

        // Déterminer la méthode utilisée
        QString method = "Import"; // Par défaut
        if (stackedWidget->currentWidget() == scannerView) {
            method = "Scanner";
        }
        else if (sender() && sender()->objectName() == "manualEntry") {
            method = "Manuel";
        }

        // Sauvegarder dans la base de données
        if (databaseManager->saveBarcodeToDatabase(barcode, method, true)) {
            QMessageBox::information(this, "Code-barre validé",
                QString("✅ Code-barre EAN-13 validé et enregistré\n\n"
                    "Code-barre: %1\n"
                    "Méthode: %2\n"
                    "Statut: Valide\n\n"
                    "Le code-barre a été ajouté à l'historique.")
                .arg(barcode, method));
        }
        else {
            QMessageBox::warning(this, "Avertissement",
                QString("Code-barre validé mais erreur de sauvegarde.\n"
                    "Code-barre: %1").arg(barcode));
        }
    }
    else {
        if (barcode.isEmpty()) {
            statusLabel->setText("Aucun code-barre détecté");
            QMessageBox::information(this, "Analyse terminée",
                "Aucun code-barre EAN-13 valide n'a été détecté dans l'image.\n\n"
                "Conseils :\n"
                "• Assurez-vous que l'image contient un code-barre visible\n"
                "• Vérifiez que le code-barre est net et bien éclairé\n"
                "• Utilisez la saisie manuelle si nécessaire");
        }
        else {
            statusLabel->setText(QString("Code-barre invalide: %1").arg(barcode));

            // Sauvegarder même les codes-barres invalides pour l'historique
            databaseManager->saveBarcodeToDatabase(barcode, "Import", false);

            QMessageBox::warning(this, "Code-barre invalide",
                QString("❌ Le code-barre détecté n'est pas valide\n\n"
                    "Code-barre: %1\n"
                    "Problème: Clé de contrôle incorrecte\n\n"
                    "Vérifiez que tous les chiffres sont corrects.")
                .arg(barcode));
        }
    }
}

void MainWindow::updateHistoryTable()
{
    QList<BarcodeRecord> records = databaseManager->getAllBarcodes();

    historyTable->setRowCount(records.size());

    for (int i = 0; i < records.size(); ++i) {
        const BarcodeRecord& record = records[i];

        historyTable->setItem(i, 0, new QTableWidgetItem(record.barcode));
        historyTable->setItem(i, 1, new QTableWidgetItem(record.timestamp.toString()));
        historyTable->setItem(i, 2, new QTableWidgetItem(record.method));
        historyTable->setItem(i, 3, new QTableWidgetItem(record.isValid ? "Valide" : "Invalide"));
    }
}

void MainWindow::applyCurrentTheme()
{
    QString theme = themeManager->getCurrentTheme();

    setStyleSheet(QString(
        "QMainWindow { background-color: %1; }"
        "QFrame { %2 }"
        "QPushButton { %3 }"
    ).arg(themeManager->getBackgroundColor())
        .arg(getFrameStyle())
        .arg(getButtonStyle()));
}

QString MainWindow::getButtonStyle() const
{
    return QString(
        "QPushButton {"
        "background-color: %1;"
        "border: 2px solid %2;"
        "border-radius: 8px;"
        "color: white;"
        "font-weight: bold;"
        "font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "background-color: %3;"
        "}"
        "QPushButton:pressed {"
        "background-color: %4;"
        "}"
    ).arg(themeManager->getPrimaryColor())
        .arg(themeManager->getBorderColor())
        .arg(themeManager->getHoverColor())
        .arg(themeManager->getPressedColor());
}

QString MainWindow::getFrameStyle() const
{
    return QString(
        "QFrame {"
        "background-color: %1;"
        "border: 1px solid %2;"
        "border-radius: 5px;"
        "}"
    ).arg(themeManager->getFrameColor())
        .arg(themeManager->getBorderColor());
}

void MainWindow::setupImageProcessingView()
{
    // Zone d'aperçu de l'image
    imagePreview = new QLabel();
    imagePreview->setMinimumSize(400, 300);
    imagePreview->setMaximumSize(600, 450);
    imagePreview->setStyleSheet(
        "QLabel {"
        "border: 2px dashed #2E86AB;"
        "border-radius: 8px;"
        "background-color: #f8f9fa;"
        "color: #666;"
        "font-size: 14px;"
        "}"
    );
    imagePreview->setAlignment(Qt::AlignCenter);
    imagePreview->setScaledContents(false);
    imagePreview->setText("Aucune image chargée\n\nUtilisez le bouton 'Importer' pour charger une image");
    imagePreview->setVisible(false);

    // Nouveau bouton d'analyse automatique
    analyzeImageButton = new QPushButton("🔍 Analyser le Code-Barre");
    analyzeImageButton->setIcon(QIcon(":/icons/analyze.png"));
    analyzeImageButton->setFixedSize(180, 40);
    analyzeImageButton->setToolTip("Analyser automatiquement l'image pour détecter le code-barre EAN-13");
    analyzeImageButton->setStyleSheet(
        "QPushButton {"
        "background-color: #28a745;"
        "border: 2px solid #1e7e34;"
        "border-radius: 8px;"
        "color: white;"
        "font-weight: bold;"
        "font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "background-color: #218838;"
        "}"
        "QPushButton:pressed {"
        "background-color: #1e7e34;"
        "}"
        "QPushButton:disabled {"
        "background-color: #6c757d;"
        "border-color: #6c757d;"
        "}"
    );
    analyzeImageButton->setVisible(false);

    // Bouton de suppression
    deleteImageButton = new QPushButton("🗑️ Supprimer");
    deleteImageButton->setIcon(QIcon(":/icons/delete.png"));
    deleteImageButton->setFixedSize(120, 35);
    deleteImageButton->setToolTip("Supprimer l'image chargée");
    deleteImageButton->setVisible(false);

    // Label de statut d'analyse
    analysisStatusLabel = new QLabel();
    analysisStatusLabel->setStyleSheet(
        "QLabel {"
        "color: #495057;"
        "font-size: 12px;"
        "padding: 5px;"
        "}"
    );
    analysisStatusLabel->setAlignment(Qt::AlignCenter);
    analysisStatusLabel->setVisible(false);

    // Barre de progression pour l'analyse
    imageProgressBar = new QProgressBar();
    imageProgressBar->setTextVisible(true);
    imageProgressBar->setFormat("Analyse en cours... %p%");
    imageProgressBar->setFixedHeight(20);
    imageProgressBar->setStyleSheet(
        "QProgressBar {"
        "border: 2px solid #2E86AB;"
        "border-radius: 5px;"
        "text-align: center;"
        "}"
        "QProgressBar::chunk {"
        "background-color: #28a745;"
        "border-radius: 3px;"
        "}"
    );
    imageProgressBar->setVisible(false);
}

QString MainWindow::calculerCleChiffrement(const QString& douzeChiffres)
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

QImage MainWindow::enhanceImageForBarcode(const QImage& originalImage)
{
    if (originalImage.isNull()) {
        return QImage();
    }

    // Convertir en niveaux de gris
    QImage grayImage = originalImage.convertToFormat(QImage::Format_Grayscale8);

    // Améliorer le contraste
    QImage enhancedImage(grayImage.size(), QImage::Format_Grayscale8);

    // Calculer l'histogramme
    int histogram[256] = { 0 };
    for (int y = 0; y < grayImage.height(); ++y) {
        const uchar* line = grayImage.constScanLine(y);
        for (int x = 0; x < grayImage.width(); ++x) {
            histogram[line[x]]++;
        }
    }

    // Trouver min et max non-zéro
    int minVal = 0, maxVal = 255;
    for (int i = 0; i < 256; ++i) {
        if (histogram[i] > 0) {
            minVal = i;
            break;
        }
    }
    for (int i = 255; i >= 0; --i) {
        if (histogram[i] > 0) {
            maxVal = i;
            break;
        }
    }

    // Étirement de contraste
    double scale = 255.0 / (maxVal - minVal);

    for (int y = 0; y < grayImage.height(); ++y) {
        const uchar* srcLine = grayImage.constScanLine(y);
        uchar* dstLine = enhancedImage.scanLine(y);

        for (int x = 0; x < grayImage.width(); ++x) {
            int newVal = static_cast<int>((srcLine[x] - minVal) * scale);
            dstLine[x] = static_cast<uchar>(qBound(0, newVal, 255));
        }
    }

    return enhancedImage;
}
QString MainWindow::detectBarcodeFromImage(const QImage& image)
{
    if (image.isNull()) {
        return QString();
    }

    // Essayer avec l'image en niveaux de gris (format Lum pour ZXing)
    QImage grayImage = image.convertToFormat(QImage::Format_Grayscale8);

    ZXing::ImageView zxingImage(
        grayImage.constBits(),
        grayImage.width(),
        grayImage.height(),
        ZXing::ImageFormat::Lum
    );

    ZXing::ReaderOptions options;
    options.setTryHarder(true);
    options.setTryRotate(true);
    options.setFormats({ ZXing::BarcodeFormat::EAN13 });

    ZXing::Result result = ZXing::ReadBarcode(zxingImage, options);

    if (result.isValid() && !result.text().empty()) {
        QString barcode = QString::fromStdString(result.text());
        // Nettoyer le code-barre (garder seulement les chiffres)
        static const QRegularExpression nonDigitRegex("[^0-9]");
        barcode = barcode.remove(nonDigitRegex);

        qDebug() << "Code-barre détecté (niveaux de gris):" << barcode;
        return barcode;
    }

    // Si échec, essayer avec l'image RGB
    QImage rgbImage = image.convertToFormat(QImage::Format_RGB888);

    ZXing::ImageView zxingImageRGB(
        rgbImage.constBits(),
        rgbImage.width(),
        rgbImage.height(),
        ZXing::ImageFormat::RGB
    );

    ZXing::Result resultRGB = ZXing::ReadBarcode(zxingImageRGB, options);

    if (resultRGB.isValid() && !resultRGB.text().empty()) {
        QString barcode = QString::fromStdString(resultRGB.text());
        // Nettoyer le code-barre (garder seulement les chiffres)
        static const QRegularExpression nonDigitRegex("[^0-9]");
        barcode = barcode.remove(nonDigitRegex);

        qDebug() << "Code-barre détecté (RGB):" << barcode;
        return barcode;
    }

    // Si toujours échec, essayer avec une image binarisée
    QImage binaryImage = createBinaryImage(grayImage);

    ZXing::ImageView zxingImageBinary(
        binaryImage.constBits(),
        binaryImage.width(),
        binaryImage.height(),
        ZXing::ImageFormat::Lum
    );

    ZXing::Result resultBinary = ZXing::ReadBarcode(zxingImageBinary, options);

    if (resultBinary.isValid() && !resultBinary.text().empty()) {
        QString barcode = QString::fromStdString(resultBinary.text());
        // Nettoyer le code-barre (garder seulement les chiffres)
        static const QRegularExpression nonDigitRegex("[^0-9]");
        barcode = barcode.remove(nonDigitRegex);

        qDebug() << "Code-barre détecté (binaire):" << barcode;
        return barcode;
    }

    qDebug() << "Aucun code-barre détecté avec les 3 méthodes";
    return QString();
}
QImage MainWindow::createBinaryImage(const QImage& grayImage)
{
    if (grayImage.isNull()) {
        return QImage();
    }

    // Calculer le seuil automatiquement (méthode d'Otsu simplifiée)
    int histogram[256] = { 0 };
    int totalPixels = grayImage.width() * grayImage.height();

    // Construire l'histogramme
    for (int y = 0; y < grayImage.height(); ++y) {
        const uchar* line = grayImage.constScanLine(y);
        for (int x = 0; x < grayImage.width(); ++x) {
            histogram[line[x]]++;
        }
    }

    // Trouver le seuil optimal (version simplifiée)
    int sum = 0;
    for (int i = 0; i < 256; ++i) {
        sum += i * histogram[i];
    }

    int sumB = 0;
    int wB = 0;
    int wF = 0;
    double mB, mF, max = 0.0;
    double between = 0.0;
    int threshold1 = 0, threshold2 = 0;

    for (int i = 0; i < 256; ++i) {
        wB += histogram[i];
        if (wB == 0) continue;
        wF = totalPixels - wB;
        if (wF == 0) break;

        sumB += i * histogram[i];
        mB = (double)sumB / wB;
        mF = (double)(sum - sumB) / wF;
        between = wB * wF * (mB - mF) * (mB - mF);

        if (between >= max) {
            threshold1 = i;
            if (between > max) {
                threshold2 = i;
            }
            max = between;
        }
    }

    int threshold = (threshold1 + threshold2) / 2;

    // Créer l'image binaire
    QImage binaryImage(grayImage.size(), QImage::Format_Grayscale8);

    for (int y = 0; y < grayImage.height(); ++y) {
        const uchar* srcLine = grayImage.constScanLine(y);
        uchar* dstLine = binaryImage.scanLine(y);

        for (int x = 0; x < grayImage.width(); ++x) {
            dstLine[x] = (srcLine[x] > threshold) ? 255 : 0;
        }
    }

    return binaryImage;
}

void MainWindow::processImageAutomatically()
{
    if (currentImage.isNull()) {
        QMessageBox::warning(this, "Erreur", "Aucune image chargée.");
        return;
    }

    // Afficher la barre de progression et le statut
    imageProgressBar->setVisible(true);
    imageProgressBar->setValue(0);
    analysisStatusLabel->setText("Analyse en cours...");
    analyzeImageButton->setEnabled(false);

    // Démarrer l'analyse
    analyzeImageWithMultipleTechniques(currentImage);
}

void MainWindow::analyzeImageWithMultipleTechniques(const QImage& image)
{
    QStringList detectedCodes;
    
    // Technique 1: Image originale
    imageProgressBar->setValue(10);
    analysisStatusLabel->setText("Analyse avec l'image originale...");
    QApplication::processEvents();
    
    QString code = detectBarcodeFromImage(image);
    if (!code.isEmpty()) {
        detectedCodes.append(code);
    }

    // Technique 2: Image avec amélioration du contraste
    imageProgressBar->setValue(25);
    analysisStatusLabel->setText("Amélioration du contraste...");
    QApplication::processEvents();
    
    QImage enhancedImage = enhanceImageForBarcode(image);
    code = detectBarcodeFromImage(enhancedImage);
    if (!code.isEmpty() && !detectedCodes.contains(code)) {
        detectedCodes.append(code);
    }

    // Technique 3: Image avec flou gaussien pour réduire le bruit
    imageProgressBar->setValue(40);
    analysisStatusLabel->setText("Application du filtre anti-bruit...");
    QApplication::processEvents();
    
    QImage blurredImage = applyGaussianBlur(image);
    code = detectBarcodeFromImage(blurredImage);
    if (!code.isEmpty() && !detectedCodes.contains(code)) {
        detectedCodes.append(code);
    }

    // Technique 4: Image avec filtre de netteté
    imageProgressBar->setValue(55);
    analysisStatusLabel->setText("Application du filtre de netteté...");
    QApplication::processEvents();
    
    QImage sharpenedImage = applySharpenFilter(image);
    code = detectBarcodeFromImage(sharpenedImage);
    if (!code.isEmpty() && !detectedCodes.contains(code)) {
        detectedCodes.append(code);
    }

    // Technique 5: Ajustement gamma (éclaircissement)
    imageProgressBar->setValue(70);
    analysisStatusLabel->setText("Ajustement de la luminosité...");
    QApplication::processEvents();
    
    QImage gammaImage = adjustGamma(image, 1.5);
    code = detectBarcodeFromImage(gammaImage);
    if (!code.isEmpty() && !detectedCodes.contains(code)) {
        detectedCodes.append(code);
    }

    // Technique 6: Ajustement gamma (assombrissement)
    imageProgressBar->setValue(85);
    analysisStatusLabel->setText("Optimisation finale...");
    QApplication::processEvents();
    
    QImage darkGammaImage = adjustGamma(image, 0.7);
    code = detectBarcodeFromImage(darkGammaImage);
    if (!code.isEmpty() && !detectedCodes.contains(code)) {
        detectedCodes.append(code);
    }

    // Finalisation
    imageProgressBar->setValue(100);
    analyzeImageButton->setEnabled(true);

    // Traitement des résultats
    onImageProcessingFinished(detectedCodes.isEmpty() ? QString() : detectedCodes.first(), !detectedCodes.isEmpty());
}

QStringList MainWindow::tryMultipleDetectionMethods(const QImage& image)
{
    QStringList results;
    
    // Méthode 1: Image originale
    QString code = detectBarcodeFromImage(image);
    if (!code.isEmpty()) results.append(code);
    
    // Méthode 2: Image prétraitée
    QImage processed = enhanceImageForBarcode(image);
    code = detectBarcodeFromImage(processed);
    if (!code.isEmpty() && !results.contains(code)) results.append(code);
    
    // Méthode 3: Image floue
    QImage blurred = applyGaussianBlur(image);
    code = detectBarcodeFromImage(blurred);
    if (!code.isEmpty() && !results.contains(code)) results.append(code);
    
    return results;
}

QImage MainWindow::applyGaussianBlur(const QImage& image)
{
    if (image.isNull()) return QImage();
    
    QImage result = image.convertToFormat(QImage::Format_ARGB32);
    
    // Implémentation simple d'un flou gaussien 3x3
    const int kernel[3][3] = {
        {1, 2, 1},
        {2, 4, 2},
        {1, 2, 1}
    };
    const int kernelSum = 16;
    
    for (int y = 1; y < image.height() - 1; ++y) {
        for (int x = 1; x < image.width() - 1; ++x) {
            int r = 0, g = 0, b = 0;
            
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    QRgb pixel = image.pixel(x + kx, y + ky);
                    int weight = kernel[ky + 1][kx + 1];
                    r += qRed(pixel) * weight;
                    g += qGreen(pixel) * weight;
                    b += qBlue(pixel) * weight;
                }
            }
            
            r /= kernelSum;
            g /= kernelSum;
            b /= kernelSum;
            
            result.setPixel(x, y, qRgb(r, g, b));
        }
    }
    
    return result;
}

QImage MainWindow::applySharpenFilter(const QImage& image)
{
    if (image.isNull()) return QImage();
    
    QImage result = image.convertToFormat(QImage::Format_ARGB32);
    
    // Filtre de netteté 3x3
    const int kernel[3][3] = {
        { 0, -1,  0},
        {-1,  5, -1},
        { 0, -1,  0}
    };
    
    for (int y = 1; y < image.height() - 1; ++y) {
        for (int x = 1; x < image.width() - 1; ++x) {
            int r = 0, g = 0, b = 0;
            
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    QRgb pixel = image.pixel(x + kx, y + ky);
                    int weight = kernel[ky + 1][kx + 1];
                    r += qRed(pixel) * weight;
                    g += qGreen(pixel) * weight;
                    b += qBlue(pixel) * weight;
                }
            }
            
            r = qBound(0, r, 255);
            g = qBound(0, g, 255);
            b = qBound(0, b, 255);
            
            result.setPixel(x, y, qRgb(r, g, b));
        }
    }
    
    return result;
}

QImage MainWindow::adjustGamma(const QImage& image, double gamma)
{
    if (image.isNull()) return QImage();
    
    QImage result = image.convertToFormat(QImage::Format_ARGB32);
    
    // Créer une table de correspondance gamma
    uchar gammaTable[256];
    for (int i = 0; i < 256; ++i) {
        gammaTable[i] = static_cast<uchar>(qBound(0.0, 255.0 * pow(i / 255.0, 1.0 / gamma), 255.0));
    }
    
    // Appliquer la correction gamma
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QRgb pixel = image.pixel(x, y);
            int r = gammaTable[qRed(pixel)];
            int g = gammaTable[qGreen(pixel)];
            int b = gammaTable[qBlue(pixel)];
            
            result.setPixel(x, y, qRgb(r, g, b));
        }
    }
    
    return result;
}

void MainWindow::onImageProcessingFinished(const QString& barcode, bool success)
{
    imageProgressBar->setVisible(false);
    
    if (success && !barcode.isEmpty()) {
        // Vérifier que c'est un EAN-13 valide
        if (barcode.length() == 13) {
            // Vérifier la clé de contrôle
            QString calculatedKey = calculerCleChiffrement(barcode.left(12));
            bool isValid = (calculatedKey == barcode.right(1));
            
            // Sauvegarder en base de données
            databaseManager->saveBarcodeToDatabase(barcode, "Import", isValid);
            
            if (isValid) {
                analysisStatusLabel->setText(QString("✅ Code-barre EAN-13 détecté et validé: %1").arg(barcode));
                QMessageBox::information(this, "Code-barre détecté",
                    QString("🎯 Code-barre EAN-13 validé avec succès!\n\n"
                            "Code-barre: %1\n"
                            "Clé de contrôle: Correcte ✅\n"
                            "Statut: Enregistré dans l'historique")
                    .arg(barcode));
            } else {
                analysisStatusLabel->setText(QString("⚠️ Code-barre détecté mais clé invalide: %1").arg(barcode));
                QMessageBox::warning(this, "Code-barre invalide",
                    QString("Code-barre EAN-13 détecté mais invalide\n\n"
                            "Code-barre: %1\n"
                            "Problème: Clé de contrôle incorrecte\n"
                            "Le code a été enregistré comme invalide.")
                    .arg(barcode));
            }
        } else {
            analysisStatusLabel->setText("❌ Code détecté mais ce n'est pas un EAN-13");
            QMessageBox::information(this, "Format incorrect",
                QString("Un code a été détecté mais ce n'est pas un EAN-13.\n\n"
                        "Code détecté: %1\n"
                        "Longueur: %2 chiffres (13 attendus)")
                .arg(barcode).arg(barcode.length()));
        }
    } else {
        analysisStatusLabel->setText("❌ Aucun code-barre EAN-13 détecté");
        QMessageBox::information(this, "Analyse terminée",
            "🔍 Aucun code-barre EAN-13 n'a été détecté dans cette image.\n\n"
            "💡 Conseils pour améliorer la détection:\n"
            "• Assurez-vous que le code-barre est visible et net\n"
            "• Vérifiez l'éclairage de l'image\n"
            "• Essayez avec une image de meilleure qualité\n"
            "• Utilisez la saisie manuelle si nécessaire");
    }
    
    statusLabel->setText(success ? "Analyse terminée avec succès" : "Analyse terminée - aucun code détecté");
}

void MainWindow::onImageProcessingProgress(int percentage)
{
    imageProgressBar->setValue(percentage);
}