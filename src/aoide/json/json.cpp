#include "aoide/json/json.h"

#include <QJsonDocument>
#include <QRegularExpression>
#include <QTextStream>
#include <QTimeZone>
#include <QVariant>

#include "util/assert.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("aoide JSON");

const QRegularExpression kRegexpWhitespace("\\s+");

constexpr QByteArray::Base64Options kBase64Options =
        QByteArray::Base64UrlEncoding |
        QByteArray::OmitTrailingEquals;

} // anonymous namespace

namespace aoide {

namespace json {

QByteArray encodeBase64(const QByteArray& bytes) {
    return bytes.toBase64(kBase64Options);
}

QByteArray decodeBase64(const QByteArray& base64) {
    return QByteArray::fromBase64(base64, kBase64Options);
}

void Object::putOptional(QLatin1String key, const QString& value) {
    if (value.isNull()) {
        m_jsonObject.remove(key);
    } else {
        m_jsonObject.insert(key, value);
    }
}

void Object::putOptional(QLatin1String key, const QLatin1String& value) {
    if (value.isNull()) {
        m_jsonObject.remove(key);
    } else {
        m_jsonObject.insert(key, value);
    }
}

void Object::putOptional(QLatin1String key, std::optional<double> value) {
    if (value) {
        m_jsonObject.insert(key, *value);
    } else {
        m_jsonObject.remove(key);
    }
}

std::optional<double> Object::getOptionalDouble(
        QLatin1String key) const {
    const auto value = m_jsonObject.value(key);
    if (value.isDouble()) {
        return std::make_optional(value.toDouble());
    } else {
        return std::nullopt;
    }
}

void Object::putOptionalNonEmpty(QLatin1String key, QJsonValue value) {
    if (value.isUndefined() || value.isNull() ||
            (value.isObject() && value.toObject().isEmpty()) ||
            (value.isArray() && value.toArray().isEmpty()) ||
            (value.isString() && value.toString().isEmpty())) {
        m_jsonObject.remove(key);
    } else {
        m_jsonObject.insert(key, std::move(value));
    }
}

void Object::putOptionalNonEmpty(QLatin1String key, QJsonArray array) {
    if (array.isEmpty()) {
        m_jsonObject.remove(key);
    } else {
        m_jsonObject.insert(key, std::move(array));
    }
}

void Object::putOptionalNonEmpty(QLatin1String key, QJsonObject object) {
    if (object.isEmpty()) {
        m_jsonObject.remove(key);
    } else {
        m_jsonObject.insert(key, std::move(object));
    }
}

void Object::putOptionalNonEmpty(QLatin1String key, const QString& value) {
    if (value.isEmpty()) {
        m_jsonObject.remove(key);
    } else {
        m_jsonObject.insert(key, value);
    }
}

void Object::putOptionalNonEmpty(QLatin1String key, const QLatin1String& value) {
    if (value.isEmpty()) {
        m_jsonObject.remove(key);
    } else {
        m_jsonObject.insert(key, value);
    }
}

void Object::putOptionalNonEmpty(QLatin1String key, const QVariant& value) {
    auto jsonValue = QJsonValue::fromVariant(value);
    DEBUG_ASSERT(!jsonValue.isUndefined());
    if (jsonValue.isNull() ||
            (jsonValue.isString() && jsonValue.toString().isEmpty())) {
        m_jsonObject.remove(key);
    } else {
        m_jsonObject.insert(key, std::move(jsonValue));
    }
}

QString Object::toPrettyString() const {
    return QString::fromUtf8(QJsonDocument(m_jsonObject).toJson(QJsonDocument::Indented));
}

QJsonValue exportDateTimeOrYear(const QString& value) {
    if (value.isNull()) {
        return QJsonValue::Null;
    }
    // To upper: 't' -> 'T', 'z' -> 'Z'
    const auto trimmed = value.toUpper().trimmed();
    auto compact = trimmed;
    compact.remove(kRegexpWhitespace);
    if (compact.isEmpty()) {
        return QJsonValue{}; // undefined
    }
    if (compact.contains('T')) {
        // Full ISO 8601 / RFC 3339 time stamp
        auto datetime = QDateTime::fromString(compact, Qt::ISODateWithMs);
        if (datetime.isValid()) {
            return exportDateTime(datetime);
        }
        // Try to append missing seconds
        datetime = QDateTime::fromString(compact + ":00", Qt::ISODate);
        if (datetime.isValid()) {
            return exportDateTime(datetime);
        }
    } else if (trimmed.contains(':')) {
        auto datetime = QDateTime::fromString(trimmed, Qt::RFC2822Date);
        if (datetime.isValid()) {
            return exportDateTime(datetime);
        }
        // Non-standard date/time time stamp
        datetime = QDateTime::fromString(trimmed, "yyyy-M-d h:m:s");
        if (datetime.isValid()) {
            return exportDateTime(datetime);
        }
        // Try to parse without seconds
        datetime = QDateTime::fromString(trimmed, "yyyy-M-d h:m");
        if (datetime.isValid()) {
            return exportDateTime(datetime);
        }
    } else {
        // Simple date with both month and day optional
        // Try to parse year + month + day
        auto date = QDate::fromString(compact, Qt::ISODate);
        if (date.isValid()) {
            return QJsonValue(date.year() * 10000 + date.month() * 100 + date.day());
        }
        date = QDate::fromString(trimmed, Qt::RFC2822Date);
        if (date.isValid()) {
            return QJsonValue(date.year() * 10000 + date.month() * 100 + date.day());
        }
        // Try to parse simple date: year + month + day
        date = QDate::fromString(compact, "yyyy-M-d");
        if (date.isValid()) {
            return QJsonValue(date.year() * 10000 + date.month() * 100 + date.day());
        }
        // Try to parse incomplete date: year + month (without day)
        date = QDate::fromString(compact, "yyyy-M");
        if (date.isValid()) {
            return QJsonValue(date.year() * 10000 + date.month() * 100 /*day = 0*/);
        }
        // Try to parse a single row of digits
        date = QDate::fromString(compact, "yyyyMMdd");
        if (date.isValid()) {
            return QJsonValue(date.year() * 10000 + date.month() * 100 + date.day());
        }
        date = QDate::fromString(compact, "yyyy");
        if (date.isValid()) {
            return QJsonValue(date.year());
        }
    }
    // TODO: Add any missing cases that are reported. Don't forget
    // to extend the corresponding unit test to prevent regressions!
    kLogger.warning()
            << "Failed to parse date/time from string"
            << value;
    return QJsonValue{};
}

std::optional<QString> importDateTimeOrYear(const QJsonValue& value) {
    if (value.isUndefined()) {
        return std::nullopt;
    }
    if (value.isNull()) {
        return QString();
    }
    if (value.isString()) {
        return value.toString();
    }
    if (value.type() != QJsonValue::Double) {
        return std::nullopt;
    }
    auto ymd = value.toInt(0);
    if (ymd < 10000) {
        return QString();
    }
    int d = ymd % 100;
    ymd /= 100;
    int m = ymd % 100;
    ymd /= 100;
    auto year = QString("%1").arg(ymd, 4, 10, QLatin1Char('0'));
    QString month;
    if (m > 0 || d > 0) {
        month = QString("-%1").arg(m, 2, 10, QLatin1Char('0'));
    }
    QString day;
    if (d > 0) {
        day = QString("-%1").arg(d, 2, 10, QLatin1Char('0'));
    }
    return QString("%1%2%3").arg(year, month, day);
}

namespace {

QDateTime normalizeDateTimeBeforeExport(QDateTime value) {
    if (value.isNull()) {
        return value;
    }
    DEBUG_ASSERT(value.isValid());
    if (!value.timeZone().isValid()) {
        // No time zone -> assume UTC
        value.setTimeZone(QTimeZone::utc());
    } else {
        // Convert time zone to offset from UTC
        value = value.toOffsetFromUtc(value.offsetFromUtc());
    }
    DEBUG_ASSERT(value.timeZone().isValid());
    DEBUG_ASSERT(value.timeSpec() == Qt::UTC ||
            value.timeSpec() == Qt::OffsetFromUTC);
    return value;
}

} // anonymous namespace

QString exportDateTimeString(const QDateTime& value) {
    if (!value.isValid()) {
        return QString();
    }
    const auto normalized = normalizeDateTimeBeforeExport(value);
    DEBUG_ASSERT(normalized.timeZone().isValid());
    DEBUG_ASSERT(normalized.timeSpec() == Qt::UTC ||
            normalized.timeSpec() == Qt::OffsetFromUTC);
    Qt::DateFormat fmt;
    if (normalized.time().msec() == 0) {
        // Omit milliseconds if zero
        fmt = Qt::ISODate;
    } else {
        // Use full precision
        fmt = Qt::ISODateWithMs;
    }
    return normalized.toString(fmt);
}

QJsonValue exportDateTime(const QDateTime& value) {
    const auto str = exportDateTimeString(value);
    if (str.isNull()) {
        return QJsonValue{QJsonValue::Null};
    }
    DEBUG_ASSERT(!str.isEmpty());
    return QJsonValue{exportDateTimeString(value)};
}

std::optional<QDateTime> importDateTime(const QJsonValue& value) {
    if (value.isUndefined() || !value.isString()) {
        return std::nullopt;
    }
    if (value.isNull()) {
        return QDateTime();
    }
    return QDateTime::fromString(value.toString(), Qt::ISODateWithMs);
}

QJsonValue exportDateTimeTicks(const QDateTime& value) {
    if (!value.isValid()) {
        return QJsonValue();
    }
    const auto normalized = normalizeDateTimeBeforeExport(value);
    DEBUG_ASSERT(normalized.timeZone().isValid());
    DEBUG_ASSERT(normalized.timeSpec() == Qt::UTC ||
            normalized.timeSpec() == Qt::OffsetFromUTC);
    qint64 msecs = normalized.toMSecsSinceEpoch();
    return msecs * 1000;
}

QStringList arrayToStringList(const QJsonArray& jsonArray) {
    QStringList result;
    result.reserve(jsonArray.size());
    for (auto&& jsonValue : qAsConst(jsonArray)) {
        result += jsonValue.toString();
    }
    return result;
}

} // namespace json

} // namespace aoide
