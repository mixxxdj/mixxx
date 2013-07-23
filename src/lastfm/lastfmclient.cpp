#include <QNetworkReply>
#include <QStringBuilder>

#include "lastfm/lastfmclient.h"
#include "track/tagutils.h"

const int LastFmClient::m_iDefaultTimeout = 5000; // msec
const QString LastFmClient::m_sApiKey = "154ac2038b19ca6d5d3ef109eca3a7f8";
const QString LastFmClient::m_sRestUrl = "http://ws.audioscrobbler.com/2.0/";

LastFmClient::LastFmClient(QObject *parent)
            : QObject(parent),
              m_network(this),
              m_timeouts(m_iDefaultTimeout, this) {
}

void LastFmClient::start(int id, const QString& artist, const QString& title) {
    typedef QPair<QString, QString> Param;

    QList<Param> parameters;
    parameters << Param("method", "track.getTopTags");
    parameters << Param("artist", escapeString(artist));
    parameters << Param("track", escapeString(title));
    parameters << Param("api_key", m_sApiKey);

    QUrl url(m_sRestUrl);
    url.setQueryItems(parameters);
    QNetworkRequest req(url);

//    qDebug() << url;
    qDebug() << "sending request for " << artist << "-" << title;

    QNetworkReply* reply = m_network.get(req);
    connect(reply, SIGNAL(finished()), SLOT(requestFinished()));
    m_requests[reply] = id;

    m_timeouts.addReply(reply);
}


void LastFmClient::requestFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply)
        return;

    reply->deleteLater();
    if (!m_requests.contains(reply))
        return;
    int id = m_requests.take(reply);

    TagCounts ret;
    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()
            != 200) {
        emit finished(id, ret);
        return;
    }

    QXmlStreamReader reader(reply);
    while (!reader.atEnd()) {
        if (reader.readNext() == QXmlStreamReader::StartElement
            && reader.name() == "toptags") {
            ret = parseTopTags(reader);
        }
    }
    emit finished(id, ret);
}


void LastFmClient::cancel(int id) {
    QNetworkReply* reply = m_requests.key(id);
    m_requests.remove(reply);
    delete reply;
}

void LastFmClient::cancelAll() {
    qDeleteAll(m_requests.keys());
    m_requests.clear();
}

QString LastFmClient::escapeString(const QString& string) {
    QString escaped(string);
    escaped.toLower();
    escaped.replace(' ', '+');
    return escaped;
}

TagCounts LastFmClient::parseTopTags(QXmlStreamReader& reader) {
    TagCounts ret;
    QString tag_name;
    int count;

    while (!reader.atEnd()) {
        QXmlStreamReader::TokenType type = reader.readNext();

        if (type == QXmlStreamReader::StartElement) {
            QStringRef name = reader.name();
            if (name == "name") {
                tag_name = reader.readElementText();
            } else if (name == "count") {
                count = reader.readElementText().toInt();

                qDebug() << tag_name << ":" << count;
                ret.insert(tag_name, count);
            }
        }

        if (type == QXmlStreamReader::EndElement && reader.name() == "toptags") {
        break;
        }
    }
    return ret;
}
