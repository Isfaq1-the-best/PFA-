#pragma once
#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QDateTime>
#include <QList>
#include <QMutex>
#include <QVariant>

struct BarcodeRecord {
    int id;
    QString barcode;
    QDateTime timestamp;
    QString method;  // "Import", "Scanner", "Manuel"
    bool isValid;

    BarcodeRecord() : id(-1), isValid(false) {}
    BarcodeRecord(const QString& bc, const QString& m, bool valid)
        : id(-1), barcode(bc), timestamp(QDateTime::currentDateTime()),
        method(m), isValid(valid) {}
};

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject* parent = nullptr);
    ~DatabaseManager();

    // Méthodes de gestion de la base de données
    bool initializeDatabase();
    bool saveBarcodeToDatabase(const QString& barcode, const QString& method, bool isValid);
    QList<BarcodeRecord> getAllBarcodes();
    QList<BarcodeRecord> getBarcodesByDate(const QDate& date);
    QList<BarcodeRecord> getValidBarcodes();
    bool deleteBarcodeRecord(int id);
    bool clearAllRecords();

    // Statistiques
    int getTotalBarcodeCount();
    int getValidBarcodeCount();
    int getInvalidBarcodeCount();

    // Export
    bool exportToCSV(const QString& filePath);
    bool exportToJSON(const QString& filePath);

signals:
    void databaseError(const QString& error);
    void recordSaved(const BarcodeRecord& record);

private:
    QSqlDatabase database;
    QMutex databaseMutex;

    bool createTables();
    bool executeQuery(const QString& queryString, const QVariantList& parameters = QVariantList());
    QSqlQuery prepareQuery(const QString& queryString);
    void logError(const QString& operation, const QSqlError& error);
};

#endif // DATABASEMANAGER_H
