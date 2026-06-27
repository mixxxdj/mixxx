#pragma once

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QTimer>

#include "util/logger.h"

namespace mixxx {

struct MusicMatcherSuggestion {
    QString trackId;
    QString title;
    QString artist;
    QString album;
    int duration = 0;
    bool isLive = false;
    bool isInstrumental = false;
    bool isAward = false;
    bool isFestival = false;
    QString sourceUrl;

    bool isValid() const {
        return !trackId.isEmpty() && !title.isEmpty() && !artist.isEmpty();
    }
};

class MusicMatcherClient : public QObject {
    Q_OBJECT

  public:
    explicit MusicMatcherClient(QObject* parent = nullptr);
    ~MusicMatcherClient() override;

    void findSimilar(const QString& artist, const QString& title, int limit = 10);
    QList<MusicMatcherSuggestion> suggestions() const {
        return m_suggestions;
    }

  signals:
    void suggestionsReady(const QList<mixxx::MusicMatcherSuggestion>& suggestions);
    void searchFailed(const QString& error);

  private slots:
    void slotSearchFinished();
    void slotTimeout();

  private:
    void searchTrack(const QString& artist, const QString& title);
    void fetchTrackRadio(const QString& trackId);
    void searchArtist(const QString& query);
    void fetchArtistTopTracks(const QString& artistId);

    QNetworkAccessManager* m_pNam;
    QTimer* m_pTimer;
    QList<MusicMatcherSuggestion> m_suggestions;
    QString m_pendingArtistId;
    QString m_originalArtist;
    QString m_originalTitle;
    int m_limit = 10;
};

} // namespace mixxx
