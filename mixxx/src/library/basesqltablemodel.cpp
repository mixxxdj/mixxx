// basesqltablemodel.h
// Created by RJ Ryan (rryan@mit.edu) 1/29/2010

#include <QtDebug>

#include "trackinfoobject.h"
#include "library/trackcollection.h"
#include "library/basesqltablemodel.h"
#include "mixxxutils.cpp"
#include "library/starrating.h"


BaseSqlTableModel::BaseSqlTableModel(QObject* parent,
                                     TrackCollection* pTrackCollection,
                                     QSqlDatabase db) :
        QSqlTableModel(parent, db),
        m_pTrackCollection(pTrackCollection),
        m_trackDAO(m_pTrackCollection->getTrackDAO()) {
    connect(&m_trackDAO, SIGNAL(trackChanged(int)),
            this, SLOT(trackChanged(int)));
    m_iSortColumn = 0;
    m_eSortOrder = Qt::AscendingOrder;
}

BaseSqlTableModel::~BaseSqlTableModel() {

}

void BaseSqlTableModel::initHeaderData() {
    //Set the column heading labels, rename them for translations and have
    //proper capitalization
    setHeaderData(fieldIndex(LIBRARYTABLE_TIMESPLAYED),
                  Qt::Horizontal, tr("Played"));
    setHeaderData(fieldIndex(LIBRARYTABLE_ARTIST),
                  Qt::Horizontal, tr("Artist"));
    setHeaderData(fieldIndex(LIBRARYTABLE_TITLE),
                  Qt::Horizontal, tr("Title"));
    setHeaderData(fieldIndex(LIBRARYTABLE_ALBUM),
                  Qt::Horizontal, tr("Album"));
    setHeaderData(fieldIndex(LIBRARYTABLE_GENRE),
                  Qt::Horizontal, tr("Genre"));
    setHeaderData(fieldIndex(LIBRARYTABLE_YEAR),
                  Qt::Horizontal, tr("Year"));
    setHeaderData(fieldIndex(LIBRARYTABLE_FILETYPE),
                  Qt::Horizontal, tr("Type"));
    setHeaderData(fieldIndex(LIBRARYTABLE_LOCATION),
                  Qt::Horizontal, tr("Location"));
    setHeaderData(fieldIndex(LIBRARYTABLE_COMMENT),
                  Qt::Horizontal, tr("Comment"));
    setHeaderData(fieldIndex(LIBRARYTABLE_DURATION),
                  Qt::Horizontal, tr("Duration"));
    setHeaderData(fieldIndex(LIBRARYTABLE_RATING),
                  Qt::Horizontal, tr("Rating"));
    setHeaderData(fieldIndex(LIBRARYTABLE_BITRATE),
                  Qt::Horizontal, tr("Bitrate"));
    setHeaderData(fieldIndex(LIBRARYTABLE_BPM),
                  Qt::Horizontal, tr("BPM"));
    setHeaderData(fieldIndex(LIBRARYTABLE_TRACKNUMBER),
                  Qt::Horizontal, tr("Track #"));
    setHeaderData(fieldIndex(LIBRARYTABLE_DATETIMEADDED),
                  Qt::Horizontal, tr("Date Added"));
    setHeaderData(fieldIndex(PLAYLISTTRACKSTABLE_POSITION),
                  Qt::Horizontal, tr("#"));
}

bool BaseSqlTableModel::select() {
    //qDebug() << "select()";
    bool result = QSqlTableModel::select();
    m_rowToTrackId.clear();
    m_trackIdToRow.clear();

    if (result) {
        // We need to fetch as much data as is available or else the database will
        // be locked.
        while (canFetchMore()) {
            fetchMore();
        }

        // TODO(XXX) let child specify this
        int idColumn = record().indexOf("id");
        for (int row = 0; row < rowCount(); ++row) {
            QModelIndex ind = index(row, idColumn);
            int trackId = QSqlTableModel::data(ind).toInt();
            m_rowToTrackId[row] = trackId;
            m_trackIdToRow[trackId] = row;
        }
    }

    return result;
}

QVariant BaseSqlTableModel::getBaseValue(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    int row = index.row();
    int col = index.column();

    Q_ASSERT(m_rowToTrackId.contains(row));
    if (!m_rowToTrackId.contains(row)) {
        return QSqlTableModel::data(index, role);
    }

    int trackId = m_rowToTrackId[row];

    /*
     * The if-block below is only executed when a table item has been edited.
     *
     */
    if ((role == Qt::DisplayRole || role == Qt::ToolTipRole || role == Qt::EditRole) &&
        m_trackOverrides.contains(trackId)) {
        //qDebug() << "Returning override for track" << trackId;
        TrackPointer pTrack = m_trackDAO.getTrack(trackId);

        // TODO(XXX) Qt properties could really help here.
        if (fieldIndex(LIBRARYTABLE_ARTIST) == col) {
            return QVariant(pTrack->getArtist());
        } else if (fieldIndex(LIBRARYTABLE_TITLE) == col) {
            return QVariant(pTrack->getTitle());
        } else if (fieldIndex(LIBRARYTABLE_ALBUM) == col) {
            return QVariant(pTrack->getAlbum());
        } else if (fieldIndex(LIBRARYTABLE_YEAR) == col) {
            return QVariant(pTrack->getYear());
        } else if (fieldIndex(LIBRARYTABLE_GENRE) == col) {
            return QVariant(pTrack->getGenre());
        } else if (fieldIndex(LIBRARYTABLE_FILETYPE) == col) {
            return QVariant(pTrack->getType());
        } else if (fieldIndex(LIBRARYTABLE_TRACKNUMBER) == col) {
            return QVariant(pTrack->getTrackNumber());
        } else if (fieldIndex(LIBRARYTABLE_LOCATION) == col) {
            return QVariant(pTrack->getLocation());
        } else if (fieldIndex(LIBRARYTABLE_COMMENT) == col) {
            return QVariant(pTrack->getComment());
        } else if (fieldIndex(LIBRARYTABLE_DURATION) == col) {
            return pTrack->getDuration();
        } else if (fieldIndex(LIBRARYTABLE_BITRATE) == col) {
            return QVariant(pTrack->getBitrate());
        } else if (fieldIndex(LIBRARYTABLE_BPM) == col) {
            return QVariant(pTrack->getBpm());
        } else if (fieldIndex(LIBRARYTABLE_PLAYED) == col) {
            return QVariant(pTrack->getPlayed());
        } else if (fieldIndex(LIBRARYTABLE_TIMESPLAYED) == col) {
            return QVariant(pTrack->getTimesPlayed());
        } else if (fieldIndex(LIBRARYTABLE_RATING) == col) {
            return pTrack->getRating();
        }
    }

    // If none of these work, hand off to the lower layer to deal with. The role
    // might not be Edit/Display/ToolTip, or we might have a bug.
    return QSqlTableModel::data(index, role);
}


QVariant BaseSqlTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    int row = index.row();
    int col = index.column();

    //qDebug() << "BaseSqlTableModel::data() column:" << col << "role:" << role;

    // This value is the value in its most raw form. It was looked up either
    // from the SQL table or from the cached track layer.
    QVariant value = getBaseValue(index, role);

    // Format the value based on whether we are in a tooltip, display, or edit
    // role
    if (role == Qt::ToolTipRole || role == Qt::DisplayRole) {
        if (index.column() == fieldIndex(LIBRARYTABLE_DURATION)) {
            if (qVariantCanConvert<int>(value))
                value = MixxxUtils::secondsToMinutes(qVariantValue<int>(value));
        } else if (index.column() == fieldIndex(LIBRARYTABLE_RATING)) {
            if (qVariantCanConvert<int>(value))
                value = qVariantFromValue(StarRating(value.toInt()));
        } else if (index.column() == fieldIndex(LIBRARYTABLE_TIMESPLAYED)) {
            if (qVariantCanConvert<int>(value))
                value =  QString("(%1)").arg(value.toInt());
        } else if (index.column() == fieldIndex(LIBRARYTABLE_PLAYED)) {
            // Convert to a bool. Not really that useful since it gets converted
            // right back to a QVariant
            value = (value == "true") ? true : false;
        } else if (index.column() == fieldIndex(LIBRARYTABLE_LOCATION)) {
			if (value.toString().startsWith(m_sPrefix))
				return value.toString().remove(0, m_sPrefix.size() + 1);
		}
    } else if (role == Qt::EditRole) {
        if (index.column() == fieldIndex(LIBRARYTABLE_BPM)) {
            return value.toDouble();
        } else if (index.column() == fieldIndex(LIBRARYTABLE_TIMESPLAYED)) {
            return index.sibling(index.row(), fieldIndex(LIBRARYTABLE_PLAYED)).data().toBool();
        } else if (index.column() == fieldIndex(LIBRARYTABLE_RATING)) {
            if (qVariantCanConvert<int>(value))
                value = qVariantFromValue(StarRating(value.toInt()));
        }
    } else if (role == Qt::CheckStateRole) {
        if (index.column() == fieldIndex(LIBRARYTABLE_TIMESPLAYED)) {
            bool played = index.sibling(index.row(), fieldIndex(LIBRARYTABLE_PLAYED)).data().toBool();
            value = played ? Qt::Checked : Qt::Unchecked;
        }
    }

    return value;
}

bool BaseSqlTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid())
        return false;

    int row = index.row();
    int col = index.column();

    //qDebug() << "BaseSqlTableModel::setData() column:" << col << "value:" << value << "role:" << role;

    // Over-ride sets to TIMESPLAYED and re-direct them to PLAYED
    if (role == Qt::CheckStateRole) {
        if (index.column() == fieldIndex(LIBRARYTABLE_TIMESPLAYED)) {
            QString val = value.toInt() > 0 ? QString("true") : QString("false");
            QModelIndex playedIndex = index.sibling(index.row(), fieldIndex(LIBRARYTABLE_PLAYED));
            return setData(playedIndex, val, Qt::EditRole);
        }
    }

    Q_ASSERT(m_rowToTrackId.contains(row));
    if (!m_rowToTrackId.contains(row)) {
        return QSqlTableModel::setData(index, value, role);
    }

    int trackId = m_rowToTrackId[row];
    TrackPointer pTrack = m_trackDAO.getTrack(trackId);
    
    // TODO(XXX) Qt properties could really help here.
    if (fieldIndex(LIBRARYTABLE_ARTIST) == col) {
        pTrack->setArtist(value.toString());
    } else if (fieldIndex(LIBRARYTABLE_TITLE) == col) {
        pTrack->setTitle(value.toString());
    } else if (fieldIndex(LIBRARYTABLE_ALBUM) == col) {
        pTrack->setAlbum(value.toString());
    } else if (fieldIndex(LIBRARYTABLE_YEAR) == col) {
        pTrack->setYear(value.toString());
    } else if (fieldIndex(LIBRARYTABLE_GENRE) == col) {
        pTrack->setGenre(value.toString());
    } else if (fieldIndex(LIBRARYTABLE_FILETYPE) == col) {
        pTrack->setType(value.toString());
    } else if (fieldIndex(LIBRARYTABLE_TRACKNUMBER) == col) {
        pTrack->setTrackNumber(value.toString());
    } else if (fieldIndex(LIBRARYTABLE_LOCATION) == col) {
        pTrack->setLocation(value.toString());
    } else if (fieldIndex(LIBRARYTABLE_COMMENT) == col) {
        pTrack->setComment(value.toString());
    } else if (fieldIndex(LIBRARYTABLE_DURATION) == col) {
        pTrack->setDuration(value.toInt());
    } else if (fieldIndex(LIBRARYTABLE_BITRATE) == col) {
        pTrack->setBitrate(value.toInt());
    } else if (fieldIndex(LIBRARYTABLE_BPM) == col) {
        //QVariant::toFloat needs >= QT 4.6.x
        pTrack->setBpm((float) value.toDouble());
    } else if (fieldIndex(LIBRARYTABLE_PLAYED) == col) {
        pTrack->setPlayed(value.toBool());
    } else if (fieldIndex(LIBRARYTABLE_TIMESPLAYED) == col) {
        pTrack->setTimesPlayed(value.toInt());
    } else if (fieldIndex(LIBRARYTABLE_RATING) == col) {
        StarRating starRating = qVariantValue<StarRating>(value);
        pTrack->setRating(starRating.starCount());
    }
    // Do not save the track here. Changing the track dirties it and the caching
    // system will automatically save the track once it is unloaded from
    // memory. rryan 10/2010
    //m_trackDAO.saveTrack(pTrack);
    return true;
}

void BaseSqlTableModel::trackChanged(int trackId) {
    m_trackOverrides.insert(trackId);
    if (m_trackIdToRow.contains(trackId)) {
        int row = m_trackIdToRow[trackId];
        //qDebug() << "Row in this result set was updated. Signalling update. track:" << trackId << "row:" << row;
        QModelIndex left = index(row, 0);
        QModelIndex right = index(row, columnCount());
        emit(dataChanged(left, right));
    }
}

void BaseSqlTableModel::setTable(const QString& tableName) {
    m_qTableName = tableName;
    QSqlTableModel::setTable(tableName);
}

void BaseSqlTableModel::setSort(int column, Qt::SortOrder order) {
    m_iSortColumn = column;
    m_eSortOrder = order;
    QSqlTableModel::setSort(column, order);
}

QString BaseSqlTableModel::orderByClause() const {
    // This is all stolen from QSqlTableModel::orderByClause(), just rigged to
    // sort case-insensitively.
    QString s;
    int sortColumn = 0;
    QSqlField f = record().field(m_iSortColumn);
    if (!f.isValid())
        return s;

    QString table = m_qTableName;
    QString field = database().driver()->escapeIdentifier(f.name(),
                                                           QSqlDriver::FieldName);
    s.append(QLatin1String("ORDER BY "));
    QString sort_field = QString("%1.%2").arg(table).arg(field);

    // If the field is a string, sort using its lowercase form so sort is
    // case-insensitive.
    QVariant::Type type = f.type();

    // TODO(XXX) Instead of special-casing tracknumber here, we should ask the
    // child class to format the expression for sorting.
    if (sort_field.contains("tracknumber")) {
        sort_field = QString("cast(%1 as integer)").arg(sort_field);
    } else if (type == QVariant::String) {
        sort_field = QString("lower(%1)").arg(sort_field);
    }
    s.append(sort_field);

    s += m_eSortOrder == Qt::AscendingOrder ? QLatin1String(" ASC") : QLatin1String(" DESC");
    return s;
}

Qt::ItemFlags BaseSqlTableModel::readWriteFlags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    //Enable dragging songs from this data model to elsewhere (like the waveform
    //widget to load a track into a Player).
    defaultFlags |= Qt::ItemIsDragEnabled;

    if ( index.column() == fieldIndex(LIBRARYTABLE_FILETYPE)
         || index.column() == fieldIndex(LIBRARYTABLE_LOCATION)
         || index.column() == fieldIndex(LIBRARYTABLE_DURATION)
         || index.column() == fieldIndex(LIBRARYTABLE_BITRATE)
         || index.column() == fieldIndex(LIBRARYTABLE_DATETIMEADDED))
    {
        return defaultFlags | QAbstractItemModel::flags(index);
    }
    else if (index.column() == fieldIndex(LIBRARYTABLE_TIMESPLAYED)) {
        return defaultFlags | QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
    }
    else {
        return defaultFlags | QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }
}

Qt::ItemFlags BaseSqlTableModel::readOnlyFlags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (!index.isValid())
      return Qt::ItemIsEnabled;

    //Enable dragging songs from this data model to elsewhere (like the waveform widget to
    //load a track into a Player).
    defaultFlags |= Qt::ItemIsDragEnabled;

    return defaultFlags;

}

Qt::ItemFlags BaseSqlTableModel::flags(const QModelIndex &index) const
{
    return readWriteFlags(index);
}

void BaseSqlTableModel::setLibraryPrefix(QString sPrefix)
{
	m_sPrefix = sPrefix;
	if (sPrefix[sPrefix.length()-1] == '/' || sPrefix[sPrefix.length()-1] == '\\')
		m_sPrefix.chop(1);
}
