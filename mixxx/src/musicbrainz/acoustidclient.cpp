// Thanks to Clementine
#include <QCoreApplication>
#include <QNetworkReply>
#include <QXmlStreamReader>

#include "acoustidclient.h"
#include "gzip.h"
#include "network.h"

// TODO(kain88) get a unique API-KEY for mixxx
const QString AcoustidClient::m_ClientId = "ErlAvPUB";
const QString AcoustidClient::m_Url = "http://api.acoustid.org/v2/lookup";
const QString AcoustidClient::m_submitUrl = "http://api.acoustid.org/v2/submit";
const int AcoustidClient::m_DefaultTimeout = 5000; // msec

AcoustidClient::AcoustidClient(QObject* parent)
              : QObject(parent),
                m_network(this),
                m_timeouts(m_DefaultTimeout, this){
}

void AcoustidClient::SetTimeout(int msec) {
    m_timeouts.SetTimeout(msec);
}

void AcoustidClient::Start(int id, const QString& fingerprint, int duration) {
    
    QUrl url;
    url.addQueryItem("format", "xml");
    url.addQueryItem("client", m_ClientId);
    url.addQueryItem("duration", QString::number(duration));
    url.addQueryItem("meta", "recordingids");
    url.addQueryItem("fingerprint", fingerprint);

    QNetworkRequest req(QUrl::fromEncoded(m_Url.toAscii()));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    req.setRawHeader("Content-Encoding", "gzip");
    req.setRawHeader("User-Agent", QString("testApp_kain880.01").toAscii());
    QNetworkReply* reply = m_network.post(req, gzipCompress(url.encodedQuery()));
    connect(reply, SIGNAL(finished()), SLOT(RequestFinished()));
    m_requests[reply] = id;
    
    m_timeouts.AddReply(reply);
}

void AcoustidClient::Submit(int id,
                            const QString& fingerprint,
                            const QString& apiKey,
                            TrackPointer track) {

    qDebug() << "submit fingerprint";

    QUrl url;
    url.addQueryItem("user", apiKey);
    url.addQueryItem("client", m_ClientId);
    url.addQueryItem("format", "xml");
    // add information to submit
    url.addQueryItem("duration", QString::number(track->getDuration()));
    url.addQueryItem("track", track->getTitle());
    url.addQueryItem("artist", track->getArtist());
    url.addQueryItem("album", track->getAlbum());
    url.addQueryItem("year", track->getYear());
    url.addQueryItem("trackno", track->getTrackNumber());
    url.addQueryItem("fileformat", track->getType());
    url.addQueryItem("bitrate", track->getBitrateStr());

    url.addQueryItem("fingerprint", fingerprint);
    // qDebug()<<url.toString();

    QNetworkRequest req(QUrl::fromEncoded("http://api.acoustid.org/v2/submit"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    req.setRawHeader("Content-Encoding", "gzip");
    req.setRawHeader("User-Agent", QString("testApp_kain880.01").toAscii());
    QNetworkReply* reply = m_network.post(req, gzipCompress(url.encodedQuery()));
    connect(reply, SIGNAL(finished()), SLOT(SubmitFinished()));
    m_requests[reply] = id;
    m_timeouts.AddReply(reply);
}

void AcoustidClient::Cancel(int id) {
    QNetworkReply* reply = m_requests.key(id);
    m_requests.remove(reply);
    delete reply;
}

void AcoustidClient::CancelAll() {
    qDeleteAll(m_requests.keys());
    m_requests.clear();
}

void AcoustidClient::SubmitFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    // qDebug() << "submitfinished parse result";
    if (!reply)
        return;

    reply->deleteLater();
    if (!m_requests.contains(reply)){
        qDebug() << "unkown reply";
        return;
    }

    int id = m_requests.take(reply);

    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200) {
        // qDebug() << "error from network received:"<<
                 // reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        emit submited(id, tr("Network Error: ") + reply->attribute(
                      QNetworkRequest::HttpStatusCodeAttribute).toString());
        return;
    }

    QXmlStreamReader reader(reply);
    QString status;
    while (!reader.atEnd()) {
        if (reader.readNext() == QXmlStreamReader::StartElement 
            && reader.name()== "status") {
                status = reader.readElementText();
            }
    }

    if (status == "ok") {
        emit submited(id, tr("Metadata submited to AcoustID Server"));
    } else {
        emit submited(id, tr("AcoustID status: ") + status);
    }
}

void AcoustidClient::RequestFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply)
        return;
    
    reply->deleteLater();
    if (!m_requests.contains(reply))
        return;
    
    int id = m_requests.take(reply);
    
    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200) {
        emit Finished(id, QString());
        return;
    }

    QXmlStreamReader reader(reply);
    QString ID;
    while (!reader.atEnd()) {
        if (reader.readNext() == QXmlStreamReader::StartElement 
            && reader.name()== "results") {
                ID = ParseResult(reader);
            }
    }

    // qDebug() << "found something " << ID;
    emit Finished(id, ID);
}

QString AcoustidClient::ParseResult(QXmlStreamReader& reader){

    while (!reader.atEnd()) {
        QXmlStreamReader::TokenType type = reader.readNext();

        if (type== QXmlStreamReader::StartElement) {
            QStringRef name = reader.name();

            if (name == "id") {
                return reader.readElementText();
            }
        }

        if (type == QXmlStreamReader::EndElement && reader.name()=="result") {
            break;
        }

    }
    return QString();
}
