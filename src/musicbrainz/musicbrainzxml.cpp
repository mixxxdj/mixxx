#include "musicbrainz/musicbrainzxml.h"

#include <QTextStream>
#include <QXmlStreamReader>
#include <QTextCodec>

#include "util/logger.h"


namespace mixxx {

namespace musicbrainz {

namespace {

Logger kLogger("MusicBrainz XML");

constexpr int kDefaultErrorCode = 0;

bool finishElement(
        const QXmlStreamReader& reader,
        const QString& elementName = QString()) {
    if (reader.tokenType() != QXmlStreamReader::EndElement) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(elementName.isEmpty() || reader.name() == elementName) {
        kLogger.warning()
                << "Unexpected end element tag"
                << reader.name()
                << "instead of"
                << elementName;
        return false;
    }
    return true;
}

bool continueReading(QXmlStreamReader& reader) {
    return !(reader.atEnd() || reader.hasError());
}

void consumeCurrentElement(QXmlStreamReader& reader) {
    DEBUG_ASSERT(reader.tokenType() == QXmlStreamReader::StartElement);
    const auto elementName = reader.name().toString();
    int level = 0;
    while (continueReading(reader)) {
        switch (reader.readNext()) {
        case QXmlStreamReader::StartElement:
            if (kLogger.traceEnabled()) {
                kLogger.trace()
                        << "Start skipping"
                        << reader.name();
            }
            ++level;
            break;
        case QXmlStreamReader::EndElement:
            if (level == 0) {
                finishElement(reader, elementName);
                return;
            }
            DEBUG_ASSERT(level > 0);
            if (kLogger.traceEnabled()) {
                kLogger.trace()
                        << "End skipping"
                        << reader.name();
            }
            --level;
            break;
        case QXmlStreamReader::Characters:
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::ProcessingInstruction:
            // ignore token types
        default:
            DEBUG_ASSERT(reader.tokenType() != QXmlStreamReader::NoToken);
            DEBUG_ASSERT(reader.tokenType() != QXmlStreamReader::Invalid);
            DEBUG_ASSERT(reader.tokenType() != QXmlStreamReader::StartDocument);
            DEBUG_ASSERT(reader.tokenType() != QXmlStreamReader::EndDocument);
            DEBUG_ASSERT(reader.tokenType() != QXmlStreamReader::DTD);
            DEBUG_ASSERT(reader.tokenType() != QXmlStreamReader::EntityReference);
        }
    }
}

void skipCurrentElement(QXmlStreamReader& reader) {
    if (kLogger.traceEnabled()) {
        kLogger.trace()
                << "Skipping current element"
                << reader.name();
    }
    consumeCurrentElement(reader);
}

bool parseDuration(
        const QString& text,
        Duration* pDuration = nullptr) {
    bool ok = false;
    int durationMillis = text.toInt(&ok);
    if (ok && durationMillis > 0) {
        if (pDuration) {
            *pDuration = Duration::fromMillis(durationMillis);
        }
        return true;
    }
    kLogger.warning()
            << "Invalid duration [ms]:"
            << text;
    return false;
}

void readElementArtistCredit(QXmlStreamReader& reader, QString& artistName, QUuid& artistId) {
    DEBUG_ASSERT(reader.name() == QLatin1String("artist-credit"));
    DEBUG_ASSERT(artistName.isEmpty());
    DEBUG_ASSERT(artistId.isNull());

    QString nextJoinPhrase;
    while (continueReading(reader)) {
        const QXmlStreamReader::TokenType type = reader.readNext();
        if (type == QXmlStreamReader::StartElement) {
            if (reader.name() == QLatin1String("name-credit")) {
                QString thisJoinPhrase = nextJoinPhrase;
                nextJoinPhrase = reader.attributes().value(QStringLiteral("joinphrase")).toString();
                QString creditedName;
                // Consume name-credit
                while (continueReading(reader)) {
                    const QXmlStreamReader::TokenType type = reader.readNext();
                    if (type == QXmlStreamReader::StartElement) {
                        if (reader.name() == QLatin1String("name")) {
                            creditedName = reader.readElementText();
                        } else if (reader.name() == QLatin1String("artist")) {
                            // Consume artist
                            if (artistId.isNull()) {
                                artistId = QUuid(reader.attributes().value(QStringLiteral("id")).toString());
                            } else {
                                kLogger.info()
                                        << "Ignoring additional artist id"
                                        << reader.attributes().value(QStringLiteral("id"));
                            }
                            DEBUG_ASSERT(!artistId.isNull());
                            if (creditedName.isEmpty()) {
                                while (continueReading(reader)) {
                                    const QXmlStreamReader::TokenType type = reader.readNext();
                                    if (type == QXmlStreamReader::StartElement) {
                                        if (reader.name() == QLatin1String("name")) {
                                            creditedName = reader.readElementText();
                                        } else {
                                            skipCurrentElement(reader);
                                        }
                                    } else if (finishElement(reader, QStringLiteral("artist"))) {
                                        break;
                                    }
                                }
                            } else {
                                // Already parsed from credited name
                                skipCurrentElement(reader);
                            }
                        } else {
                            skipCurrentElement(reader);
                        }
                    } else if (finishElement(reader, QStringLiteral("name-credit"))) {
                        break;
                    }
                }
                // Apply credited name
                DEBUG_ASSERT(!creditedName.isEmpty());
                if (!artistName.isEmpty()) {
                    DEBUG_ASSERT(!thisJoinPhrase.isEmpty());
                    artistName.append(thisJoinPhrase);
                    thisJoinPhrase = QString();
                }
                artistName.append(creditedName);
            } else {
                skipCurrentElement(reader);
            }
        } else if (finishElement(reader, QStringLiteral("artist-credit"))) {
            break;
        }
    }
}

TrackRelease readElementRelease(
        QXmlStreamReader& reader) {
    DEBUG_ASSERT(reader.name() == QLatin1String("release"));

    QString firstReleaseDate;
    QString releaseGroupTitle;
    QString releaseGroupArtist;
    QUuid releaseGroupArtistId;
    QString releaseTitleDisambiguation;
    QString discNumber;
    QString trackNumber;
    TrackRelease trackRelease;
    trackRelease.albumReleaseId = QUuid(reader.attributes().value(QStringLiteral("id")).toString());
    DEBUG_ASSERT(!trackRelease.albumReleaseId.isNull());
    while (continueReading(reader)) {
        const QXmlStreamReader::TokenType type = reader.readNext();
        if (type == QXmlStreamReader::StartElement) {
            if (reader.name() == QLatin1String("title")) {
                DEBUG_ASSERT(trackRelease.albumTitle.isNull());
                trackRelease.albumTitle = reader.readElementText();
            } else if (reader.name() == QLatin1String("disambiguation")) {
                DEBUG_ASSERT(releaseTitleDisambiguation.isNull());
                releaseTitleDisambiguation = reader.readElementText();
            } else if (reader.name() == QLatin1String("artist-credit")) {
                readElementArtistCredit(reader, trackRelease.albumArtist, trackRelease.albumArtistId);
            } else if (reader.name() == QLatin1String("date")) {
                DEBUG_ASSERT(trackRelease.date.isNull());
                trackRelease.date = reader.readElementText();
            } else if (reader.name() == QLatin1String("release-group")) {
                DEBUG_ASSERT(trackRelease.releaseGroupId.isNull());
                trackRelease.releaseGroupId = QUuid(reader.attributes().value(QStringLiteral("id")).toString());
                DEBUG_ASSERT(!trackRelease.releaseGroupId.isNull());
                DEBUG_ASSERT(trackRelease.releaseGroupType.isNull());
                trackRelease.releaseGroupType = reader.attributes().value(QStringLiteral("type")).toString();
                // Consume release-group
                while (continueReading(reader)) {
                    const QXmlStreamReader::TokenType type = reader.readNext();
                    if (type == QXmlStreamReader::StartElement) {
                        if (reader.name() == QLatin1String("first-release-date")) {
                            firstReleaseDate = reader.readElementText();
                        } else if (reader.name() == QLatin1String("title")) {
                            releaseGroupTitle = reader.readElementText();
                        } else if (reader.name() == QLatin1String("artist-credit")) {
                            readElementArtistCredit(reader, releaseGroupArtist, releaseGroupArtistId);
                        } else {
                            skipCurrentElement(reader);
                        }
                    } else if (finishElement(reader, QStringLiteral("release-group"))) {
                        break;
                    }
                }
            } else if (reader.name() == QLatin1String("medium-list")) {
                DEBUG_ASSERT(trackRelease.discTotal.isNull());
                trackRelease.discTotal = reader.attributes().value(QStringLiteral("count")).toString();
                // Consume medium-list (with a single medium element)
                int mediumCount = 0;
                while (continueReading(reader)) {
                    const QXmlStreamReader::TokenType type = reader.readNext();
                    if (type == QXmlStreamReader::StartElement) {
                        if (reader.name() == QLatin1String("medium")) {
                            // Consume medium
                            VERIFY_OR_DEBUG_ASSERT(++mediumCount == 1) {
                                kLogger.warning()
                                        << "Ignoring additional medium"
                                        << mediumCount
                                        << "in medium-list";
                                consumeCurrentElement(reader);
                                continue;
                            }
                            while (continueReading(reader)) {
                                const QXmlStreamReader::TokenType type = reader.readNext();
                                if (type == QXmlStreamReader::StartElement) {
                                    if (reader.name() == QLatin1String("position")) {
                                        DEBUG_ASSERT(trackRelease.discNumber.isNull());
                                        trackRelease.discNumber = reader.readElementText();
                                    } else if (reader.name() == QLatin1String("number")) {
                                        // Literal, non-numeric disc number
                                        DEBUG_ASSERT(discNumber.isNull());
                                        discNumber = reader.readElementText();
                                    } else if (reader.name() == QLatin1String("format")) {
                                        DEBUG_ASSERT(trackRelease.mediumFormat.isNull());
                                        trackRelease.mediumFormat = reader.readElementText();
                                    } else if (reader.name() == QLatin1String("track-list")) {
                                        DEBUG_ASSERT(trackRelease.trackTotal.isNull());
                                        trackRelease.trackTotal = reader.attributes().value(QStringLiteral("count")).toString();
                                        // Consume track-list (with a single track element)
                                        int trackCount = 0;
                                        while (continueReading(reader)) {
                                            const QXmlStreamReader::TokenType type = reader.readNext();
                                            if (type == QXmlStreamReader::StartElement) {
                                                if (reader.name() == QLatin1String("track")) {
                                                    // Consume track
                                                    VERIFY_OR_DEBUG_ASSERT(++trackCount == 1) {
                                                        kLogger.warning()
                                                                << "Ignoring additional track"
                                                                << trackCount
                                                                << "in track-list";
                                                        consumeCurrentElement(reader);
                                                        continue;
                                                    }
                                                    DEBUG_ASSERT(trackRelease.trackReleaseId.isNull());
                                                    trackRelease.trackReleaseId = QUuid(reader.attributes().value(QStringLiteral("id")).toString());
                                                    DEBUG_ASSERT(!trackRelease.trackReleaseId.isNull());
                                                    while (continueReading(reader)) {
                                                        const QXmlStreamReader::TokenType type = reader.readNext();
                                                        if (type == QXmlStreamReader::StartElement) {
                                                            if (reader.name() ==
                                                                    QLatin1String("position")) {
                                                                DEBUG_ASSERT(trackRelease.trackNumber.isNull());
                                                                trackRelease.trackNumber = reader.readElementText();
                                                            } else if (reader.name() ==
                                                                    QLatin1String("number")) {
                                                                // Literal, non-numeric track number
                                                                DEBUG_ASSERT(trackNumber.isNull());
                                                                trackNumber = reader.readElementText();
                                                            } else if (reader.name() ==
                                                                    QLatin1String("title")) {
                                                                DEBUG_ASSERT(trackRelease.title.isNull());
                                                                trackRelease.title = reader.readElementText();
                                                            } else if (reader.name() ==
                                                                    QLatin1String("length")) {
                                                                DEBUG_ASSERT(trackRelease.duration == Duration::empty());
                                                                parseDuration(reader.readElementText(), &trackRelease.duration);
                                                            } else {
                                                                skipCurrentElement(reader);
                                                            }
                                                        } else if (finishElement(reader, QStringLiteral("track"))) {
                                                            break;
                                                        }
                                                    }
                                                } else {
                                                    skipCurrentElement(reader);
                                                }
                                            } else if (finishElement(reader, QStringLiteral("track-list"))) {
                                                break;
                                            }
                                        }
                                    } else {
                                        skipCurrentElement(reader);
                                    }
                                } else if (finishElement(reader, QStringLiteral("medium"))) {
                                    break;
                                }
                            }
                        } else {
                            skipCurrentElement(reader);
                        }
                    } else if (finishElement(reader, QStringLiteral("medium-list"))) {
                        break;
                    }
                }
            } else {
                skipCurrentElement(reader);
            }
        } else if (finishElement(reader, QStringLiteral("release"))) {
            break;
        }
    }

    if (trackRelease.date.isEmpty()) {
        trackRelease.date = firstReleaseDate;
    }
    if (trackRelease.albumTitle.isEmpty()) {
        DEBUG_ASSERT(releaseTitleDisambiguation.isEmpty());
        trackRelease.albumTitle = releaseGroupTitle;
    } else {
        if (!releaseTitleDisambiguation.isEmpty()) {
            trackRelease.albumTitle = QStringLiteral("%1 (%2)").arg(trackRelease.albumTitle, releaseTitleDisambiguation);
        }
    }
    if (trackRelease.albumArtist.isEmpty()) {
        trackRelease.albumArtist = releaseGroupArtist;
        trackRelease.albumArtistId = releaseGroupArtistId;
    }
    if (trackRelease.discNumber.isEmpty()) {
        // Use literal disc number only if numeric disc number not available
        trackRelease.discNumber = discNumber;
    }
    if (trackRelease.trackNumber.isEmpty()) {
        // Use literal track number only if numeric track number not available
        trackRelease.trackNumber = trackNumber;
    }

    return trackRelease;
}

QPair<QList<TrackRelease>, bool> readElementRecording(QXmlStreamReader& reader) {
    DEBUG_ASSERT(reader.name() == QLatin1String("recording"));

    QList<TrackRelease> trackReleases;
    const auto recordingId =
            QUuid(reader.attributes().value(QStringLiteral("id")).toString());
    DEBUG_ASSERT(!recordingId.isNull());

    QString recordingTitle;
    QString recordingArtist;
    QUuid recordingArtistId;
    QUuid releaseGroupId;
    QString releaseGroupType;
    Duration recordingDuration;
    while (continueReading(reader)) {
        const QXmlStreamReader::TokenType type = reader.readNext();
        if (type == QXmlStreamReader::StartElement) {
            if (reader.name() == QLatin1String("title")) {
                DEBUG_ASSERT(recordingTitle.isNull());
                recordingTitle = reader.readElementText();
            } else if (reader.name() == QLatin1String("length")) {
                DEBUG_ASSERT(recordingDuration == Duration::empty());
                parseDuration(reader.readElementText(), &recordingDuration);
            } else if (reader.name() == QLatin1String("artist-credit")) {
                readElementArtistCredit(reader, recordingArtist, recordingArtistId);
            } else if (reader.name() == QLatin1String("release-list")) {
                // Consume release-list
                while (continueReading(reader)) {
                    const QXmlStreamReader::TokenType type = reader.readNext();
                    if (type == QXmlStreamReader::StartElement) {
                        if (reader.name() == QLatin1String("release")) {
                            trackReleases.append(readElementRelease(reader));
                        } else if (reader.name() == QLatin1String("release-group")) {
                            DEBUG_ASSERT(releaseGroupId.isNull());
                            releaseGroupId = QUuid(reader.attributes().value(QStringLiteral("id")).toString());
                            DEBUG_ASSERT(!releaseGroupId.isNull());
                            DEBUG_ASSERT(releaseGroupType.isNull());
                            releaseGroupType = reader.attributes().value(QStringLiteral("type")).toString();
                            consumeCurrentElement(reader);
                        } else {
                            skipCurrentElement(reader);
                        }
                    } else if (finishElement(reader, QStringLiteral("release-list"))) {
                        break;
                    }
                }
            } else {
                skipCurrentElement(reader);
            }
        } else if (finishElement(reader, QStringLiteral("recording"))) {
            break;
        }
    }

    // Merge releases with common recording properties
    for (auto&& trackRelease : trackReleases) {
        DEBUG_ASSERT(trackRelease.recordingId.isNull());
        trackRelease.recordingId = recordingId;
        if (trackRelease.releaseGroupId.isNull()) {
            trackRelease.releaseGroupId = releaseGroupId;
        }
        if (trackRelease.releaseGroupType.isEmpty()) {
            trackRelease.releaseGroupType = releaseGroupType;
        }
        if (trackRelease.title.isEmpty()) {
            trackRelease.title = recordingTitle;
        }
        if (trackRelease.artist.isEmpty()) {
            trackRelease.artist = recordingArtist;
        }
        if (trackRelease.artistId.isNull()) {
            trackRelease.artistId = recordingArtistId;
        }
        if (trackRelease.duration == Duration::empty()) {
            trackRelease.duration = recordingDuration;
        }
    }

    return qMakePair(trackReleases, true);
}

} // anonymous namespace

Error::Error()
        : code(kDefaultErrorCode) {
}

Error::Error(
        QXmlStreamReader& reader)
        : Error() {
    while (continueReading(reader)) {
        if (reader.readNext() == QXmlStreamReader::StartElement) {
            if (reader.name() == QLatin1String("message")) {
                DEBUG_ASSERT(message == Error().message);
                message = reader.readElementText();
            } else if (reader.name() == QLatin1String("code")) {
                DEBUG_ASSERT(code == Error().code);
                bool ok;
                int val = reader.readElementText().toInt(&ok);
                if (ok) {
                    code = val;
                }
            }
        }
    }
}

QPair<QList<TrackRelease>, bool> parseRecordings(QXmlStreamReader& reader) {
    QList<TrackRelease> trackReleases;
    while (continueReading(reader)) {
        switch (reader.readNext()) {
        case QXmlStreamReader::Invalid:
        {
            return qMakePair(trackReleases, false);
        }
        case QXmlStreamReader::StartElement:
        {
            if (reader.name() == QLatin1String("recording")) {
                auto recordingResult = readElementRecording(reader);
                trackReleases += recordingResult.first;
                if (!recordingResult.second) {
                    DEBUG_ASSERT(reader.hasError());
                    kLogger.warning()
                            << "Aborting reading recording results after error"
                            << reader.errorString();
                    return qMakePair(trackReleases, false);
                }
            }
            break;
        }
        default:
        {
            // ignore any other token type
        }
        }
    }
    return qMakePair(trackReleases, true);
}

} // namespace musicbrainz

} // namespace mixxx
