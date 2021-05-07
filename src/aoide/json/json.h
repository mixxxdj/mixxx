#pragma once

#include <QColor>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QLatin1String>
#include <QMetaType>
#include <QUuid>
#include <QVariant>

#include "util/optional.h"

///////////////////////////////////////////////////////////////////////////////
// Thin and efficient wrappers around QJsonObject with some accessors and
// only limited editing functionality.
//
// Signal types should to be placed in the global namespace! Handling of
// meta types in namespaces is hardly documented and may not work as expected.
///////////////////////////////////////////////////////////////////////////////

namespace aoide {

namespace json {

// ISO 8601 / RFC 3339
QString exportDateTimeString(const QDateTime& value);
QJsonValue exportDateTime(const QDateTime& value);
inline QJsonValue exportCurrentDateTime() {
    return exportDateTime(QDateTime::currentDateTime());
}
std::optional<QDateTime> importDateTime(const QJsonValue& value);

// Date/time or YYYYMMDD integer
QJsonValue exportDateTimeOrYear(const QString& value);
std::optional<QString> importDateTimeOrYear(const QJsonValue& value);

QStringList arrayToStringList(const QJsonArray& jsonArray);

QByteArray encodeBase64(const QByteArray& bytes);
QByteArray decodeBase64(const QByteArray& base64);
inline QByteArray decodeBase64(const QLatin1String& base64) {
    return decodeBase64(QByteArray::fromRawData(base64.data(), base64.size()));
}
inline QByteArray decodeBase64(const QString& base64) {
    return decodeBase64(base64.toLatin1());
}

class Value {
  public:
    virtual ~Value() = default;

    virtual QJsonValue intoQJsonValue() = 0;
};

class Object : public Value {
  public:
    explicit Object(QJsonObject jsonObject = QJsonObject())
            : m_jsonObject(std::move(jsonObject)) {
    }
    ~Object() override = default;

    bool isEmpty() const {
        return m_jsonObject.isEmpty();
    }

    operator const QJsonObject&() const {
        return m_jsonObject;
    }

    QJsonValue intoQJsonValue() override {
        return std::move(m_jsonObject);
    }

    QString toPrettyString() const;

  protected:
    void putOptional(QLatin1String key, const QString& value);
    void putOptional(QLatin1String key, const QLatin1String& value);
    void putOptional(QLatin1String key, std::optional<double> value);
    void putOptionalNonEmpty(QLatin1String key, const QString& value);
    void putOptionalNonEmpty(QLatin1String key, const QLatin1String& value);
    void putOptionalNonEmpty(QLatin1String key, QJsonObject object);
    void putOptionalNonEmpty(QLatin1String key, QJsonArray array);
    void putOptionalNonEmpty(QLatin1String key, QJsonValue value);
    void putOptionalNonEmpty(QLatin1String key, const QVariant& value);

    std::optional<double> getOptionalDouble(
            QLatin1String key) const;

    QJsonObject m_jsonObject;
};

class Array : public Value {
  public:
    explicit Array(QJsonArray jsonArray = QJsonArray())
            : m_jsonArray(std::move(jsonArray)) {
    }
    ~Array() override = default;

    bool isEmpty() const {
        return m_jsonArray.isEmpty();
    }

    void shrink(int shrinkedSize) {
        while (m_jsonArray.size() > shrinkedSize) {
            m_jsonArray.removeLast();
        }
    }

    operator const QJsonArray&() const {
        return m_jsonArray;
    }

    QJsonValue intoQJsonValue() override {
        return std::move(m_jsonArray);
    }

  protected:
    QJsonArray m_jsonArray;
};

} // namespace json

} // namespace aoide

Q_DECLARE_METATYPE(aoide::json::Object);
Q_DECLARE_METATYPE(aoide::json::Array);
