#include "util/json.h"

namespace mixxx {

namespace json {

Q_LOGGING_CATEGORY(kLogger, "mixxx.util.json")

std::pair<QJsonDocument, QJsonParseError::ParseError> parseDocument(
        const QByteArray& jsonData) {
    if (jsonData.isEmpty()) {
        return std::make_pair(QJsonDocument{}, QJsonParseError::NoError);
    }
    QJsonParseError parseError;
    const auto doc = QJsonDocument::fromJson(jsonData, &parseError);
    // QJsonDocument::fromJson() returns a non-null document
    // if parsing succeeds and otherwise null on error. The
    // parse error must only be evaluated if the returned
    // document is null, otherwise it might not be initialized!
    if (doc.isNull() &&
            parseError.error != QJsonParseError::NoError) {
        qCWarning(kLogger)
                << "Failed to parse JSON data:"
                << parseError.errorString()
                << "at offset"
                << parseError.offset;
        return std::make_pair(doc, parseError.error);
    }
    return std::make_pair(doc, QJsonParseError::NoError);
}

std::pair<QJsonObject, QJsonParseError::ParseError> parseObject(
        const QByteArray& jsonData) {
    const auto [jsonDoc, parseError] = parseDocument(jsonData);
    if (jsonDoc.isNull()) {
        return std::make_pair(QJsonObject{}, parseError);
    }
    if (!jsonDoc.isObject()) {
        qCWarning(kLogger)
                << "JSON document does not contain an object:"
                << jsonDoc;
        return std::make_pair(QJsonObject{}, QJsonParseError::MissingObject);
    }
    return std::make_pair(jsonDoc.object(), parseError);
}

std::pair<QJsonArray, QJsonParseError::ParseError> parseArray(
        const QByteArray& jsonData) {
    const auto [jsonDoc, parseError] = parseDocument(jsonData);
    if (jsonDoc.isNull()) {
        return std::make_pair(QJsonArray{}, parseError);
    }
    if (!jsonDoc.isArray()) {
        qCWarning(kLogger)
                << "JSON document does not contain an array"
                << jsonDoc;
        return std::make_pair(QJsonArray{}, QJsonParseError::IllegalValue);
    }
    return std::make_pair(jsonDoc.array(), parseError);
}

} // namespace json

} // namespace mixxx
