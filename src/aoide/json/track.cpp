#include "aoide/json/track.h"

#include "analyzer/analyzerebur128.h"
#include "track/trackinfo.h"
#include "util/assert.h"
#include "util/fpclassify.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("aoide Track");

QJsonValue optionalPositiveIntJsonValue(const QString& value) {
    bool valid = false;
    int intValue = value.toInt(&valid);
    if (valid && (intValue > 0)) {
        return QJsonValue(intValue);
    } else {
        return QJsonValue();
    }
}

constexpr int kArtworkDigestSize = 32;

constexpr int kArtworkThumbnailWidth = 4;
constexpr int kArtworkThumbnailHeight = 4;

inline QImage createArtworkThumbnailImage() {
    return QImage(kArtworkThumbnailWidth, kArtworkThumbnailHeight, QImage::Format_RGB888);
}

} // anonymous namespace

namespace aoide {

namespace json {

mixxx::Duration AudioContent::duration(
        mixxx::Duration defaultValue) const {
    const auto durationMillis =
            m_jsonObject.value(QLatin1String("durationMs")).toDouble(defaultValue.toDoubleMillis());
    return mixxx::Duration::fromMillis(
            static_cast<quint64>(std::round(durationMillis)));
}

void AudioContent::setDuration(
        mixxx::Duration newValue) {
    const auto durationMillis =
            std::round(newValue.toDoubleMillis());
    DEBUG_ASSERT(!util_isnan(durationMillis));
    VERIFY_OR_DEBUG_ASSERT(durationMillis >= 0.0) {
        m_jsonObject.remove(QLatin1String("durationMs"));
        return;
    }
    m_jsonObject.insert(QLatin1String("durationMs"), durationMillis);
}

mixxx::audio::ChannelCount AudioContent::channelCount(
        mixxx::audio::ChannelCount defaultValue) const {
    auto jsonValue = m_jsonObject.value(QLatin1String("channels"));
    return mixxx::audio::ChannelCount(jsonValue.toInt(defaultValue));
}

void AudioContent::setChannelCount(
        mixxx::audio::ChannelCount newValue) {
    if (newValue.isValid()) {
        m_jsonObject.insert(QLatin1String("channels"), static_cast<int>(newValue));
    } else {
        m_jsonObject.remove(QLatin1String("channels"));
    }
}

mixxx::audio::SampleRate AudioContent::sampleRate(
        mixxx::audio::SampleRate defaultValue) const {
    const auto sampleRateHz =
            m_jsonObject.value(QLatin1String("samplerateHz")).toInt(defaultValue);
    return mixxx::audio::SampleRate(sampleRateHz);
}

void AudioContent::setSampleRate(
        mixxx::audio::SampleRate newValue) {
    if (newValue.isValid()) {
        m_jsonObject.insert(QLatin1String("sampleRateHz"), static_cast<int>(newValue));
    } else {
        m_jsonObject.remove(QLatin1String("sampleRateHz"));
    }
}

mixxx::audio::Bitrate AudioContent::bitrate(
        mixxx::audio::Bitrate defaultValue) const {
    const auto defaultBps = defaultValue * 1000; // kbps -> bps
    const auto bps = m_jsonObject.value(QLatin1String("bitrateBps")).toInt(defaultBps);
    return mixxx::audio::Bitrate(bps / 1000); // bps -> kbps
}

void AudioContent::setBitrate(
        mixxx::audio::Bitrate newValue) {
    if (newValue.isValid()) {
        const auto newBps = newValue * 1000; // kbps -> bps
        m_jsonObject.insert(QLatin1String("bitrateBps"), static_cast<int>(newBps));
    } else {
        m_jsonObject.remove(QLatin1String("bitrateBps"));
    }
}

std::optional<double> AudioContent::loudnessLufs() const {
    return getOptionalDouble(QLatin1String("loudnessLufs"));
}

void AudioContent::setLoudnessLufs(std::optional<double> loudnessLufs) {
    putOptional(QLatin1String("loudnessLufs"), loudnessLufs);
}

mixxx::ReplayGain AudioContent::replayGain() const {
    mixxx::ReplayGain replayGain;
    const auto lufs = loudnessLufs();
    if (lufs) {
        const auto referenceGainDb =
                AnalyzerEbur128::kReplayGain2ReferenceLUFS - *lufs;
        replayGain.setRatio(db2ratio(referenceGainDb));
    }
    return replayGain;
}

void AudioContent::setReplayGain(
        mixxx::ReplayGain replayGain) {
    if (replayGain.hasRatio()) {
        // Assumption: Gain has been calculated with the new EBU R128 algorithm.
        const double referenceGainDb = ratio2db(replayGain.getRatio());
        // Reconstruct the LUFS value from the relative gain
        const double ituBs1770Lufs = AnalyzerEbur128::kReplayGain2ReferenceLUFS - referenceGainDb;
        setLoudnessLufs(ituBs1770Lufs);
    } else {
        setLoudnessLufs();
    }
}

QString AudioContent::encoder() const {
    return m_jsonObject.value(QLatin1String("encoder")).toString();
}

void AudioContent::setEncoder(const QString& encoder) {
    putOptionalNonEmpty(QLatin1String("encoder"), encoder);
}

QString MediaSource::path() const {
    return m_jsonObject.value(QLatin1String("path")).toString();
}

void MediaSource::setPath(const QString& path) {
    putOptionalNonEmpty(QLatin1String("path"), path);
}

mixxx::EncodedUrl MediaSource::pathUrl() const {
    return mixxx::EncodedUrl::fromEncodedQByteArray(path().toUtf8());
}

void MediaSource::setPathUrl(const mixxx::EncodedUrl& url) {
    putOptionalNonEmpty(QLatin1String("path"), url.toQString());
}

std::optional<QDateTime> MediaSource::synchronizedAt() const {
    return importDateTime(m_jsonObject.value(QLatin1String("synchronizedAt")));
}

void MediaSource::setSynchronizedAt(const QDateTime& synchronizedAt) {
    putOptionalNonEmpty(QLatin1String("synchronizedAt"), exportDateTime(synchronizedAt));
}

QString MediaSource::contentTypeName() const {
    return m_jsonObject.value(QLatin1String("contentType")).toString();
}

void MediaSource::setContentType(QMimeType contentType) {
    putOptionalNonEmpty(QLatin1String("contentType"), contentType.name());
}

AudioContent MediaSource::audioContent() const {
    return AudioContent(m_jsonObject.value(QLatin1String("audio")).toObject());
}

void MediaSource::setAudioContent(AudioContent audioContent) {
    // Replace the entire audio content object
    m_jsonObject.insert(
            QLatin1String("audio"), audioContent.intoQJsonValue());
}

Artwork MediaSource::artwork() const {
    return Artwork(m_jsonObject.value(QLatin1String("artwork")).toObject());
}

void MediaSource::setArtwork(Artwork artwork) {
    putOptionalNonEmpty(QLatin1String("artwork"), artwork.intoQJsonValue());
}

QString Artwork::source() const {
    return m_jsonObject.value(QLatin1String("source")).toString();
}

void Artwork::setSource(const QString& source) {
    if (source.isEmpty()) {
        m_jsonObject.remove(QLatin1String("source"));
    } else {
        m_jsonObject.insert(QLatin1String("source"), source);
    }
}

mixxx::EncodedUrl Artwork::uri() const {
    return mixxx::EncodedUrl::fromEncodedQByteArray(
            m_jsonObject.value(QLatin1String("uri")).toString().toUtf8());
}

void Artwork::setUri(const mixxx::EncodedUrl& uri) {
    putOptionalNonEmpty(QLatin1String("uri"), uri.toQString());
}

ArtworkImage Artwork::image() const {
    return ArtworkImage{m_jsonObject.value(QLatin1String("image")).toObject()};
}

void Artwork::setImage(ArtworkImage image) {
    if (image.isEmpty()) {
        m_jsonObject.remove(QLatin1String("image"));
    } else {
        m_jsonObject.insert(QLatin1String("image"), image.intoQJsonValue());
    }
}

std::optional<int> ArtworkImage::apicType() const {
    auto value = m_jsonObject.value(QLatin1String("apicType"));
    if (value.isNull() || value.isUndefined()) {
        return std::nullopt;
    }
    const auto apicType = value.toInt(-1);
    if (apicType < 0) {
        return std::nullopt;
    }
    return apicType;
}

void ArtworkImage::setApicType(std::optional<int> apicType) {
    if (apicType) {
        m_jsonObject.insert(QLatin1String("apicType"), *apicType);
    } else {
        m_jsonObject.remove(QLatin1String("apicType"));
    }
}

QString ArtworkImage::mediaType() const {
    return m_jsonObject.value(QLatin1String("mediaType")).toString();
}

void ArtworkImage::setMediaType(QString mediaType) {
    putOptionalNonEmpty(QLatin1String("mediaType"), std::move(mediaType));
}

QByteArray ArtworkImage::digest() const {
    return json::decodeBase64(
            m_jsonObject.value(QLatin1String("digest")).toString());
}

void ArtworkImage::setDigest(const QByteArray& digest) {
    DEBUG_ASSERT(digest.isEmpty() || digest.size() == kArtworkDigestSize);
    putOptionalNonEmpty(QLatin1String("digest"), QLatin1String(json::encodeBase64(digest)));
}

QSize ArtworkImage::size() const {
    const auto arr = m_jsonObject.value(QLatin1String("size")).toArray();
    if (arr.size() != 2) {
        return QSize();
    }
    const auto width = arr.at(0).toInt(-1);
    const auto height = arr.at(1).toInt(-1);
    if (width <= 0 || height <= 0) {
        return QSize();
    }
    return QSize(width, height);
}

void ArtworkImage::setSize(const QSize& size) {
    if (size.isValid() && !size.isEmpty()) {
        m_jsonObject.insert(QLatin1String("size"), QJsonArray{size.width(), size.height()});
    } else {
        m_jsonObject.remove(QLatin1String("size"));
    }
}

QImage ArtworkImage::thumbnail() const {
    const auto bytes = json::decodeBase64(
            m_jsonObject.value(QLatin1String("thumbnail")).toString());
    if (bytes.isEmpty()) {
        return QImage();
    }
    VERIFY_OR_DEBUG_ASSERT(bytes.size() == 4 * 4 * 3) {
        return QImage();
    }
    auto thumbnail = createArtworkThumbnailImage();
    for (int offset = 0; offset < bytes.size(); offset += 3) {
        const auto x = (offset / 3) % kArtworkThumbnailWidth;
        const auto y = (offset / 3) / kArtworkThumbnailWidth;
        const uint r = static_cast<uchar>(bytes[offset]);
        const uint g = static_cast<uchar>(bytes[offset + 1]);
        const uint b = static_cast<uchar>(bytes[offset + 2]);
        thumbnail.setPixelColor(x, y, QColor(r, g, b));
    }
    return thumbnail;
}

void ArtworkImage::setThumbnail(
        const QImage& thumbnail) {
    if (thumbnail.isNull()) {
        m_jsonObject.remove(QLatin1String("thumbnail"));
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(thumbnail.size() ==
            QSize(kArtworkThumbnailWidth, kArtworkThumbnailHeight)) {
        return;
    }
    uchar bytes[kArtworkThumbnailWidth * kArtworkThumbnailHeight * 3];
    int offset = 0;
    for (int x = 0; x < kArtworkThumbnailWidth; ++x) {
        for (int y = 0; y < kArtworkThumbnailHeight; ++y) {
            const auto rgb = thumbnail.pixel(x, y);
            bytes[offset++] = (rgb >> 16) & 0xff;
            bytes[offset++] = (rgb >> 8) & 0xff;
            bytes[offset++] = rgb & 0xff;
        }
    }
    const auto base64 = json::encodeBase64(
            QByteArray::fromRawData(
                    reinterpret_cast<char*>(bytes),
                    sizeof(bytes)));
    m_jsonObject.insert(QLatin1String("thumbnail"), QLatin1String(base64));
}

//static
QImage ArtworkImage::generateThumbnailFromRgbColor(
        mixxx::RgbColor::optional_t colorRgb) {
    const auto color = mixxx::RgbColor::toQColor(colorRgb);
    if (!color.isValid()) {
        return QImage();
    }
    auto thumbnail = createArtworkThumbnailImage();
    thumbnail.fill(color);
    return thumbnail;
}

//static
std::optional<Title> Title::fromQJsonValue(
        const QJsonValue& value) {
    if (value.isString()) {
        return Title(value.toString());
    }
    if (value.isArray()) {
        const auto jsonArray = value.toArray();
        VERIFY_OR_DEBUG_ASSERT(jsonArray.size() == 2) {
            return std::nullopt;
        }
        VERIFY_OR_DEBUG_ASSERT(jsonArray.at(0).isString()) {
            return std::nullopt;
        }
        const int kind = jsonArray.at(1).toInt(kKindInvalid);
        VERIFY_OR_DEBUG_ASSERT(kind != kKindInvalid) {
            return std::nullopt;
        }
        return Title(jsonArray.at(0).toString(), kind);
    }
    VERIFY_OR_DEBUG_ASSERT(value.isObject()) {
        return std::nullopt;
    }
    const auto jsonObject = value.toObject();
    const int kind = jsonObject.value(QLatin1String("kind")).toInt(kKindInvalid);
    VERIFY_OR_DEBUG_ASSERT(kind != kKindInvalid) {
        return std::nullopt;
    }
    const auto name = jsonObject.value(QLatin1String("name")).toString();
    return Title{name, kind};
}

QJsonValue Title::toQJsonValue() const {
    if (m_kind == kKindDefault) {
        // name
        return m_name;
    } else {
        // [name, kind]
        QJsonArray nameKind;
        nameKind.append(m_name);
        nameKind.append(m_kind);
        return nameKind;
    }
}

//static
std::optional<Actor> Actor::fromQJsonValue(
        const QJsonValue& value) {
    if (value.isString()) {
        return Actor(value.toString());
    }
    if (value.isArray()) {
        const auto jsonArray = value.toArray();
        VERIFY_OR_DEBUG_ASSERT(jsonArray.size() == 2) {
            return std::nullopt;
        }
        VERIFY_OR_DEBUG_ASSERT(jsonArray.at(0).isString()) {
            return std::nullopt;
        }
        const auto role = jsonArray.at(1).toInt(kRoleInvalid);
        VERIFY_OR_DEBUG_ASSERT(role != kRoleInvalid) {
            return std::nullopt;
        }
        return Actor(jsonArray.at(0).toString(), role);
    }
    VERIFY_OR_DEBUG_ASSERT(value.isObject()) {
        return std::nullopt;
    }
    const auto jsonObject = value.toObject();
    const int kind = jsonObject.value(QLatin1String("kind")).toInt(kKindInvalid);
    VERIFY_OR_DEBUG_ASSERT(kind != kKindInvalid) {
        return std::nullopt;
    }
    const int role = jsonObject.value(QLatin1String("role")).toInt(kRoleInvalid);
    VERIFY_OR_DEBUG_ASSERT(role != kRoleInvalid) {
        return std::nullopt;
    }
    const auto name = jsonObject.value(QLatin1String("name")).toString();
    auto actor = Actor{name, role};
    actor.m_kind = kind;
    actor.m_roleNotes = jsonObject.value(QLatin1String("roleNotes")).toString();
    return actor;
}

QJsonValue Actor::toQJsonValue() const {
    if (m_kind == kKindDefault && m_roleNotes.isNull()) {
        if (m_role == kRoleDefault) {
            // name
            return m_name;
        } else {
            // [name, role]
            QJsonArray nameRole;
            nameRole.append(m_name);
            nameRole.append(m_role);
            return nameRole;
        }
    } else {
        // { name, kind, role, roleNotes }
        auto jsonObject = QJsonObject{
                {QLatin1String("name"), m_name},
        };
        if (m_kind != kKindDefault) {
            jsonObject.insert(QLatin1String("kind"), m_kind);
        }
        if (m_role != kRoleDefault) {
            jsonObject.insert(QLatin1String("role"), m_role);
        }
        if (!m_roleNotes.isNull()) {
            jsonObject.insert(QLatin1String("roleNotes"), m_roleNotes);
        }
        return jsonObject;
    }
}

TitleVector TrackOrAlbum::titles(int kind) const {
    TitleVector result;
    const QJsonArray jsonTitles = m_jsonObject.value(QLatin1String("titles")).toArray();
    for (auto&& jsonValue : jsonTitles) {
        auto title = Title::fromQJsonValue(jsonValue);
        VERIFY_OR_DEBUG_ASSERT(title) {
            continue;
        }
        if (title->kind() == kind) {
            result += std::move(*title);
        }
    }
    return result;
}

TitleVector TrackOrAlbum::allTitles() const {
    TitleVector result;
    const QJsonArray jsonTitles = m_jsonObject.value(QLatin1String("titles")).toArray();
    result.reserve(jsonTitles.size());
    for (auto&& jsonValue : jsonTitles) {
        auto title = Title::fromQJsonValue(jsonValue);
        VERIFY_OR_DEBUG_ASSERT(title) {
            continue;
        }
        result += std::move(*title);
    }
    return result;
}

TitleVector TrackOrAlbum::removeTitles(int kind) {
    TitleVector result;
    // NOTE(uklotzde, 2019-07-28): Modifying the QJsonArray in place
    // caused undefined behavior and crashes with Qt 5.12.4!
    const auto oldTitles = m_jsonObject.value(QLatin1String("titles")).toArray();
    QJsonArray newTitles;
    for (const auto&& jsonValue : oldTitles) {
        auto title = Title::fromQJsonValue(jsonValue);
        VERIFY_OR_DEBUG_ASSERT(title) {
            continue;
        }
        if (title->kind() == kind) {
            result += std::move(*title);
        } else {
            newTitles += jsonValue;
        }
    }
    putOptionalNonEmpty(QLatin1String("titles"), std::move(newTitles));
    return result;
}

TitleVector TrackOrAlbum::clearTitles() {
    TitleVector result;
    const QJsonArray jsonTitles = m_jsonObject.take(QLatin1String("titles")).toArray();
    result.reserve(jsonTitles.size());
    for (const auto&& jsonValue : jsonTitles) {
        auto title = Title::fromQJsonValue(jsonValue);
        VERIFY_OR_DEBUG_ASSERT(title) {
            continue;
        }
        result += std::move(*title);
    }
    return result;
}

void TrackOrAlbum::addTitles(TitleVector titles) {
    if (titles.isEmpty()) {
        // Avoid any modifications if noop
        return;
    }
    QJsonArray jsonTitles = m_jsonObject.take(QLatin1String("titles")).toArray();
    for (auto&& title : titles) {
        jsonTitles += title.toQJsonValue();
    }
    putOptionalNonEmpty(QLatin1String("titles"), std::move(jsonTitles));
}

ActorVector TrackOrAlbum::actors(int role, int kind) const {
    ActorVector result;
    const QJsonArray jsonActors = m_jsonObject.value(QLatin1String("actors")).toArray();
    for (auto&& jsonValue : jsonActors) {
        auto actor = Actor::fromQJsonValue(jsonValue);
        VERIFY_OR_DEBUG_ASSERT(actor) {
            continue;
        }
        if (actor->role() == role && actor->kind() == kind) {
            result += std::move(*actor);
        }
    }
    return result;
}

ActorVector TrackOrAlbum::allActors() const {
    ActorVector result;
    const QJsonArray jsonActors = m_jsonObject.value(QLatin1String("actors")).toArray();
    result.reserve(jsonActors.size());
    for (auto&& jsonValue : jsonActors) {
        auto actor = Actor::fromQJsonValue(jsonValue);
        VERIFY_OR_DEBUG_ASSERT(actor) {
            continue;
        }
        result += std::move(*actor);
    }
    return result;
}

ActorVector TrackOrAlbum::removeActors(int role) {
    ActorVector result;
    // NOTE(uklotzde, 2019-07-28): Modifying the QJsonArray in place
    // caused undefined behavior and crashes with Qt 5.12.4!
    const auto oldActors = m_jsonObject.value(QLatin1String("actors")).toArray();
    QJsonArray newActors;
    for (const auto&& jsonValue : oldActors) {
        auto actor = Actor::fromQJsonValue(jsonValue);
        VERIFY_OR_DEBUG_ASSERT(actor) {
            continue;
        }
        if (actor->role() == role) {
            result += std::move(*actor);
        } else {
            newActors += std::move(jsonValue);
        }
    }
    putOptionalNonEmpty(QLatin1String("actors"), std::move(newActors));
    return result;
}

ActorVector TrackOrAlbum::clearActors() {
    ActorVector result;
    const QJsonArray jsonActors = m_jsonObject.take(QLatin1String("actors")).toArray();
    result.reserve(jsonActors.size());
    for (const auto&& jsonValue : jsonActors) {
        auto actor = Actor::fromQJsonValue(jsonValue);
        VERIFY_OR_DEBUG_ASSERT(actor) {
            continue;
        }
        result += std::move(*actor);
    }
    return result;
}

void TrackOrAlbum::addActors(const ActorVector& actors) {
    if (actors.isEmpty()) {
        // Avoid any modifications if noop
        return;
    }
    QJsonArray jsonActors = m_jsonObject.take(QLatin1String("actors")).toArray();
    for (const auto& actor : actors) {
        jsonActors += actor.toQJsonValue();
    }
    putOptionalNonEmpty(QLatin1String("actors"), std::move(jsonActors));
}

std::optional<int> Album::type() const {
    const auto jsonValue = m_jsonObject.value(QLatin1String("type"));
    if (jsonValue.isNull() || jsonValue.isUndefined()) {
        return std::nullopt;
    }
    const auto intValue = jsonValue.toInt(kTypeInvalid);
    VERIFY_OR_DEBUG_ASSERT(intValue != kTypeInvalid) {
        return std::nullopt;
    }
    return intValue;
}

void Album::setType(std::optional<int> type) {
    if (type) {
        m_jsonObject.insert(QLatin1String("type"), *type);
    } else {
        m_jsonObject.remove(QLatin1String("type"));
    }
}

QString Release::releasedAt() const {
    return importDateTimeOrYear(m_jsonObject.value(QLatin1String("releasedAt")))
            .value_or(QString());
}

void Release::setReleasedAt(const QString& releasedAt) {
    putOptionalNonEmpty(QLatin1String("releasedAt"), exportDateTimeOrYear(releasedAt));
}

QString Release::releasedBy() const {
    return m_jsonObject.value(QLatin1String("releasedBy")).toString();
}

void Release::setReleasedBy(QString label) {
    putOptionalNonEmpty(QLatin1String("releasedBy"), std::move(label));
}

QString Release::copyright() const {
    return m_jsonObject.value(QLatin1String("copyright")).toString();
}

void Release::setCopyright(QString copyright) {
    putOptional(QLatin1String("copyright"), std::move(copyright));
}

std::optional<QDateTime> PlayCounter::lastPlayedAt() const {
    return importDateTime(m_jsonObject.value(QLatin1String("lastPlayedAt")));
}

void PlayCounter::setLastPlayedAt(const QDateTime& lastPlayedAt) {
    putOptionalNonEmpty(QLatin1String("lastPlayedAt"), exportDateTime(lastPlayedAt));
}

int PlayCounter::timesPlayed() const {
    return m_jsonObject.value(QLatin1String("timesPlayed")).toInt(0);
}

void PlayCounter::setTimesPlayed(int playCount) {
    if (playCount > 0) {
        m_jsonObject.insert(QLatin1String("timesPlayed"), playCount);
    } else {
        m_jsonObject.remove(QLatin1String("timesPlayed"));
    }
}

std::optional<QDateTime> MediaSource::collectedAt() const {
    return importDateTime(m_jsonObject.value(QLatin1String("collectedAt")));
}

void MediaSource::setCollectedAt(const QDateTime& collectedAt) {
    putOptionalNonEmpty(QLatin1String("collectedAt"), exportDateTime(collectedAt));
}

PlayCounter Track::playCounter() const {
    return PlayCounter(m_jsonObject.value(QLatin1String("playCounter")).toObject());
}

void Track::setPlayCounter(PlayCounter playCounter) {
    m_jsonObject.insert(QLatin1String("playCounter"), playCounter.intoQJsonValue());
}

mixxx::RgbColor::optional_t Track::color() const {
    return mixxx::RgbColor::fromQString(
            m_jsonObject
                    .value(QLatin1String("color"))
                    .toObject()
                    .value(QLatin1String("rgb"))
                    .toString());
}

void Track::setColor(
        mixxx::RgbColor::optional_t color) {
    if (color) {
        m_jsonObject.insert(QLatin1String("color"),
                QJsonObject{
                        {QLatin1String("rgb"),
                                mixxx::RgbColor::toQString(*color)},
                });
    } else {
        m_jsonObject.remove(QLatin1String("color"));
    }
}

CueMarkers Track::cueMarkers() const {
    return CueMarkers(m_jsonObject.value(QLatin1String("cues")).toArray());
}

void Track::setCueMarkers(CueMarkers cueMarkers) {
    putOptionalNonEmpty(QLatin1String("cues"), cueMarkers.intoQJsonValue());
}

MusicMetrics Track::musicMetrics() const {
    return MusicMetrics(m_jsonObject.value(QLatin1String("metrics")).toObject());
}

void Track::setMusicMetrics(MusicMetrics musicMetrics) {
    putOptionalNonEmpty(QLatin1String("metrics"), musicMetrics.intoQJsonValue());
}

Release Track::release() const {
    return Release(m_jsonObject.value(QLatin1String("release")).toObject());
}

void Track::setRelease(Release release) {
    putOptionalNonEmpty(QLatin1String("release"), release.intoQJsonValue());
}

Album Track::album() const {
    return Album(m_jsonObject.value(QLatin1String("album")).toObject());
}

void Track::setAlbum(Album album) {
    putOptionalNonEmpty(QLatin1String("album"), album.intoQJsonValue());
}

Tags Track::tags() const {
    return Tags(m_jsonObject.value(QLatin1String("tags")).toObject());
}

Tags Track::removeTags() {
    return Tags(m_jsonObject.take(QLatin1String("tags")).toObject());
}

void Track::setTags(Tags tags) {
    putOptionalNonEmpty(QLatin1String("tags"), tags.intoQJsonValue());
}

QString Track::trackNumbers() const {
    const auto value = m_jsonObject.value(QLatin1String("indexes"))
                               .toObject()
                               .value(QLatin1String("track"));
    if (value.isArray()) {
        auto array = value.toArray();
        DEBUG_ASSERT(array.size() == 2);
        return QString("%1/%2").arg(QString::number(array.at(0).toInt()),
                QString::number(array.at(1).toInt()));
    } else {
        return QString::number(value.toInt());
    }
}

QString Track::discNumbers() const {
    const auto value = m_jsonObject.value(QLatin1String("indexes"))
                               .toObject()
                               .value(QLatin1String("disc"));
    if (value.isArray()) {
        auto array = value.toArray();
        DEBUG_ASSERT(array.size() == 2);
        return QString("%1/%2").arg(
                QString::number(array.at(0).toInt()),
                QString::number(array.at(1).toInt()));
    } else {
        return QString::number(value.toInt());
    }
}

void Track::setIndexNumbers(
        const mixxx::TrackInfo& trackInfo) {
    auto indexes = m_jsonObject.value(QLatin1String("indexes")).toObject();

    auto trackNumber = optionalPositiveIntJsonValue(trackInfo.getTrackNumber());
    auto trackTotal = optionalPositiveIntJsonValue(trackInfo.getTrackTotal());
    if (trackTotal.isNull()) {
        if (trackNumber.isNull()) {
            indexes.remove(QLatin1String("track"));
        } else {
            // Single value
            indexes.insert(QLatin1String("track"), std::move(trackNumber));
        }
    } else {
        // Tuple
        auto trackNumbers = QJsonArray{
                trackNumber.isNull() ? QJsonValue(0) : trackNumber,
                trackTotal,
        };
        indexes.insert(QLatin1String("track"), std::move(trackNumbers));
    }

    auto discNumber = optionalPositiveIntJsonValue(trackInfo.getDiscNumber());
    auto discTotal = optionalPositiveIntJsonValue(trackInfo.getDiscTotal());
    if (discTotal.isNull()) {
        if (discNumber.isNull()) {
            indexes.remove(QLatin1String("disc"));
        } else {
            // Single value
            indexes.insert(QLatin1String("disc"), std::move(discNumber));
        }
    } else {
        // Tuple
        auto discNumbers = QJsonArray{
                discNumber.isNull() ? QJsonValue(0) : discNumber,
                discTotal,
        };
        indexes.insert(QLatin1String("disc"), std::move(discNumbers));
    }

    putOptionalNonEmpty(QLatin1String("indexes"), indexes);
}

MediaSource Track::mediaSource() const {
    return MediaSource(
            m_jsonObject.value(QLatin1String("mediaSource")).toObject());
}

void Track::setMediaSource(MediaSource mediaSource) {
    putOptionalNonEmpty(QLatin1String("mediaSource"), mediaSource.intoQJsonValue());
}

EntityHeader TrackEntity::header() const {
    return EntityHeader(m_jsonArray.at(0).toArray());
}

Track TrackEntity::body() const {
    return Track(m_jsonArray.at(1).toObject());
}

void TrackEntity::setBody(Track body) {
    while (m_jsonArray.size() <= 1) {
        m_jsonArray.append(QJsonValue());
    }
    m_jsonArray.replace(1, body.intoQJsonValue());
}

MusicMetricsFlags MusicMetrics::flags() const {
    return MusicMetricsFlags(
            m_jsonObject
                    .value(QLatin1String("flags"))
                    .toInt(MusicMetricsFlags(MusicMetricsBitflags::Default)));
}

void MusicMetrics::setFlags(MusicMetricsFlags locks) {
    if (locks == MusicMetricsFlags(MusicMetricsBitflags::Default)) {
        m_jsonObject.remove(QLatin1String("flags"));
    } else {
        m_jsonObject.insert(QLatin1String("flags"), static_cast<int>(locks));
    }
}

mixxx::Bpm MusicMetrics::bpm() const {
    return mixxx::Bpm(
            m_jsonObject
                    .value(QLatin1String("tempoBpm"))
                    .toDouble(mixxx::Bpm::kValueUndefined));
}

void MusicMetrics::setBpm(mixxx::Bpm bpm) {
    if (bpm.isValid()) {
        m_jsonObject.insert(QLatin1String("tempoBpm"), bpm.value());
    } else {
        m_jsonObject.remove(QLatin1String("tempoBpm"));
    }
}

mixxx::track::io::key::ChromaticKey MusicMetrics::key() const {
    const int keyCode = m_jsonObject
                                .value(QLatin1String("keyCode"))
                                .toInt(0);
    DEBUG_ASSERT(keyCode >= 0);
    DEBUG_ASSERT(keyCode <= 24);
    if ((keyCode <= 0) || (keyCode > 24)) {
        return mixxx::track::io::key::ChromaticKey::INVALID;
    }
    const int openKeyCode = 1 + (keyCode - 1) / 2;
    const bool major = (keyCode % 2) == 1;
    return KeyUtils::openKeyNumberToKey(openKeyCode, major);
}

void MusicMetrics::setKey(
        mixxx::track::io::key::ChromaticKey chromaticKey) {
    if (chromaticKey == mixxx::track::io::key::ChromaticKey::INVALID) {
        m_jsonObject.remove(QLatin1String("keyCode"));
    } else {
        const int openKeyNumber = KeyUtils::keyToOpenKeyNumber(chromaticKey);
        DEBUG_ASSERT(openKeyNumber >= 1);
        DEBUG_ASSERT(openKeyNumber <= 12);
        const int keyCode = 2 * openKeyNumber - (KeyUtils::keyIsMajor(chromaticKey) ? 1 : 0);
        m_jsonObject.insert(QLatin1String("keyCode"), keyCode);
    }
}

} // namespace json

} // namespace aoide
