#ifndef AUDIOSCROBBLER_H
#define AUDIOSCROBBLER_H

#include <QNetworkAccessManager>
#include <QObject>

#include "preferences/usersettings.h"

class AutoDJCratesDAO;

class CAudioscrobbler : public QObject {
    typedef enum { good,
        well,
        poor } qual_t;

  public:
    CAudioscrobbler();
    ~CAudioscrobbler();

    void search_similar_artists(const QString& sArtist,
            AutoDJCratesDAO* dao,
            UserSettingsPointer pConfig);

    bool isWorking() {
        return m_bWorking;
    }
    int getResult() {
        return m_iResult;
    }

  private:
    void evaluate(QByteArray& data);
    QString legalFilename(QString sArtists);

    QNetworkAccessManager* manager;
    AutoDJCratesDAO* m_autoDjCratesDao;
    UserSettingsPointer m_pConfig;
    int m_iResult;
    volatile bool m_bWorking;
    QString m_sArtist;

  public slots:
    void slotReplyFinished(QNetworkReply* reply);
};

#endif // AUDIOSCROBBLER_H
