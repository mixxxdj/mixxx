#include "audioscrobbler.h"

#include <QApplication>
#include <QByteArray>
#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QList>
#include <QNetworkReply>

#include "library/dao/autodjcratesdao.h"

/* Large parts of this code are copied from the https://sayonara-player.com/ project */

/*
https://www.last.fm/api

API account created
Here are the details of your new API account.
Application name 	Mixxx
API key 	29abea817440bba143565ea6528d5a7d
Shared secret 	4599291386deee42e93b7a24600bcb37
Registered to 	Limag1
*/

#define LFM_API_KEY QByteArray("29abea817440bba143565ea6528d5a7d")
#define LFM_API_SECRET QByteArray("4599291386deee42e93b7a24600bcb37")

CAudioscrobbler::CAudioscrobbler()
        : m_iResult(0) {
    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &CAudioscrobbler::slotReplyFinished);
}

CAudioscrobbler::~CAudioscrobbler() {
    delete manager;
}

void CAudioscrobbler::search_similar_artists(const QString& sArtist,
        AutoDJCratesDAO* dao,
        UserSettingsPointer pConfig) {
    qInfo() << "search_similar_artists:" << sArtist;
    m_autoDjCratesDao = dao;
    m_pConfig = pConfig;
    m_sArtist = sArtist;
    m_iResult = 0;
    m_bWorking = true;

    QFile f(legalFilename(m_sArtist));

    if (!f.open(QIODevice::ReadOnly)) {
        qInfo() << "Could not open " << legalFilename(m_sArtist) << ", try audioscrobbler....";

        QString url = QString("http://ws.audioscrobbler.com/2.0/?");
        QString encoded = QUrl::toPercentEncoding(sArtist);
        url += QString("method=artist.getsimilar&");
        url += QString("artist=") + encoded + QString("&");
        url += QString("api_key=") + LFM_API_KEY;
        qWarning() << "URL:" << url;

        QNetworkRequest request;

#if 0
         QSslConfiguration config = QSslConfiguration::defaultConfiguration();
         config.setProtocol(QSsl::TlsV1_2);
         request.setSslConfiguration(config);
#endif
        request.setUrl(QUrl(url));
        request.setHeader(QNetworkRequest::ServerHeader, "application/json");

        manager->get(request);
    } else {
        qInfo() << "Opened " << legalFilename(m_sArtist) << ", do it offline....";
        QByteArray data = qUncompress(f.readAll());
        f.close();
        evaluate(data);
    }
}

void CAudioscrobbler::slotReplyFinished(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        qCritical() << "CAudioscrobbler slotReplyFinished:" << reply->error();
        m_bWorking = false;
        return;
    }

    qInfo() << "CAudioscrobbler slotReplyFinished successful.";

    /*
     * Parse Result
     */
    QByteArray data = reply->readAll();

    /*
     * Save result for next time
     */
    QFile f(legalFilename(m_sArtist));
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "Could not write '" << legalFilename(m_sArtist) << "'";
    } else {
        f.write(qCompress(data));
        f.close();
    }

    evaluate(data);
}

QString CAudioscrobbler::legalFilename(QString sArtists) {
    QString sArtistNew = "";
    for (int i = 0; i < sArtists.length(); i++) {
        QChar a = sArtists[i];
        if (a == '&' || a == '>' || a == '<' || a == '|' || a == '\\' || a == '/')
            continue;

        sArtistNew += a;
    }

    QString sPath = m_pConfig->getSettingsPath() + "/similar_artists/";
    QDir dir = sPath;
    dir.mkpath(sPath);

    return dir.filePath(sArtistNew + ".comp");
}

void CAudioscrobbler::evaluate(QByteArray& data) {
    QMap<QString, qual_t> binArtists; //Name, match-score

    QDomDocument doc("similar_artists");
    if (!doc.setContent(data)) {
        qWarning() << "Cannot parse similar artists document";
        m_bWorking = false;
        return;
    }

    QDomElement docElement = doc.documentElement();
    QDomNode similar_artists = docElement.firstChild(); // similarartists

    if (!similar_artists.hasChildNodes()) {
        m_bWorking = false;
        return;
    }

    QString artist_name, mbid;
    double match = -1.0;

    QDomNodeList child_nodes = similar_artists.childNodes();
    for (int idx_artist = 0; idx_artist < child_nodes.size(); idx_artist++) {
        QDomNode artist = child_nodes.item(idx_artist);
        QString node_name = artist.nodeName();

        if (node_name.compare("artist", Qt::CaseInsensitive) != 0) {
            continue;
        }

        if (!artist.hasChildNodes()) {
            continue;
        }

        QDomNodeList artist_child_nodes = artist.childNodes();
        for (int idx_content = 0; idx_content < artist_child_nodes.size(); idx_content++) {
            QDomNode content = artist_child_nodes.item(idx_content);
            QString node_name = content.nodeName().toLower();
            QDomElement e = content.toElement();

            if (node_name.compare("name") == 0) {
                if (!e.isNull()) {
                    artist_name = e.text();
                }
            } else if (node_name.compare("match") == 0) {
                if (!e.isNull()) {
                    match = e.text().toDouble();
                }
            } else if (node_name.compare("mbid") == 0) {
                if (!e.isNull()) {
                    mbid = e.text();
                }
            }

            if (!artist_name.isEmpty() && match > 0 && !mbid.isEmpty()) {
                if (match > 0.15)
                    binArtists.insert(artist_name, good);
                else if (match > 0.05)
                    binArtists.insert(artist_name, well);
                else
                    binArtists.insert(artist_name, poor);

                artist_name = "";
                match = -1.0;
                mbid = "";
                break;
            }
        }
    }

    if (binArtists.size() == 0) {
        m_bWorking = false;
        return;
    }

    /*
     * Evaluate match binArtists, if we always take the best, it's boring
     */
    qual_t quality, quality_org;
    int rnd_number = (rand() % 999) + 1; //1, 999

    if (rnd_number > 350) {
        quality = good; // [250-999]
    } else if (rnd_number > 75) {
        quality = well; // [50-250]
    } else {
        quality = poor;
    }

    quality_org = quality;
    QList<QString> possible_artists;

    while (possible_artists.isEmpty()) {
        QMapIterator<QString, qual_t> it(binArtists);
        while (it.hasNext()) {
            it.next();
            if (it.value() != quality)
                continue;

            qApp->processEvents();
            if (m_autoDjCratesDao->existsArtist(it.key()))
                possible_artists.append(it.key());
        }

        switch (quality) {
        case poor:
            quality = good;
            break;
        case well:
            quality = poor;
            break;
        case good:
            quality = well;
            break;
        default: // will never be executed
            quality = quality_org;
            break;
        }

        if (quality == quality_org) {
            break;
        }
    }

    if (possible_artists.isEmpty()) {
        m_bWorking = false;
        return;
    }

    // Choose artist and its track with little random
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(possible_artists.begin(), possible_artists.end(), g);

    for (auto it = possible_artists.begin(); it != possible_artists.end(); it++) {
        qApp->processEvents();
        QList<int> liTracks = m_autoDjCratesDao->getAllUnplayedTracks(*it);
        if (liTracks.size() == 0)
            continue;

        int rnd_track = rand() % liTracks.size();
        m_iResult = liTracks.takeAt(rnd_track);
        m_bWorking = false;
        return;
    }

    m_bWorking = false; //can't happen
}
