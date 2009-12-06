#include <QtCore>
#include <QtGui>
#include <QtSql>
#include <QtDebug>
#include <QtXmlPatterns/QXmlQuery>

#include "itunestrackmodel.h"
#include "xmlparse.h"
#include "trackinfoobject.h"
#include "defs.h"
#include "defs_audiofiles.h"

ITunesTrackModel::ITunesTrackModel()
{
    QXmlQuery query;
    QString res;
    QDomDocument itunesdb;


    /*
     * Try and open the ITunes DB. An API call which tells us where
     * the file is would be nice.
     */
    QString itunesXmlPath;
    itunesXmlPath = MIXXX_ITUNES_DB_LOCATION;

    QFile db(itunesXmlPath);
    if ( ! db.exists())
        return;

    if (!db.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    /*
     * Use QXmlQuery to execute an XPath query. We add the version to
     * the XPath query to make sure it is the schema we expect.
     *
     * TODO: filter /key='Track Type'/string='URL' (remote) files
     */
    query.setFocus(&db);
    query.setQuery("/plist[@version='1.0']/dict[key='Tracks']/dict/dict");
    if ( ! query.isValid())
        return;

    query.evaluateTo(&res);
    db.close();


    /*
     * Parse the result as an XML file. These shennanigans actually
     * reduce the load time from a minute to a matter of seconds.
     */
    itunesdb.setContent("<plist version='1.0'>" + res + "</plist>");
    m_trackNodes = itunesdb.elementsByTagName("dict");


    for (int i = 0; i < m_trackNodes.count(); i++) {
        QDomNode n = m_trackNodes.at(i);
        QString location = findValueByKey(n, "Location");

        m_mTracksByLocation[location] = n;
    }

    qDebug() << itunesdb.doctype().name();
    qDebug() << "ITunesTrackModel: m_entryNodes size is" << m_trackNodes.size();


    addColumnName(ITunesTrackModel::COLUMN_ARTIST, "Artist");
    addColumnName(ITunesTrackModel::COLUMN_TITLE, "Title");
    addColumnName(ITunesTrackModel::COLUMN_ALBUM, "Album");
    addColumnName(ITunesTrackModel::COLUMN_DATE, "Date");
    addColumnName(ITunesTrackModel::COLUMN_GENRE, "Genre");
    addColumnName(ITunesTrackModel::COLUMN_LOCATION, "Location");
    addColumnName(ITunesTrackModel::COLUMN_DURATION, "Duration");

    addSearchColumn(ITunesTrackModel::COLUMN_ARTIST);
    addSearchColumn(ITunesTrackModel::COLUMN_TITLE);
    addSearchColumn(ITunesTrackModel::COLUMN_ALBUM);
    addSearchColumn(ITunesTrackModel::COLUMN_GENRE);
    addSearchColumn(ITunesTrackModel::COLUMN_LOCATION);

    for (int i = 0; i < m_trackNodes.count(); i++) {
        QDomNode n;


        n = m_trackNodes.at(i);
        qDebug() << "Track=[" << i << "]"
                    << "Artist:" << findValueByKey(n, "Artist")
                    << "Title:" << findValueByKey(n, "Name");
    }
}

ITunesTrackModel::~ITunesTrackModel()
{

}

QItemDelegate* ITunesTrackModel::delegateForColumn(const int i)
{
    return NULL;
}

QString ITunesTrackModel::findValueByKey(QDomNode dictNode, QString key) const
{
    QDomElement curElem;


    curElem = dictNode.firstChildElement("key");
    while(!curElem.isNull()) {
        if ( curElem.text() == key ) {
            QDomElement value;


            value = curElem.nextSiblingElement();
            return value.text();
        }

        curElem = curElem.nextSiblingElement("key");
    }

    return QString();
}

QVariant ITunesTrackModel::getTrackColumnData(QDomNode songNode, const QModelIndex& index) const
{
    QVariant value;

    switch (index.column()) {
        case ITunesTrackModel::COLUMN_ARTIST:
            return findValueByKey(songNode, "Artist");
        case ITunesTrackModel::COLUMN_TITLE:
            return findValueByKey(songNode, "Name");
        case ITunesTrackModel::COLUMN_ALBUM:
            return findValueByKey(songNode,"Album");
        case ITunesTrackModel::COLUMN_DATE:
            return findValueByKey(songNode,"Year");
        case ITunesTrackModel::COLUMN_GENRE:
            return findValueByKey(songNode,"Genre");
        case ITunesTrackModel::COLUMN_LOCATION:
            return findValueByKey(songNode,"Location");
        case ITunesTrackModel::COLUMN_DURATION:
            value = findValueByKey(songNode,"Total Time");
            if (qVariantCanConvert<int>(value)) {
                // TODO(XXX) Pull this out into a MixxxUtil or something.
                //XXX: Code based on LibraryTableModel::data(...) but slightly different
                //Let's reformat this song length into a human readable MM:SS format.
                int totalMilliseconds = qVariantValue<int>(value);
                int seconds = (totalMilliseconds % 60000) / 1000;
                int mins = totalMilliseconds / 60000;
                //int hours = mins / 60; //Not going to worry about this for now. :)

                //Construct a nicely formatted duration string now.
                value = QString("%1:%2").arg(mins).arg(seconds, 2, 10, QChar('0'));
            }
            return value;

        default:
            return QVariant();
    }
}

bool ITunesTrackModel::isColumnInternal(int column) {
    return false;
}

TrackInfoObject *ITunesTrackModel::parseTrackNode(QDomNode songNode) const
{
    TrackInfoObject *pTrack = new TrackInfoObject();


    pTrack->setArtist(findValueByKey(songNode, "Artist"));
    pTrack->setTitle(findValueByKey(songNode, "Name"));
    pTrack->setAlbum(findValueByKey(songNode,"Album"));
    pTrack->setYear(findValueByKey(songNode,"Year"));
    pTrack->setGenre(findValueByKey(songNode,"Genre"));
    pTrack->setDuration(findValueByKey(songNode,"Total Time").toUInt());

    QString strloc = findValueByKey(songNode,"Location");
    QByteArray strlocbytes = strloc.toUtf8();
    QUrl location = QUrl::fromEncoded(strlocbytes);
    //Strip the crappy localhost from the URL since Qt barfs on this :(
    pTrack->setLocation(location.toLocalFile().remove("//localhost"));
    //pTrack->setLocation(QUrl(findValueByKey(songNode,"Location")).toLocalFile());

    return pTrack;
}

