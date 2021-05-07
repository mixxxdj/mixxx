#pragma once

#include "aoide/json/entity.h"
#include "util/color/rgbcolor.h"
#include "util/encodedurl.h"
#include "util/fileinfo.h"

namespace aoide {

namespace json {

constexpr int kSourcePathKindUri = 0;
constexpr int kSourcePathKindUrl = 1;
constexpr int kSourcePathKindFileUrl = 2;
constexpr int kSourcePathKindVirtualFilePath = 3;

class SourcePathConfig : public Object {
  public:
    static SourcePathConfig forLocalFiles(
            const std::optional<mixxx::EncodedUrl>& rootUrl = std::nullopt);

    explicit SourcePathConfig(
            QJsonObject jsonObject = QJsonObject());
    ~SourcePathConfig() override = default;

    int pathKind() const;
    void setPathKind(
            int pathKind);

    std::optional<mixxx::EncodedUrl> rootUrl() const;
    void setRootUrl(
            const std::optional<mixxx::EncodedUrl>& rootUrl = std::nullopt);

    // The mapping of file URLs to virtual file paths and vice
    // versa is performed by aoide and not by Mixxx. These
    // functions are currently unused, but might be needed
    // again.
    QString virtualFilePathFromFileInfo(
            const mixxx::FileInfo& fileInfo) const;
    mixxx::FileInfo fileInfoFromVirtualFilePath(
            const QString& path) const;

    friend bool operator==(
            const SourcePathConfig& lhs,
            const SourcePathConfig& rhs) {
        return lhs.m_jsonObject == rhs.m_jsonObject;
    }
};

inline bool operator!=(
        const SourcePathConfig& lhs,
        const SourcePathConfig& rhs) {
    return !(lhs == rhs);
}

class MediaSourceConfig : public Object {
  public:
    static MediaSourceConfig forLocalFiles(
            const std::optional<mixxx::EncodedUrl>& rootUrl = std::nullopt);

    explicit MediaSourceConfig(
            QJsonObject jsonObject = QJsonObject());
    ~MediaSourceConfig() override = default;

    SourcePathConfig sourcePath() const;
    void setSourcePath(
            SourcePathConfig sourcePath);

    friend bool operator==(
            const MediaSourceConfig& lhs,
            const MediaSourceConfig& rhs) {
        return lhs.m_jsonObject == rhs.m_jsonObject;
    }
};

inline bool operator!=(
        const MediaSourceConfig& lhs,
        const MediaSourceConfig& rhs) {
    return !(lhs == rhs);
}

class Collection : public Object {
  public:
    explicit Collection(
            QJsonObject jsonObject = QJsonObject());
    ~Collection() override = default;

    QString title() const;
    void setTitle(const QString& title = QString());

    QString kind() const;
    void setKind(
            QString kind = QString());

    mixxx::RgbColor::optional_t color() const;
    void setColor(
            mixxx::RgbColor::optional_t color = mixxx::RgbColor::optional_t());

    QString notes() const;
    void setNotes(
            const QString& notes = QString());

    MediaSourceConfig mediaSourceConfig() const;
    void setMediaSourceConfig(
            MediaSourceConfig mediaSourceConfig);
};

class CollectionEntity : public Array {
  public:
    explicit CollectionEntity(
            QJsonArray jsonArray = QJsonArray())
            : Array(std::move(jsonArray)) {
    }
    CollectionEntity(
            EntityHeader header,
            Collection body)
            : Array(QJsonArray{
                      header.intoQJsonValue(),
                      body.intoQJsonValue()}) {
    }
    ~CollectionEntity() override = default;

    EntityHeader header() const;

    Collection body() const;
};

} // namespace json

} // namespace aoide

Q_DECLARE_METATYPE(aoide::json::Collection);
Q_DECLARE_METATYPE(aoide::json::CollectionEntity);
