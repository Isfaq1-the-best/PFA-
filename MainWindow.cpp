#include "MainWindow.h"
#include <QApplication>
#include <QHeaderView>
#include <QSplitter>
#include <QStatusBar>
#include <QDesktopServices>
#include <QUrl>

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

    QHBoxLayout* imageButtonsLayout = new QHBoxLayout();
    imageButtonsLayout->addWidget(googleLensResultButton);
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
        "<p><b>Importer :</b> Permet d'importer des images (JPEG, JPG, PNG) et redirige automatiquement vers Google Lens pour l'analyse des codes-barres.</p>"
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
        "<h3>Utilisation de Google Lens</h3>"
        "<p>Lorsque vous importez une image :</p>"
        "<ol>"
        "<li>L'application ouvre automatiquement Google Lens dans votre navigateur</li>"
        "<li>Cliquez sur l'icône d'appareil photo dans Google Lens</li>"
        "<li>Sélectionnez l'image que vous venez d'importer</li>"
        "<li>Google Lens analysera le code-barre et affichera les chiffres</li>"
        "<li>Copiez les chiffres détectés</li>"
        "<li>Revenez dans cette application et utilisez 'Saisie Manuelle' pour coller les chiffres</li>"
        "<li>L'application vérifiera automatiquement la clé de chiffrement</li>"
        "</ol>"
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

    // Connexions pour le traitement d'images
    connect(googleLensResultButton, &QPushButton::clicked, this, &MainWindow::handleGoogleLensResult);

    connect(deleteImageButton, &QPushButton::clicked, this, [this]() {
        currentImage = QImage();
        imagePreview->clear();
        imagePreview->setText("Aucune image chargée\n\nUtilisez le bouton 'Importer' pour charger une image");
        imagePreview->setVisible(false);
        googleLensResultButton->setVisible(false);
        deleteImageButton->setVisible(false);
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
        "Importer une image",
        "",
        "Images (*.png *.jpg *.jpeg);;Tous les fichiers (*.*)"
    );

    if (fileName.isEmpty())
        return;

    QImage image(fileName);
    if (image.isNull()) {
        QMessageBox::critical(this, "Erreur", "Impossible de charger l'image.\n\n"
            "Vérifiez que :\n"
            "• Le fichier existe\n"
            "• Le format est supporté (PNG, JPG, JPEG)\n"
            "• Le fichier n'est pas corrompu");
        return;
    }

    // Stocker l'image importée pour réutilisation
    currentImage = image;

    // Afficher l'image originale
    imagePreview->setPixmap(QPixmap::fromImage(image).scaled(400, 400, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    imagePreview->setVisible(true);

    // Afficher le bouton pour gérer les résultats Google Lens
    googleLensResultButton->setVisible(true);
    deleteImageButton->setVisible(true);

    // Rediriger vers Google Lens pour l'analyse
    redirectToGoogleLens(fileName);

    statusLabel->setText(QString("Image chargée (%1x%2) - Redirection vers Google Lens...")
        .arg(image.width()).arg(image.height()));

    qDebug() << "Image importée et redirection vers Google Lens";
}

void MainWindow::redirectToGoogleLens(const QString& imagePath)
{
    // Créer une URL Google Lens avec l'image
    QUrl googleLensUrl("https://lens.google.com/");
    
    // Ouvrir Google Lens dans le navigateur par défaut
    QDesktopServices::openUrl(googleLensUrl);
    
    // Afficher un message à l'utilisateur
    QMessageBox::information(this, "Redirection vers Google Lens",
        "Google Lens s'ouvre dans votre navigateur.\n\n"
        "Instructions :\n"
        "1. Cliquez sur l'icône d'appareil photo dans Google Lens\n"
        "2. Sélectionnez l'image que vous venez d'importer\n"
        "3. Google Lens analysera le code-barre\n"
        "4. Copiez les chiffres détectés\n"
        "5. Revenez dans cette application et cliquez sur 'Saisie Manuelle'\n"
        "6. Collez les chiffres pour validation\n\n"
        "Note : L'image reste disponible dans l'application pour référence.");
}

void MainWindow::handleGoogleLensResult()
{
    // Cette méthode peut être utilisée pour traiter les résultats
    // si on implémente une intégration plus avancée avec Google Lens
    QMessageBox::information(this, "Résultat Google Lens",
        "Pour utiliser les résultats de Google Lens :\n\n"
        "1. Copiez les chiffres détectés par Google Lens\n"
        "2. Cliquez sur 'Saisie Manuelle' dans cette application\n"
        "3. Collez les chiffres dans le champ de saisie\n"
        "4. L'application vérifiera automatiquement la clé de chiffrement");
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

    // Boutons de traitement
    googleLensResultButton = new QPushButton("Résultat\nGoogle Lens");
    googleLensResultButton->setIcon(QIcon(":/icons/lens.png"));
    googleLensResultButton->setFixedSize(120, 50);
    googleLensResultButton->setToolTip("Gérer les résultats de Google Lens");
    googleLensResultButton->setVisible(false);

    deleteImageButton = new QPushButton("Supprimer");
    deleteImageButton->setIcon(QIcon(":/icons/delete.png"));
    deleteImageButton->setFixedSize(120, 35);
    deleteImageButton->setToolTip("Supprimer l'image chargée");
    deleteImageButton->setVisible(false);
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
    // Cette fonction n'est plus utilisée car nous redirigeons vers Google Lens
    // Retourner un message informatif
    Q_UNUSED(image)
    return QString("Utilisez Google Lens pour analyser le code-barre");
}
QImage MainWindow::createBinaryImage(const QImage& grayImage)
{
    // Cette fonction n'est plus utilisée car nous redirigeons vers Google Lens
    Q_UNUSED(grayImage)
    return QImage();
}