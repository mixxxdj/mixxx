#pragma once

#include "aoide/json/collection.h"

namespace aoide {

namespace json {

class Playlist : public Collection {
  public:
    explicit Playlist(QJsonObject jsonObject = QJsonObject())
            : Collection(std::move(jsonObject)) {
    }
    ~Playlist() override = default;

    std::optional<QDateTime> collectedAt() const;
    void setCollectedAt(const QDateTime& collectedAt = QDateTime());
};

class PlaylistEntity : public Array {
  public:
    explicit PlaylistEntity(QJsonArray jsonArray = QJsonArray())
            : Array(std::move(jsonArray)) {
    }
    PlaylistEntity(
            EntityHeader header,
            Playlist body)
            : Array(QJsonArray{
                      header.intoQJsonValue(),
                      body.intoQJsonValue()}) {
    }
    ~PlaylistEntity() override = default;

    EntityHeader header() const;

    Playlist body() const;
};

class PlaylistEntry : public Object {
  public:
    static PlaylistEntry newSeparator();
    static PlaylistEntry newTrack(QString trackUid);

    explicit PlaylistEntry(QJsonObject jsonObject = QJsonObject())
            : Object(std::move(jsonObject)) {
    }
    ~PlaylistEntry() override = default;

    std::optional<QDateTime> addedAt() const;
    void setAddedAt(QDateTime addedAt = QDateTime());

    QString trackUid() const;
};

class PlaylistWithEntries : public Playlist {
  public:
    explicit PlaylistWithEntries(QJsonObject jsonObject = QJsonObject())
            : Playlist(std::move(jsonObject)) {
    }
    ~PlaylistWithEntries() override = default;

    QJsonArray entries() const;
    void setEntries(QJsonArray entries);
};

class PlaylistWithEntriesEntity : public Array {
  public:
    explicit PlaylistWithEntriesEntity(QJsonArray jsonArray = QJsonArray())
            : Array(std::move(jsonArray)) {
    }
    PlaylistWithEntriesEntity(
            EntityHeader header,
            PlaylistWithEntries body)
            : Array(QJsonArray{
                      header.intoQJsonValue(),
                      body.intoQJsonValue()}) {
    }
    ~PlaylistWithEntriesEntity() override = default;

    EntityHeader header() const;

    PlaylistWithEntries body() const;
};

class PlaylistEntriesSummary : public Object {
  public:
    explicit PlaylistEntriesSummary(QJsonObject jsonObject = QJsonObject())
            : Object(std::move(jsonObject)) {
    }
    ~PlaylistEntriesSummary() override = default;

    // All entries
    quint64 totalCount() const;

    // Only tracks
    quint64 totalTracksCount() const;

    std::optional<QDateTime> addedAtMin() const;

    std::optional<QDateTime> addedAtMax() const;
};

class PlaylistWithEntriesSummary : public Playlist {
  public:
    explicit PlaylistWithEntriesSummary(QJsonObject jsonObject = QJsonObject())
            : Playlist(std::move(jsonObject)) {
    }
    ~PlaylistWithEntriesSummary() override = default;

    PlaylistEntriesSummary entries() const;
};

class PlaylistWithEntriesSummaryEntity : public Array {
  public:
    explicit PlaylistWithEntriesSummaryEntity(QJsonArray jsonArray = QJsonArray())
            : Array(std::move(jsonArray)) {
    }
    PlaylistWithEntriesSummaryEntity(
            EntityHeader header,
            PlaylistWithEntriesSummary body)
            : Array(QJsonArray{
                      header.intoQJsonValue(),
                      body.intoQJsonValue()}) {
    }
    ~PlaylistWithEntriesSummaryEntity() override = default;

    EntityHeader header() const;

    PlaylistWithEntriesSummary body() const;
};

} // namespace json

} // namespace aoide

Q_DECLARE_METATYPE(aoide::json::Playlist);
Q_DECLARE_METATYPE(aoide::json::PlaylistEntry);
Q_DECLARE_METATYPE(aoide::json::PlaylistWithEntries);
Q_DECLARE_METATYPE(aoide::json::PlaylistEntity);
Q_DECLARE_METATYPE(aoide::json::PlaylistWithEntriesEntity);
Q_DECLARE_METATYPE(aoide::json::PlaylistEntriesSummary);
Q_DECLARE_METATYPE(aoide::json::PlaylistWithEntriesSummary);
Q_DECLARE_METATYPE(aoide::json::PlaylistWithEntriesSummaryEntity);
