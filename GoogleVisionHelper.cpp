#include "GoogleVisionHelper.h"
#include <QBuffer>
#include <QByteArray>
#include <QRegularExpression>
#include <QDebug>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

namespace {
// Fonction utilitaire pour encoder une QImage en base64 (JPEG)
QString imageToBase64(const QImage& image) {
    QByteArray ba;
    QBuffer buffer(&ba);
    image.save(&buffer, "JPEG");
    return ba.toBase64();
}

// Callback pour libcurl (écriture de la réponse)
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
}

namespace GoogleVisionHelper {

QString detectBarcodeWithGoogleVision(const QImage& image, const QString& apiKey) {
    QString base64 = imageToBase64(image);

    nlohmann::json request_json = {
        {"requests", {{
            {"image", {{"content", base64.toStdString()}}},
            {"features", {{{"type", "TEXT_DETECTION"}}}}
        }}}
    };

    CURL* curl = curl_easy_init();
    std::string response_string;
    if (curl) {
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        std::string url = "https://vision.googleapis.com/v1/images:annotate?key=" + apiKey.toStdString();
        std::string request_str = request_json.dump();

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_str.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            qDebug() << "Erreur curl:" << curl_easy_strerror(res);
        }
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    // Analyse du JSON pour extraire le code-barre
    auto json = nlohmann::json::parse(response_string, nullptr, false);
    if (json.is_discarded()) return QString();

    try {
        auto text = json["responses"][0]["textAnnotations"][0]["description"].get<std::string>();
        // Chercher un EAN-13 (13 chiffres consécutifs)
        QRegularExpression re("\\b\\d{13}\\b");
        QRegularExpressionMatch match = re.match(QString::fromStdString(text));
        if (match.hasMatch()) {
            return match.captured(0);
        }
    } catch (...) {
        // Rien trouvé
    }
    return QString();
}

} // namespace GoogleVisionHelper