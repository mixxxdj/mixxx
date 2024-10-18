#pragma once

#include <QObject>

#include "gmock/gmock.h"
#include "musicbrainz/musicbrainz.h"
#include "network/webtask.h"

class MockMusicBrainzReceiver : public QObject {
    Q_OBJECT
  public:
    MockMusicBrainzReceiver() = default;
    ~MockMusicBrainzReceiver() override = default;

    MOCK_METHOD(void, MocSucceeded, ());
    MOCK_METHOD(void, MocFailed, ());
    MOCK_METHOD(void, MocAborted, ());
    MOCK_METHOD(void, MocNetworkError, ());

  public slots:
    void slotMusicBrainzTaskSucceeded(
            const QList<mixxx::musicbrainz::TrackRelease>& guessedTrackReleases);
    void slotMusicBrainzTaskFailed(
            const mixxx::network::WebResponse& response,
            int errorCode,
            const QString& errorMessage);
    void slotMusicBrainzTaskAborted();
    void slotMusicBrainzTaskNetworkError(
            QNetworkReply::NetworkError errorCode,
            const QString& errorString,
            const mixxx::network::WebResponseWithContent& responseWithContent);
};
