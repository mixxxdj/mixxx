#pragma once

#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QLoggingCategory>
#include <utility>

namespace mixxx {

namespace json {

Q_DECLARE_LOGGING_CATEGORY(kLogger)

std::pair<QJsonDocument, QJsonParseError::ParseError> parseDocument(
        const QByteArray& jsonData);

std::pair<QJsonObject, QJsonParseError::ParseError> parseObject(
        const QByteArray& jsonData);

std::pair<QJsonArray, QJsonParseError::ParseError> parseArray(
        const QByteArray& jsonData);

} // namespace json

} // namespace mixxx
