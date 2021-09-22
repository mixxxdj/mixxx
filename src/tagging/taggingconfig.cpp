#include "tagging/taggingconfig.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "util/logger.h"

namespace mixxx {

namespace {

const Logger kLogger("tagging.config");

} // anonymous namespace

//static
std::optional<FacetedTagConfig> FacetedTagConfig::fromJsonObject(
        const QJsonObject& jsonObject,
        bool strict) {
    FacetedTagConfig res;
    {
        const auto jsonValue = jsonObject.value(QLatin1String("displayName"));
        if (jsonValue.isString()) {
            auto displayName = jsonValue.toString();
            if (!isValidDisplayName(displayName)) {
                kLogger.warning()
                        << "Invalid display name"
                        << displayName;
                VERIFY_OR_DEBUG_ASSERT(!strict) {
                    return std::nullopt;
                }
                displayName = clampDisplayName(displayName);
            }
            DEBUG_ASSERT(isValidDisplayName(displayName));
            res.setDisplayName(displayName);
        } else {
            if (!(jsonValue.isUndefined() || jsonValue.isNull())) {
                kLogger.warning()
                        << "Invalid JSON value"
                        << jsonValue;
                VERIFY_OR_DEBUG_ASSERT(!strict) {
                    return std::nullopt;
                }
            }
        }
    }
    {
        const auto jsonValue = jsonObject.value(QLatin1String("tagMapping"));
        if (jsonValue.isObject()) {
            res.setTagMapping(
                    TagMappingConfig::fromJsonObject(
                            jsonValue.toObject()));
        } else {
            if (!(jsonValue.isUndefined() || jsonValue.isNull())) {
                kLogger.warning()
                        << "Invalid JSON value"
                        << jsonValue;
                VERIFY_OR_DEBUG_ASSERT(!strict) {
                    return std::nullopt;
                }
            }
        }
    }
    VERIFY_OR_DEBUG_ASSERT(res.isValid()) {
        kLogger.warning()
                << "Invalid"
                << res
                << "from JSON";
        return std::nullopt;
    }
    return res;
}

QJsonObject FacetedTagConfig::toJsonObject() const {
    QJsonObject jsonObject;
    if (hasDisplayName()) {
        jsonObject.insert(
                QLatin1String("displayName"),
                getDisplayName());
    }
    if (hasTagMapping()) {
        auto tagMapping = getTagMapping()->toJsonObject();
        DEBUG_ASSERT(!tagMapping.isEmpty());
        jsonObject.insert(
                QLatin1String("tagMapping"),
                tagMapping);
    }
    return jsonObject;
}

bool operator==(
        const FacetedTagConfig& lhs,
        const FacetedTagConfig& rhs) {
    return lhs.getDisplayName() == rhs.getDisplayName() &&
            lhs.getTagMapping() == rhs.getTagMapping();
}

QDebug operator<<(
        QDebug dbg,
        const FacetedTagConfig& arg) {
    dbg << "FacetedTagConfig{";
    arg.dbgDisplayName(dbg);
    arg.dbgTagMapping(dbg);
    dbg << '}';
    return dbg;
}

void TaggingConfig::setFacetDisplayName(
        const TagFacetId& facetId,
        const QString& displayName) {
    DEBUG_ASSERT(FacetedTagConfig::isValidDisplayName(displayName));
    refCustomFacets()[facetId].setDisplayName(displayName);
}

const TagMappingConfig* TaggingConfig::findFacetTagMapping(
        const TagFacetId& facetId) const {
    const auto i = getCustomFacets().find(facetId);
    if (i != getCustomFacets().end() && i->getTagMapping()) {
        // Return a pointer to the referenced map entry value
        // after unwrapping from std::optional.
        const auto& ref = *(i->getTagMapping());
        return &ref;
    }
    return nullptr;
}

//static
std::optional<TaggingConfig> TaggingConfig::fromJsonObject(
        const QJsonObject& jsonObject,
        bool strict) {
    TaggingConfig res;
    {
        const auto jsonValue = jsonObject.value(QLatin1String("facets"));
        if (jsonValue.isObject()) {
            const auto jsonObject = jsonValue.toObject();
            const auto fromJsonMode = strict
                    ? Facets::FromJsonMode::Strict
                    : Facets::FromJsonMode::Lenient;
            auto optFacets = Facets::fromJsonObject(jsonObject, fromJsonMode);
            if (optFacets) {
                DEBUG_ASSERT(optFacets->validate());
                res.setFacets(std::move(*optFacets));
            } else {
                kLogger.warning()
                        << "Invalid JSON object"
                        << jsonObject;
                VERIFY_OR_DEBUG_ASSERT(!strict) {
                    return std::nullopt;
                }
            }
        } else {
            if (!(jsonValue.isUndefined() || jsonValue.isNull())) {
                VERIFY_OR_DEBUG_ASSERT(!strict) {
                    kLogger.warning()
                            << "Invalid JSON value"
                            << jsonValue;
                    return std::nullopt;
                }
                kLogger.warning()
                        << "Skipping invalid JSON value"
                        << jsonValue;
            }
        }
    }
    {
        const auto jsonValue = jsonObject.value(QLatin1String("customFacets"));
        if (jsonValue.isObject()) {
            const auto jsonObject = jsonValue.toObject();
            for (auto i = jsonObject.begin();
                    i != jsonObject.end();
                    ++i) {
                const auto facetIdValue = TagFacetId::filterEmptyValue(i.key());
                if (facetIdValue.isEmpty() ||
                        !TagFacetId::isValidValue(facetIdValue)) {
                    VERIFY_OR_DEBUG_ASSERT(!strict) {
                        kLogger.warning()
                                << "Invalid facet"
                                << facetIdValue
                                << "from JSON";
                        return std::nullopt;
                    }
                    kLogger.warning()
                            << "Skipping invalid facet"
                            << facetIdValue
                            << "from JSON";
                    continue;
                }
                if (i.value().isObject()) {
                    auto optFacetConfig = FacetedTagConfig::fromJsonObject(
                            i.value().toObject(),
                            strict);
                    if (!optFacetConfig) {
                        VERIFY_OR_DEBUG_ASSERT(!strict) {
                            kLogger.warning()
                                    << "Invalid JSON object"
                                    << jsonObject
                                    << "for facet"
                                    << facetIdValue;
                            return std::nullopt;
                        }
                        kLogger.warning()
                                << "Skipping invalid JSON object"
                                << jsonObject
                                << "for facet"
                                << facetIdValue;
                        continue;
                    }
                    DEBUG_ASSERT(optFacetConfig->isValid());
                    res.refCustomFacets().insert(
                            TagFacetId(facetIdValue),
                            std::move(*optFacetConfig));
                } else {
                    if (!(i.value().isUndefined() || i.value().isNull())) {
                        VERIFY_OR_DEBUG_ASSERT(!strict) {
                            kLogger.warning()
                                    << "Invalid JSON value"
                                    << i.value()
                                    << "for facet"
                                    << facetIdValue;
                            return std::nullopt;
                        }
                        kLogger.warning()
                                << "Skipping invalid JSON value"
                                << i.value()
                                << "for facet"
                                << facetIdValue;
                        continue;
                    }
                }
            }
        } else {
            if (!jsonValue.isUndefined()) {
                VERIFY_OR_DEBUG_ASSERT(!strict) {
                    kLogger.warning()
                            << "Invalid JSON value"
                            << jsonValue;
                    return std::nullopt;
                }
                kLogger.warning()
                        << "Skipping invalid JSON value"
                        << jsonValue;
            }
        }
    }
    return res;
}

QJsonObject TaggingConfig::toJsonObject() const {
    QJsonObject jsonObject;
    const auto facets = getFacets().toJsonObject(
            Facets::ToJsonMode::Plain);
    if (!facets.isEmpty()) {
        jsonObject.insert(
                QLatin1String("facets"),
                facets);
    }
    {
        QJsonObject customFacets;
        for (auto i = getCustomFacets().begin();
                i != getCustomFacets().end();
                ++i) {
            const auto facetConfig = i.value().toJsonObject();
            if (!facetConfig.isEmpty()) {
                customFacets.insert(i.key(), facetConfig);
            }
        }
        if (!customFacets.isEmpty()) {
            jsonObject.insert(
                    QLatin1String("customFacets"),
                    customFacets);
        }
    }
    return jsonObject;
}

//static
std::optional<TaggingConfig> TaggingConfig::parseJsonData(
        const QByteArray& jsonData,
        bool strict) {
    if (jsonData.isEmpty()) {
        kLogger.warning()
                << "Failed to parse empty JSON data";
        return std::nullopt;
    }
    QJsonParseError parseError;
    const auto jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);
    // QJsonDocument::fromJson() returns a non-null document
    // if parsing succeeds and otherwise null on error. The
    // parse error must only be evaluated if the returned
    // document is null!
    if (jsonDoc.isNull() &&
            parseError.error != QJsonParseError::NoError) {
        kLogger.warning()
                << "Failed to parse JSON data:"
                << parseError.errorString()
                << "at offset"
                << parseError.offset;
        return std::nullopt;
    }
    if (!jsonDoc.isObject()) {
        kLogger.warning()
                << "Invalid JSON document"
                << jsonDoc;
        return std::nullopt;
    }
    const auto jsonObject = jsonDoc.object();
    auto facetsConfig = TaggingConfig::fromJsonObject(jsonObject, strict);
    if (!facetsConfig) {
        kLogger.warning()
                << "Invalid JSON object"
                << jsonObject;
        return std::nullopt;
    }
    return facetsConfig;
}

QByteArray TaggingConfig::dumpJsonData() const {
    return QJsonDocument(toJsonObject()).toJson(QJsonDocument::Compact);
}

//static
std::optional<TaggingConfig> TaggingConfig::loadFromFile(
        const QString& filePath,
        bool strict) {
    kLogger.debug()
            << "Loading from file"
            << filePath;
    QByteArray jsonData;
    {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            kLogger.warning()
                    << "Failed to open file"
                    << filePath
                    << "for reading";
            return std::nullopt;
        }
        jsonData = file.readAll();
        // File is closed implicitly
    }
    return TaggingConfig::parseJsonData(jsonData, strict);
}

bool TaggingConfig::saveIntoFile(
        const QString& filePath) const {
    kLogger.debug()
            << "Saving into file"
            << filePath;
    QFile file(filePath);
    if (!file.open(
                QIODevice::WriteOnly |
                QIODevice::Truncate)) {
        kLogger.warning()
                << "Failed to open file"
                << filePath
                << "for writing";
        return false;
    }
    const QByteArray jsonData =
            QJsonDocument(toJsonObject())
                    .toJson(QJsonDocument::Indented);
    const auto bytesWritten = file.write(jsonData);
    DEBUG_ASSERT(bytesWritten <= jsonData.size());
    if (bytesWritten < jsonData.size()) {
        kLogger.warning()
                << "Failed to write into file"
                << filePath;
        return false;
    }
    // File is closed implicitly
    return true;
}

bool operator==(
        const TaggingConfig& lhs,
        const TaggingConfig& rhs) {
    return lhs.getFacets() == rhs.getFacets() &&
            lhs.getCustomFacets() == rhs.getCustomFacets();
}

QDebug operator<<(
        QDebug dbg,
        const TaggingConfig& arg) {
    dbg << "TaggingConfig{";
    arg.dbgFacets(dbg);
    arg.dbgCustomFacets(dbg);
    dbg << '}';
    return dbg;
}

} // namespace mixxx
