#include <QtCore>
#include <QtGui>
#include <QtSql>
#include <QtDebug>
#include <QSettings>
#include <QRegExp>
#include <QtXmlPatterns/QXmlQuery>

#include "itunestrackmodel.h"
#include "xmlparse.h"
#include "trackinfoobject.h"
#include "defs.h"
#include "soundsourceproxy.h"

#include "mixxxutils.cpp"

ITunesTrackModel::ITunesTrackModel()
        : AbstractXmlTrackModel("mixxx.db.model.itunes") {
    QXmlQuery query;
    QString res, playlistRes;
    QDomDocument itunesdb;

    QRegExp supportedFileRegex(SoundSourceProxy::supportedFileExtensionsRegex(),
                               Qt::CaseInsensitive);

    QString itunesXmlPath = getiTunesMusicPath();

    QFile db(itunesXmlPath);
    if (!db.exists())
        return;

    if (!db.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    // Workaround for Bug #501916. Read the file, convert it to UTF-8 and then
    // load it.
    QByteArray db_bytes_utf8 = QString(db.readAll()).toUtf8();
    QBuffer buffer(&db_bytes_utf8);
    if (!buffer.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    /*
     * Use QXmlQuery to execute an XPath query. We add the version to
     * the XPath query to make sure it is the schema we expect.
     *
     * TODO: filter /key='Track Type'/string='URL' (remote) files
     */
    query.setFocus(&buffer);
    query.setQuery("/plist[@version='1.0']/dict[key='Tracks']/dict/dict");
    if ( ! query.isValid())
        return;

    query.evaluateTo(&res);

    // Both ITunes and Rhythmbox parsing should occur in something completely
    // separate from the TrackModels, but since we're in a rush, we're just
    // going to do playlist parsing here so we don't have to re-open the file
    // again.
    query.setQuery("/plist[@version='1.0']/dict[key='Playlists']/array/dict");
    if (query.isValid()) {
        query.evaluateTo(&playlistRes);
    }
    db.close();

    /*
     * Parse the result as an XML file. These shennanigans actually
     * reduce the load time from a minute to a matter of seconds.
     */
    itunesdb.setContent("<plist version='1.0'>" + res + "</plist>");
    m_trackNodes = itunesdb.elementsByTagName("dict");

    for (int i = 0; i < m_trackNodes.count(); i++) {
        QDomNode n = m_trackNodes.at(i);
        QString trackId = findValueByKey(n, "Track ID");
        QString location = findValueByKey(n, "Location");

        // Skip files we cannot play.
        if (location.count(supportedFileRegex) == 0) {
            continue;
        }

        m_mTracksByLocation[location] = n;
        m_mTracksById[trackId] = n;
    }

    // Now process the playlist data.
    QDomDocument playlistdb;
    playlistdb.setContent("<plist version='1.0'>" + playlistRes + "</plist>");
    QDomNodeList playlistNodes = playlistdb.documentElement().childNodes();
    for (int i = 0; i < playlistNodes.count(); i++) {
        QDomNode n = playlistNodes.at(i);

        // Get the playlist name
        QString name = findValueByKey(n, "Name");
        qDebug() << "Found playlist" << name;

        // Skip invisible playlists
        QDomElement visible = findNodeByKey(n, "Visible");
        if (!visible.isNull() && visible.tagName() == "false") {
            continue;
        }

        // Now traverse to the items list
        QDomElement items = findNodeByKey(n, "Playlist Items");
        if (items.isNull()) {
            qDebug() << name << "has no items";
            continue;
        }

        // Now extract the song ids that are members of this playlist
        QDomNodeList playlistEntries = items.childNodes();
        QList<QString> playlistSongIds;
        for (int j = 0; j < playlistEntries.count(); j++) {
            QDomNode entry = playlistEntries.at(j);
            QString trackId = findValueByKey(entry, "Track ID");

            // If the track index does not contain the given track, that means
            // either the track is not playable in Mixxx, or the XML is
            // inconsistent. In this case, don't show the track in the playlist.
            if (!m_mTracksById.contains(trackId)) {
                continue;
            }

            playlistSongIds.append(trackId);
        }

        // If there were no playable items in the playlist, don't show it.
        if (playlistSongIds.count() == 0) {
            qDebug() << name << "has no items";
            continue;
        }

        // TODO(XXX) : Do we need to handle duplicate playlist names? If there
        // are duplicates, only one will show up if we do this.
        m_mPlaylists[name] = playlistSongIds;
    }

    qDebug() << "ITunesTrackModel: m_entryNodes size is" << m_trackNodes.size();

    addColumnName(ITunesTrackModel::COLUMN_ARTIST, "Artist");
    addColumnName(ITunesTrackModel::COLUMN_TITLE, "Title");
    addColumnName(ITunesTrackModel::COLUMN_ALBUM, "Album");
    addColumnName(ITunesTrackModel::COLUMN_DATE, "Date");
    addColumnName(ITunesTrackModel::COLUMN_BPM, "BPM");
    addColumnName(ITunesTrackModel::COLUMN_GENRE, "Genre");
    addColumnName(ITunesTrackModel::COLUMN_LOCATION, "Location");
    addColumnName(ITunesTrackModel::COLUMN_DURATION, "Duration");

    addSearchColumn(ITunesTrackModel::COLUMN_ARTIST);
    addSearchColumn(ITunesTrackModel::COLUMN_TITLE);
    addSearchColumn(ITunesTrackModel::COLUMN_ALBUM);
    addSearchColumn(ITunesTrackModel::COLUMN_GENRE);
    addSearchColumn(ITunesTrackModel::COLUMN_LOCATION);
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

QDomElement ITunesTrackModel::findNodeByKey(QDomNode dictNode, QString key) const
{
    QDomElement curElem;

    curElem = dictNode.firstChildElement("key");
    while(!curElem.isNull()) {
        if ( curElem.text() == key ) {
            return curElem.nextSiblingElement();
        }
        curElem = curElem.nextSiblingElement("key");
    }

    return QDomElement();
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
        case ITunesTrackModel::COLUMN_BPM:
            return findValueByKey(songNode,"BPM");
        case ITunesTrackModel::COLUMN_GENRE:
            return findValueByKey(songNode,"Genre");
        case ITunesTrackModel::COLUMN_LOCATION: {
            /*
             * Strip the crappy file://localhost/ from the URL and
             * format URL as in method ITunesTrackModel::parseTrackNode(QDomNode songNode)
             */
            QString strloc = findValueByKey(songNode,"Location");
            QByteArray strlocbytes = strloc.toUtf8();
            QString location = QUrl::fromEncoded(strlocbytes).toLocalFile();
#if defined(__WINDOWS__)
            return location.remove("//localhost/");
#else
            return location.remove("//localhost");
#endif
        }

        case ITunesTrackModel::COLUMN_DURATION:
            value = findValueByKey(songNode,"Total Time");
            if (qVariantCanConvert<int>(value)) {
                value = MixxxUtils::millisecondsToMinutes(qVariantValue<int>(value));
            }
            return value;
        default:
            return QVariant();
    }
}

bool ITunesTrackModel::isColumnInternal(int column) {
    return false;
}

TrackPointer ITunesTrackModel::parseTrackNode(QDomNode songNode) const
{
    QString strloc = findValueByKey(songNode,"Location");
    QByteArray strlocbytes = strloc.toUtf8();
    QUrl location = QUrl::fromEncoded(strlocbytes);

    QString trackLocation;
    //Strip the crappy localhost from the URL since Qt barfs on this :(
#if defined(__WINDOWS__)
    trackLocation = location.toLocalFile().remove("//localhost/");
#else
    trackLocation = location.toLocalFile().remove("//localhost");
#endif
    //pTrack->setLocation(QUrl(findValueByKey(songNode,"Location")).toLocalFile());

    TrackInfoObject* pTrack = new TrackInfoObject(trackLocation);

    pTrack->setArtist(findValueByKey(songNode, "Artist"));
    pTrack->setTitle(findValueByKey(songNode, "Name"));
    pTrack->setAlbum(findValueByKey(songNode,"Album"));
    pTrack->setYear(findValueByKey(songNode,"Year"));
    pTrack->setGenre(findValueByKey(songNode,"Genre"));
    pTrack->setBpm(findValueByKey(songNode,"BPM").toFloat());

    // ITunes stores time in total milliseconds
    pTrack->setDuration(findValueByKey(songNode,"Total Time").toInt() / 1000);

    // Let Qt handle deleting the track since it isn't owned by the library.
    return TrackPointer(pTrack, &QObject::deleteLater);
}

TrackPointer ITunesTrackModel::getTrackById(QString id) {
    if (!m_mTracksById.contains(id)) {
        return TrackPointer();
    }
    return parseTrackNode(m_mTracksById[id]);
}

QString ITunesTrackModel::getiTunesMusicPath() {
    QString musicFolder;
#if defined(__APPLE__)
		musicFolder = QDir::homePath() + "/Music/iTunes/iTunes Music Library.xml";
#elif defined(__WINDOWS__)
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", QSettings::NativeFormat);
		// if the value method fails it returns QTDir::homePath
    musicFolder = settings.value("My Music", QDir::homePath()).toString();
    musicFolder += "\\iTunes\\iTunes Music Library.xml";
#elif defined(__LINUX__)
		musicFolder =  QDir::homePath() + "/.itunes.xml";
#else
		musicFolder = "";
#endif
    qDebug() << "ITunesLibrary=[" << musicFolder << "]";
    return musicFolder;
}
