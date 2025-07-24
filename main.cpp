#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include "SplashWindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Configuration de l'application
    app.setApplicationName("EPO Barcode Validator");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("EPO Project");

    // Création et affichage de l'écran de démarrage
    SplashWindow splash;
    splash.show();

    return app.exec();
}
