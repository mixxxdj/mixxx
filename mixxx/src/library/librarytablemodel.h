#ifndef LIBRARYTABLEMODEL_H
#define LIBRARYTABLEMODEL_H

#include <QtSql>
#include <QtCore>
#include "trackmodel.h"

class TrackCollection;

const QString LIBRARYTABLE_ID = "id";
const QString LIBRARYTABLE_ARTIST = "artist";
const QString LIBRARYTABLE_TITLE = "title";
const QString LIBRARYTABLE_ALBUM = "album";
const QString LIBRARYTABLE_YEAR = "year";
const QString LIBRARYTABLE_GENRE = "genre";
const QString LIBRARYTABLE_TRACKNUMBER = "tracknumber";
const QString LIBRARYTABLE_LOCATION = "location";
const QString LIBRARYTABLE_FILENAME = "filename";
const QString LIBRARYTABLE_COMMENT = "comment";
const QString LIBRARYTABLE_DURATION = "duration";
const QString LIBRARYTABLE_BITRATE = "bitrate";
const QString LIBRARYTABLE_BPM = "bpm";
const QString LIBRARYTABLE_LENGTHINBYTES = "length_in_bytes";
const QString LIBRARYTABLE_CUEPOINT = "cuepoint";
const QString LIBRARYTABLE_URL = "url";
const QString LIBRARYTABLE_SAMPLERATE = "samplerate";
const QString LIBRARYTABLE_WAVESUMMARYHEX = "wavesummaryhex";
const QString LIBRARYTABLE_CHANNELS = "channels";


class LibraryTableModel : public QSqlTableModel, public virtual TrackModel
{
public:
    LibraryTableModel(QObject* parent, TrackCollection* pTrackCollection);
    virtual ~LibraryTableModel();
    virtual TrackInfoObject* getTrack(const QModelIndex& index) const;
    virtual QString getTrackLocation(const QModelIndex& index) const;
    virtual void search(const QString& searchText);
    virtual void removeTrack(const QModelIndex& index);
    virtual void addTrack(const QModelIndex& index, QString location);
    QMimeData* mimeData(const QModelIndexList &indexes) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
private:
    TrackCollection* m_pTrackCollection;
};

#endif
