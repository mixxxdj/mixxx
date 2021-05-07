#pragma once

#include <QMimeType>

#include "aoide/json/entity.h"
#include "aoide/json/marker.h"
#include "aoide/json/tag.h"
#include "library/starrating.h"
#include "track/bpm.h"
#include "track/replaygain.h"
#include "util/color/rgbcolor.h"
#include "util/encodedurl.h"

namespace mixxx {

class TrackInfo;

}

namespace aoide {

namespace json {

class AudioContent : public Object {
  public:
    explicit AudioContent(QJsonObject jsonObject = QJsonObject())
            : Object(std::move(jsonObject)) {
    }

    mixxx::Duration duration(
            mixxx::Duration defaultValue = mixxx::Duration()) const;
    void setDuration(
            mixxx::Duration newValue);

    mixxx::audio::ChannelCount channelCount(
            mixxx::audio::ChannelCount defaultValue = mixxx::audio::ChannelCount()) const;
    void setChannelCount(
            mixxx::audio::ChannelCount newValue);

    mixxx::audio::SampleRate sampleRate(
            mixxx::audio::SampleRate defaultValue = mixxx::audio::SampleRate()) const;
    void setSampleRate(
            mixxx::audio::SampleRate newValue);

    mixxx::audio::Bitrate bitrate(
            mixxx::audio::Bitrate defaultValue = mixxx::audio::Bitrate()) const;
    void setBitrate(
            mixxx::audio::Bitrate newValue);

    std::optional<double> loudnessLufs() const;
    void setLoudnessLufs(
            std::optional<double> loudnessLufs = std::nullopt);

    mixxx::ReplayGain replayGain() const;
    void setReplayGain(
            mixxx::ReplayGain replayGain = mixxx::ReplayGain());

    QString encoder() const;
    void setEncoder(const QString& encoder);
};

class ArtworkImage : public Object {
  public:
    explicit ArtworkImage(QJsonObject jsonObject = QJsonObject())
            : Object(std::move(jsonObject)) {
    }
    ~ArtworkImage() override = default;

    std::optional<int> apicType() const;
    void setApicType(std::optional<int> apicType);

    QString mediaType() const;
    void setMediaType(QString mediaType);

    QByteArray digest() const;
    void setDigest(const QByteArray& digest);

    QSize size() const;
    void setSize(const QSize& uri);

    QImage thumbnail() const;
    void setThumbnail(
            const QImage& thumbnail = QImage{});
    static QImage generateThumbnailFromRgbColor(
            mixxx::RgbColor::optional_t colorRgb);
};

class Artwork : public Object {
  public:
    explicit Artwork(QJsonObject jsonObject = QJsonObject())
            : Object(std::move(jsonObject)) {
    }
    ~Artwork() override = default;

    QString source() const;
    void setSource(const QString& source);

    mixxx::EncodedUrl uri() const;
    void setUri(const mixxx::EncodedUrl& uri);

    ArtworkImage image() const;
    void setImage(ArtworkImage image);
};

class MediaSource : public Object {
  public:
    explicit MediaSource(QJsonObject jsonObject = QJsonObject())
            : Object(std::move(jsonObject)) {
    }
    ~MediaSource() override = default;

    std::optional<QDateTime> collectedAt() const;
    void setCollectedAt(const QDateTime& collectedAt = QDateTime());

    std::optional<QDateTime> synchronizedAt() const;
    void setSynchronizedAt(const QDateTime& synchronizedAt);

    QString path() const;
    void setPath(const QString& path);

    mixxx::EncodedUrl pathUrl() const;
    void setPathUrl(const mixxx::EncodedUrl& pathUrl);

    QString contentTypeName() const;
    void setContentType(QMimeType contentType);

    AudioContent audioContent() const;
    void setAudioContent(AudioContent audioContent = AudioContent());

    Artwork artwork() const;
    void setArtwork(Artwork artwork = Artwork());
};

class Title {
  public:
    static constexpr int kKindInvalid = -1;
    static constexpr int kKindMain = 0;
    static constexpr int kKindSub = 1;
    static constexpr int kKindSorting = 2;
    static constexpr int kKindWork = 3;
    static constexpr int kKindMovement = 4;
    static constexpr int kKindDefault = kKindMain; // default if none specified

    explicit Title(
            const QString& name = QString(),
            int kind = kKindDefault)
            : m_kind(kind),
              m_name(name) {
    }

    static std::optional<Title> fromQJsonValue(
            const QJsonValue& value);
    QJsonValue toQJsonValue() const;

    int kind() const {
        return m_kind;
    }
    void setKind(int kind) {
        m_kind = kind;
    }

    const QString& name() const {
        return m_name;
    }
    void setName(const QString& name) {
        m_name = name;
    }

  private:
    int m_kind;
    QString m_name;
};

typedef QVector<Title> TitleVector;

class Actor {
  public:
    static constexpr int kKindInvalid = -1;
    static constexpr int kKindSummary = 0;
    static constexpr int kKindPrimary = 1;
    static constexpr int kKindSecondary = 2;
    static constexpr int kKindSorting = 3;
    static constexpr int kKindDefault = kKindSummary; // default if none specified

    static constexpr int kRoleInvalid = -1;
    static constexpr int kRoleArtist = 0;
    static constexpr int kRoleComposer = 2;
    static constexpr int kRoleConductor = 3;
    static constexpr int kRoleLyricist = 6;
    static constexpr int kRoleRemixer = 11;
    static constexpr int kRoleDefault = kRoleArtist; // default if none specified

    explicit Actor(
            const QString& name = QString(),
            int role = kRoleDefault)
            : m_kind(kKindDefault),
              m_name(name),
              m_role(role) {
    }

    static std::optional<Actor> fromQJsonValue(
            const QJsonValue& value);
    QJsonValue toQJsonValue() const;

    int kind() const {
        return m_kind;
    }
    void setKind(int kind) {
        m_kind = kind;
    }

    const QString& name() const {
        return m_name;
    }
    void setName(const QString& name) {
        m_name = name;
    }

    int role() const {
        return m_role;
    }
    void setRole(int role) {
        m_role = role;
    }

    const QString& roleNotes() const {
        return m_roleNotes;
    }
    void setRoleNotes(const QString& roleNotes) {
        m_roleNotes = roleNotes;
    }

  private:
    int m_kind;
    QString m_name;
    int m_role;
    QString m_roleNotes;
};

typedef QVector<Actor> ActorVector;

class TrackOrAlbumOrRelease : public Object {
  public:
    explicit TrackOrAlbumOrRelease(QJsonObject jsonObject = QJsonObject())
            : Object(std::move(jsonObject)) {
    }
    ~TrackOrAlbumOrRelease() override = default;
};

class TrackOrAlbum : public TrackOrAlbumOrRelease {
  public:
    explicit TrackOrAlbum(QJsonObject jsonObject = QJsonObject())
            : TrackOrAlbumOrRelease(std::move(jsonObject)) {
    }
    ~TrackOrAlbum() override = default;

    TitleVector titles(int kind = Title::kKindMain) const;
    TitleVector allTitles() const;
    TitleVector removeTitles(int kind);
    TitleVector clearTitles();
    void addTitles(TitleVector titles);

    ActorVector artists(
            int kind = Actor::kKindSummary) const {
        return actors(Actor::kRoleArtist, kind);
    }
    ActorVector actors(
            int role,
            int kind = Actor::kKindSummary) const;
    ActorVector allActors() const;
    ActorVector removeActors(int role);
    ActorVector clearActors();
    void addActors(const ActorVector& actors);
};

class Release : public TrackOrAlbumOrRelease {
  public:
    explicit Release(QJsonObject jsonObject = QJsonObject())
            : TrackOrAlbumOrRelease(std::move(jsonObject)) {
    }
    ~Release() override = default;

    QString releasedAt() const;
    void setReleasedAt(const QString& releasedAt = QString());

    QString releasedBy() const;
    void setReleasedBy(QString label = QString());

    QString copyright() const;
    void setCopyright(QString copyright = QString());
};

class Album : public TrackOrAlbum {
  public:
    explicit Album(QJsonObject jsonObject = QJsonObject())
            : TrackOrAlbum(std::move(jsonObject)) {
    }
    ~Album() override = default;

    static constexpr int kTypeInvalid = -1;
    static constexpr int kTypeAlbum = 0;
    static constexpr int kTypeSingle = 1;
    static constexpr int kTypeCompilation = 2;

    std::optional<int> type() const;
    void setType(std::optional<int> type = std::nullopt);
};

class PlayCounter : public Object {
  public:
    explicit PlayCounter(QJsonObject jsonObject = QJsonObject())
            : Object(std::move(jsonObject)) {
    }

    std::optional<QDateTime> lastPlayedAt() const;
    void setLastPlayedAt(const QDateTime& lastPlayedAt = QDateTime());

    int timesPlayed() const;
    void setTimesPlayed(int timesPlayed);
};

enum class MusicMetricsBitflags : int {
    None = 0,
    TempoBpmLocked = 1 << 0,
    TimeSignatureLocked = 1 << 1,
    KeySignatureLocked = 1 << 2,
    Default = None,
};

Q_DECLARE_FLAGS(MusicMetricsFlags, MusicMetricsBitflags)
Q_DECLARE_OPERATORS_FOR_FLAGS(MusicMetricsFlags)

class MusicMetrics : public Object {
  public:
    explicit MusicMetrics(QJsonObject jsonObject = QJsonObject())
            : Object(std::move(jsonObject)) {
    }
    ~MusicMetrics() override = default;

    void setFlags(MusicMetricsFlags flags);
    MusicMetricsFlags flags() const;

    void setBpm(mixxx::Bpm bpm);
    mixxx::Bpm bpm() const;
    void setBpmLocked(bool locked = true) {
        setFlags(flags().setFlag(MusicMetricsBitflags::TempoBpmLocked, locked));
    }
    bool bpmLocked() const {
        return flags().testFlag(MusicMetricsBitflags::TempoBpmLocked);
    }

    void setKey(
            mixxx::track::io::key::ChromaticKey chromaticKey);
    mixxx::track::io::key::ChromaticKey key() const;
    void setKeyLocked(bool locked = true) {
        setFlags(flags().setFlag(MusicMetricsBitflags::KeySignatureLocked, locked));
    }
    bool keyLocked() const {
        return flags().testFlag(MusicMetricsBitflags::KeySignatureLocked);
    }
};

class Track : public TrackOrAlbum {
  public:
    explicit Track(QJsonObject jsonObject = QJsonObject())
            : TrackOrAlbum(std::move(jsonObject)) {
    }

    MediaSource mediaSource() const;
    void setMediaSource(MediaSource mediaSource);

    Release release() const;
    void setRelease(Release release);

    Album album() const;
    void setAlbum(Album album);

    QString trackNumbers() const;
    QString discNumbers() const;
    void setIndexNumbers(const mixxx::TrackInfo& trackInfo);

    Tags tags() const;
    Tags removeTags();
    void setTags(Tags tags);

    mixxx::RgbColor::optional_t color() const;
    void setColor(mixxx::RgbColor::optional_t color = std::nullopt);

    MusicMetrics musicMetrics() const;
    void setMusicMetrics(MusicMetrics metrics);

    CueMarkers cueMarkers() const;
    void setCueMarkers(CueMarkers cueMarkers);

    PlayCounter playCounter() const;
    void setPlayCounter(PlayCounter playCounter = PlayCounter());
};

class TrackEntity : public Array {
  public:
    explicit TrackEntity(QJsonArray jsonArray = QJsonArray())
            : Array(std::move(jsonArray)) {
    }
    TrackEntity(
            EntityHeader header,
            Track body)
            : Array(QJsonArray{
                      header.intoQJsonValue(),
                      body.intoQJsonValue()}) {
    }

    EntityHeader header() const;

    Track body() const;
    void setBody(Track body);
};

} // namespace json

} // namespace aoide

Q_DECLARE_METATYPE(aoide::json::AudioContent);
Q_DECLARE_METATYPE(aoide::json::Artwork);
Q_DECLARE_METATYPE(aoide::json::MediaSource);
Q_DECLARE_METATYPE(aoide::json::Title);
Q_DECLARE_METATYPE(aoide::json::TitleVector);
Q_DECLARE_METATYPE(aoide::json::Actor);
Q_DECLARE_METATYPE(aoide::json::ActorVector);
Q_DECLARE_METATYPE(aoide::json::TrackOrAlbum);
Q_DECLARE_METATYPE(aoide::json::Release);
Q_DECLARE_METATYPE(aoide::json::Album);
Q_DECLARE_METATYPE(aoide::json::MusicMetrics);
Q_DECLARE_METATYPE(aoide::json::Track);
Q_DECLARE_METATYPE(aoide::json::TrackEntity);
