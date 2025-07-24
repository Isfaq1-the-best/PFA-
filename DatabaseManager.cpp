#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextStream>
#include <QFile>

DatabaseManager::DatabaseManager(QObject* parent)
    : QObject(parent)
{
}

DatabaseManager::~DatabaseManager()
{
    if (database.isOpen()) {
        database.close();
    }
}

bool DatabaseManager::initializeDatabase()
{
    QMutexLocker locker(&databaseMutex);

    // Créer le répertoire de données s'il n'existe pas
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dataDir;
    if (!dataDir.exists(dataPath)) {
        dataDir.mkpath(dataPath);
    }

    // Configuration de la base de données SQLite
    database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName(dataPath + "/barcode_database.db");

    if (!database.open()) {
        logError("Ouverture de la base de données", database.lastError());
        return false;
    }

    return createTables();
}

bool DatabaseManager::createTables()
{
    QString createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS barcodes (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            barcode TEXT NOT NULL,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
            method TEXT NOT NULL,
            is_valid BOOLEAN NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";

    QString createIndexQuery = R"(
        CREATE INDEX IF NOT EXISTS idx_barcode ON barcodes(barcode);
        CREATE INDEX IF NOT EXISTS idx_timestamp ON barcodes(timestamp);
        CREATE INDEX IF NOT EXISTS idx_valid ON barcodes(is_valid);
    )";

    QSqlQuery query(database);

    if (!query.exec(createTableQuery)) {
        logError("Création de la table", query.lastError());
        return false;
    }

    if (!query.exec(createIndexQuery)) {
        logError("Création des index", query.lastError());
        return false;
    }

    return true;
}

bool DatabaseManager::saveBarcodeToDatabase(const QString& barcode, const QString& method, bool isValid)
{
    QMutexLocker locker(&databaseMutex);

    QString queryString = R"(
        INSERT INTO barcodes (barcode, method, is_valid, timestamp)
        VALUES (?, ?, ?, ?)
    )";

    QVariantList parameters;
    parameters << barcode << method << isValid << QDateTime::currentDateTime();

    if (executeQuery(queryString, parameters)) {
        BarcodeRecord record(barcode, method, isValid);
        emit recordSaved(record);
        return true;
    }

    return false;
}

QList<BarcodeRecord> DatabaseManager::getAllBarcodes()
{
    QMutexLocker locker(&databaseMutex);
    QList<BarcodeRecord> records;

    QString queryString = R"(
        SELECT id, barcode, timestamp, method, is_valid
        FROM barcodes
        ORDER BY timestamp DESC
    )";

    QSqlQuery query = prepareQuery(queryString);

    if (query.exec()) {
        while (query.next()) {
            BarcodeRecord record;
            record.id = query.value(0).toInt();
            record.barcode = query.value(1).toString();
            record.timestamp = query.value(2).toDateTime();
            record.method = query.value(3).toString();
            record.isValid = query.value(4).toBool();
            records.append(record);
        }
    }
    else {
        logError("Récupération de tous les codes-barres", query.lastError());
    }

    return records;
}

QList<BarcodeRecord> DatabaseManager::getBarcodesByDate(const QDate& date)
{
    QMutexLocker locker(&databaseMutex);
    QList<BarcodeRecord> records;

    QString queryString = R"(
        SELECT id, barcode, timestamp, method, is_valid
        FROM barcodes
        WHERE DATE(timestamp) = ?
        ORDER BY timestamp DESC
    )";

    QSqlQuery query = prepareQuery(queryString);
    query.addBindValue(date.toString(Qt::ISODate));

    if (query.exec()) {
        while (query.next()) {
            BarcodeRecord record;
            record.id = query.value(0).toInt();
            record.barcode = query.value(1).toString();
            record.timestamp = query.value(2).toDateTime();
            record.method = query.value(3).toString();
            record.isValid = query.value(4).toBool();
            records.append(record);
        }
    }
    else {
        logError("Récupération des codes-barres par date", query.lastError());
    }

    return records;
}

QList<BarcodeRecord> DatabaseManager::getValidBarcodes()
{
    QMutexLocker locker(&databaseMutex);
    QList<BarcodeRecord> records;

    QString queryString = R"(
        SELECT id, barcode, timestamp, method, is_valid
        FROM barcodes
        WHERE is_valid = 1
        ORDER BY timestamp DESC
    )";

    QSqlQuery query = prepareQuery(queryString);

    if (query.exec()) {
        while (query.next()) {
            BarcodeRecord record;
            record.id = query.value(0).toInt();
            record.barcode = query.value(1).toString();
            record.timestamp = query.value(2).toDateTime();
            record.method = query.value(3).toString();
            record.isValid = query.value(4).toBool();
            records.append(record);
        }
    }
    else {
        logError("Récupération des codes-barres valides", query.lastError());
    }

    return records;
}

bool DatabaseManager::deleteBarcodeRecord(int id)
{
    QMutexLocker locker(&databaseMutex);

    QString queryString = "DELETE FROM barcodes WHERE id = ?";
    QVariantList parameters;
    parameters << id;

    return executeQuery(queryString, parameters);
}

bool DatabaseManager::clearAllRecords()
{
    QMutexLocker locker(&databaseMutex);

    QString queryString = "DELETE FROM barcodes";
    return executeQuery(queryString);
}

int DatabaseManager::getTotalBarcodeCount()
{
    QMutexLocker locker(&databaseMutex);

    QSqlQuery query = prepareQuery("SELECT COUNT(*) FROM barcodes");

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

int DatabaseManager::getValidBarcodeCount()
{
    QMutexLocker locker(&databaseMutex);

    QSqlQuery query = prepareQuery("SELECT COUNT(*) FROM barcodes WHERE is_valid = 1");

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

int DatabaseManager::getInvalidBarcodeCount()
{
    QMutexLocker locker(&databaseMutex);

    QSqlQuery query = prepareQuery("SELECT COUNT(*) FROM barcodes WHERE is_valid = 0");

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}
bool DatabaseManager::exportToJSON(const QString& filePath)
{
    QJsonArray jsonArray;

    QList<BarcodeRecord> records = getAllBarcodes();
    for (const BarcodeRecord& record : records) {
        QJsonObject jsonObject;
        jsonObject["id"] = record.id;
        jsonObject["barcode"] = record.barcode;
        jsonObject["timestamp"] = record.timestamp.toString(Qt::ISODate);
        jsonObject["method"] = record.method;
        jsonObject["isValid"] = record.isValid;
        jsonArray.append(jsonObject);
    }

    QJsonDocument document(jsonArray);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    file.write(document.toJson());
    return true;
}

bool DatabaseManager::executeQuery(const QString& queryString, const QVariantList& parameters)
{
    QSqlQuery query = prepareQuery(queryString);

    for (const QVariant& parameter : parameters) {
        query.addBindValue(parameter);
    }

    if (!query.exec()) {
        logError("Exécution de la requête", query.lastError());
        return false;
    }

    return true;
}

QSqlQuery DatabaseManager::prepareQuery(const QString& queryString)
{
    QSqlQuery query(database);
    query.prepare(queryString);
    return query;
}

void DatabaseManager::logError(const QString& operation, const QSqlError& error)
{
    QString errorMessage = QString("Erreur %1: %2").arg(operation, error.text());
    qDebug() << errorMessage;
    emit databaseError(errorMessage);
}