#include <QtCore>
#include <QtGui>
#include <QtSql>
#include <QtDebug>
#include <QtXmlPatterns/QXmlQuery>

#include "rhythmboxtrackmodel.h"
#include "xmlparse.h"
#include "trackinfoobject.h"
#include "defs.h"
#include "defs_audiofiles.h"

RhythmboxTrackModel::RhythmboxTrackModel()
        : AbstractXmlTrackModel("mixxx.db.model.rhythmbox") {

    QXmlQuery query;
    QString res;
    QDomDocument rhythmdb;


    /*
     * Try and open the Rhythmbox DB. An API call which tells us where
     * the file is would be nice.
     */
    QFile db(QDir::homePath() + "/.gnome2/rhythmbox/rhythmdb.xml");
    if ( ! db.exists()) {
        db.setFileName(QDir::homePath() + "/.local/share/rhythmbox/rhythmdb.xml");
        if ( ! db.exists())
            return;
    }

    if (!db.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    /*
     * Use QXmlQuery to execute an XPath query. We add the version to
     * the XPath query to make sure it is the schema we expect.
     */
    query.setFocus(&db);
    query.setQuery("rhythmdb[@version='1.6']/entry[@type='song']");
    if ( ! query.isValid())
        return;

    query.evaluateTo(&res);
    db.close();


    /*
     * Parse the result as an XML file. These shennanigans actually
     * reduce the load time from a minute to a matter of seconds.
     */
    rhythmdb.setContent("<rhythmdb version='1.6'>" + res + "</rhythmdb>");
    m_trackNodes = rhythmdb.elementsByTagName("entry");


    for (int i = 0; i < m_trackNodes.count(); i++) {
        QDomNode n = m_trackNodes.at(i);
        QString location = n.firstChildElement("location").text();

        m_mTracksByLocation[location] = n;
    }
    qDebug() << "RhythmboxTrackModel: m_entryNodes size is" << m_trackNodes.size();


    addColumnName(RhythmboxTrackModel::COLUMN_ARTIST, tr("Artist"));
    addColumnName(RhythmboxTrackModel::COLUMN_TITLE, tr("Title"));
    addColumnName(RhythmboxTrackModel::COLUMN_ALBUM, tr("Album"));
    addColumnName(RhythmboxTrackModel::COLUMN_DATE, tr("Date"));
    addColumnName(RhythmboxTrackModel::COLUMN_GENRE, tr("Genre"));
    addColumnName(RhythmboxTrackModel::COLUMN_LOCATION, tr("Location"));
    addColumnName(RhythmboxTrackModel::COLUMN_DURATION, tr("Duration"));

    addSearchColumn(RhythmboxTrackModel::COLUMN_ARTIST);
    addSearchColumn(RhythmboxTrackModel::COLUMN_TITLE);
    addSearchColumn(RhythmboxTrackModel::COLUMN_ALBUM);
    addSearchColumn(RhythmboxTrackModel::COLUMN_GENRE);
    addSearchColumn(RhythmboxTrackModel::COLUMN_LOCATION);
}

RhythmboxTrackModel::~RhythmboxTrackModel()
{

}

QItemDelegate* RhythmboxTrackModel::delegateForColumn(const int i) {
    return NULL;
}

QVariant RhythmboxTrackModel::data(const QModelIndex& item, int role) const {
    if (!item.isValid())
        return QVariant();

    QVariant value = AbstractXmlTrackModel::data(item, role);

    if (role == Qt::DisplayRole &&
        item.column() == COLUMN_DURATION) {

        if (qVariantCanConvert<int>(value)) {
            // TODO(XXX) Pull this out into a MixxxUtil or something.

            //Let's reformat this song length into a human readable MM:SS format.
            int totalSeconds = qVariantValue<int>(value);
            int seconds = totalSeconds % 60;
            int mins = totalSeconds / 60;
            //int hours = mins / 60; //Not going to worry about this for now. :)

            //Construct a nicely formatted duration string now.
            value = QString("%1:%2").arg(mins).arg(seconds, 2, 10, QChar('0'));
        }
    }
    return value;
}

bool RhythmboxTrackModel::isColumnInternal(int column) {
    return false;
}

QVariant RhythmboxTrackModel::getTrackColumnData(QDomNode songNode, const QModelIndex& index) const
{
    int date;
    switch (index.column()) {
        case RhythmboxTrackModel::COLUMN_ARTIST:
            return songNode.firstChildElement("artist").text();
        case RhythmboxTrackModel::COLUMN_TITLE:
            return songNode.firstChildElement("title").text();
        case RhythmboxTrackModel::COLUMN_ALBUM:
            return songNode.firstChildElement("album").text();
        case RhythmboxTrackModel::COLUMN_DATE:
            date = songNode.firstChildElement("date").text().toInt();
            // Rhythmbox uses the Rata Die Julian Day system while Qt uses true
            // Julian Days. It's just a difference in epoch start dates.
            if (date == 0)
                return "";
            return QDate::fromJulianDay(ceil(float(date) + 1721424.5)).year();
        case RhythmboxTrackModel::COLUMN_GENRE:
            return songNode.firstChildElement("genre").text();
        case RhythmboxTrackModel::COLUMN_LOCATION:
            return songNode.firstChildElement("location").text();
        case RhythmboxTrackModel::COLUMN_DURATION:
            return songNode.firstChildElement("duration").text();

        default:
            return QVariant();
    }
}

TrackInfoObject *RhythmboxTrackModel::parseTrackNode(QDomNode songNode) const
{
    QString trackLocation = QUrl(songNode.firstChildElement("location").text()).toLocalFile();
    TrackInfoObject *pTrack = new TrackInfoObject(trackLocation);

    pTrack->setArtist(songNode.firstChildElement("artist").text());
    pTrack->setTitle(songNode.firstChildElement("title").text());
    pTrack->setAlbum(songNode.firstChildElement("album").text());
    int date = songNode.firstChildElement("date").text().toInt();
    // Rhythmbox uses the Rata Die Julian Day system while Qt uses true
    // Julian Days. It's just a difference in epoch start dates.
    if (date == 0) {
        pTrack->setYear("");
    } else {
        date = QDate::fromJulianDay(ceil(float(date) + 1721424.5)).year();
        pTrack->setYear(QString("%1").arg(date));
    }
    pTrack->setGenre(songNode.firstChildElement("genre").text());
    pTrack->setDuration(songNode.firstChildElement("duration").text().toUInt());
    pTrack->setLocation();

    return pTrack;
}
