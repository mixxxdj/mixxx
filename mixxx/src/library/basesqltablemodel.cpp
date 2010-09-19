// basesqltablemodel.h
// Created by RJ Ryan (rryan@mit.edu) 1/29/2010

#include <QtDebug>

#include "trackinfoobject.h"
#include "library/trackcollection.h"
#include "library/basesqltablemodel.h"
#include "mixxxutils.cpp"

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

QVariant BaseSqlTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()){
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
    if ((role == Qt::DisplayRole || role == Qt::ToolTipRole) && m_trackOverrides.contains(trackId)) {
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
            QVariant value = pTrack->getDuration();
			if (qVariantCanConvert<int>(value)) 
				value = MixxxUtils::secondsToMinutes(qVariantValue<int>(value));
			return value;
        } else if (fieldIndex(LIBRARYTABLE_BITRATE) == col) {
            return QVariant(pTrack->getBitrate());
        } else if (fieldIndex(LIBRARYTABLE_BPM) == col) {
            return QVariant(pTrack->getBpm());
        }
    }
	
	
	QVariant value; 
	
	if (role == Qt::ToolTipRole)
		value = QSqlTableModel::data(index, Qt::DisplayRole);
	else
		value = QSqlTableModel::data(index, role);
		
	if (fieldIndex(LIBRARYTABLE_DURATION) == col)
	{
		if (qVariantCanConvert<int>(value)) 
            value = MixxxUtils::secondsToMinutes(qVariantValue<int>(value));
	}

    return value;
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
