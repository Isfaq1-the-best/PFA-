#include "MainWindow.h"
#include <QApplication>
#include <QHeaderView>
#include <QSplitter>
#include <QStatusBar>

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
    imageButtonsLayout->addWidget(convertButton);
    imageButtonsLayout->addWidget(decodeButton);
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

    // Connexions pour le traitement d'images
    connect(convertButton, &QPushButton::clicked, this, [this]() {
        if (currentImage.isNull()) {
            QMessageBox::warning(this, "Erreur", "Aucune image chargée.");
            return;
        }

        // 1. Convertir en RGB888 si nécessaire
        QImage processedImage = currentImage;
        if (processedImage.format() != QImage::Format_RGB888) {
            processedImage = processedImage.convertToFormat(QImage::Format_RGB888);
        }

        // 2. Redimensionner si trop petit
        if (processedImage.width() < 500 || processedImage.height() < 500) {
            processedImage = processedImage.scaled(800, 600, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        // 3. Convertir en niveaux de gris
        QImage grayImage(processedImage.size(), QImage::Format_Grayscale8);
        for (int y = 0; y < processedImage.height(); ++y) {
            const uchar* rgbLine = processedImage.constScanLine(y);
            uchar* grayLine = grayImage.scanLine(y);

            for (int x = 0; x < processedImage.width(); ++x) {
                int r = rgbLine[x * 3];
                int g = rgbLine[x * 3 + 1];
                int b = rgbLine[x * 3 + 2];
                int gray = static_cast<int>(0.3 * r + 0.59 * g + 0.11 * b);
                grayLine[x] = static_cast<uchar>(gray);
            }
        }

        // 4. Mettre à jour l'image courante
        currentImage = grayImage;
        qDebug() << "Image convertie : taille =" << currentImage.size() << ", format =" << currentImage.format();

        // 5. Afficher l'image convertie
        imagePreview->setPixmap(QPixmap::fromImage(currentImage).scaled(
            imagePreview->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        ));

        QMessageBox::information(this, "Conversion", "Image prétraitée pour la détection du code-barres.");
        });

    connect(decodeButton, &QPushButton::clicked, this, [this]() {
        if (currentImage.isNull()) {
            QMessageBox::warning(this, "Attention", "Aucune image chargée.");
            return;
        }

        // 1. Vérification et conversion exactement comme votre code
        QImage imageForZXing = currentImage;
        if (imageForZXing.format() != QImage::Format_RGB888) {
            qDebug() << "Conversion en RGB888...";
            imageForZXing = imageForZXing.convertToFormat(QImage::Format_RGB888);
        }

        if (imageForZXing.isNull()) {
            QMessageBox::critical(this, "Erreur", "L'image à décoder est invalide.");
            return;
        }

        qDebug() << "Décodage image : taille =" << imageForZXing.size() << ", format =" << imageForZXing.format();

        // 2. Configuration ZXing exactement comme votre code
        ZXing::ImageView zxingImage(
            imageForZXing.constBits(),
            imageForZXing.width(),
            imageForZXing.height(),
            ZXing::ImageFormat::RGB
        );

        ZXing::ReaderOptions options;
        options.setTryHarder(true);
        options.setFormats({ ZXing::BarcodeFormat::EAN13 });

        // 3. Lecture du code-barres
        ZXing::Result result = ZXing::ReadBarcode(zxingImage, options);

        // 4. Vérification de la validité exactement comme votre code
        if (!result.isValid()) {
            qDebug() << "Aucun code-barres détecté.";
            QMessageBox::information(this, "Résultat", "Aucun code-barres détecté.");
            return;
        }

        // 5. Informations de débogage
        QString brut = QString::fromStdString(result.text());
        ZXing::BarcodeFormat format = result.format();

        qDebug() << "Format détecté :" << QString::fromStdString(ZXing::ToString(format));
        qDebug() << "Texte brut détecté :" << brut << ", longueur =" << brut.length();

        if (brut.trimmed().isEmpty()) {
            QMessageBox::warning(this, "Erreur", "Un code-barres a été détecté, mais le texte est vide.");
            return;
        }

        // 6. Nettoyage du texte (garder uniquement les chiffres)
        static const QRegularExpression nonDigitRegex("[^0-9]");
        brut = brut.remove(nonDigitRegex);

        qDebug() << "Code nettoyé =" << brut << ", longueur =" << brut.length();

        // 7. Vérification longueur EAN13
        if (brut.length() != 13) {
            QMessageBox::warning(this, "Code-barres", "Longueur invalide : " + QString::number(brut.length()));
            return;
        }

        // 8. Vérification clé de contrôle
        QString cleCalculee = calculerCleChiffrement(brut.left(12));
        int cleBarcode = brut.right(1).toInt();

        // 9. Sauvegarde en base de données
        bool isValid = (cleCalculee.toInt() == cleBarcode);
        databaseManager->saveBarcodeToDatabase(brut, "Import", isValid);

        if (isValid) {
            QMessageBox::information(this, "Code-barres", "Code : " + brut + "\nClé correcte !");
        }
        else {
            QMessageBox::warning(this, "Code-barres", "Code : " + brut + "\nClé incorrecte !");
        }

        // 10. Affichage final
        QMessageBox::information(this, "Code-barres détecté", "Code : " + brut);
        statusLabel->setText(QString("Code-barre %1: %2").arg(isValid ? "validé" : "invalide", brut));
        });

    connect(deleteImageButton, &QPushButton::clicked, this, [this]() {
        currentImage = QImage();
        imagePreview->clear();
        imagePreview->setText("Aucune image chargée\n\nUtilisez le bouton 'Importer' pour charger une image");
        imagePreview->setVisible(false);
        convertButton->setVisible(false);
        decodeButton->setVisible(false);
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

    // Afficher l'image originale
    imagePreview->setPixmap(QPixmap::fromImage(image).scaled(400, 400, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    imagePreview->setVisible(true);

    // Afficher les boutons Convertir et Décoder
    convertButton->setVisible(true);
    decodeButton->setVisible(true);
    deleteImageButton->setVisible(true);

    // Stocker l'image importée pour réutilisation
    currentImage = image;

    statusLabel->setText(QString("Image chargée (%1x%2) - Utilisez 'Convertir' puis 'Décoder'")
        .arg(image.width()).arg(image.height()));

    qDebug() << "Image importée et boutons Convertir/Décoder affichés";
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
    convertButton = new QPushButton("Convertir");
    convertButton->setIcon(QIcon(":/icons/convert.png"));
    convertButton->setFixedSize(120, 35);
    convertButton->setToolTip("Prétraiter l'image pour améliorer la détection");
    convertButton->setVisible(false);

    decodeButton = new QPushButton("Décoder");
    decodeButton->setIcon(QIcon(":/icons/decode.png"));
    decodeButton->setFixedSize(120, 35);
    decodeButton->setToolTip("Analyser l'image pour détecter le code-barre");
    decodeButton->setVisible(false);

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