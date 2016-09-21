#include <QSqlQuery>
#include <QSqlRecord>
#include <QString>

#include "library/dao/savedqueriesdao.h"

#include "dialog/savedqueries/savedqueriestablemodel.h"

SavedQueriesTableModel::SavedQueriesTableModel(LibraryFeature* pFeature, 
                                               QObject* parent,
                                               QSqlDatabase db) 
        : QAbstractTableModel(parent),
          m_pFeature(pFeature),
          m_savedDao(db) {    
    m_cachedData = m_savedDao.getSavedQueries(m_pFeature);
}

bool SavedQueriesTableModel::isColumnInternal(int column) {
    return column != SavedQueryColumns::QUERY &&
            column != SavedQueryColumns::TITLE &&
            column != SavedQueryColumns::PINNED;
}

QVariant SavedQueriesTableModel::data(const QModelIndex& index, int role) const {  
    if (!index.isValid()) return QVariant();
    
    const SavedSearchQuery& sQuery = m_cachedData.at(index.row());
    switch (index.column()) {
        case SavedQueryColumns::QUERY:
            if (role == Qt::DisplayRole || role == Qt::EditRole) return sQuery.query;
            break;
            
        case SavedQueryColumns::TITLE:
            if (role == Qt::DisplayRole || role == Qt::EditRole) return sQuery.title;
            break;
        
        case SavedQueryColumns::PINNED:
            if (role == Qt::CheckStateRole) {
                return sQuery.pinned ? Qt::Checked : Qt::Unchecked;
            }
            break;
    }
    return QVariant();
}

bool SavedQueriesTableModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid()) return false;
    
    SavedSearchQuery& sQuery = m_cachedData[index.row()];
    switch (index.column()) {
        case SavedQueryColumns::QUERY:
            if (role == Qt::EditRole) {
                sQuery.query = value.toString();
                return true;
            }
            break;
            
        case SavedQueryColumns::TITLE:
            if (role == Qt::EditRole) {
                sQuery.title = value.toString();
                return true;
            }
            break;
            
        case SavedQueryColumns::PINNED:
            if (role == Qt::CheckStateRole) {
                sQuery.pinned = !sQuery.pinned;
                return true;
            }
    }
    return false;
}

QVariant SavedQueriesTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
        return QAbstractTableModel::headerData(section, orientation, role);
    }
    
    switch (section) {
        case SavedQueryColumns::QUERY:
            return tr("Query");
        case SavedQueryColumns::TITLE:
            return tr("Title");
        case SavedQueryColumns::PINNED:
            return tr("Pinned");
    }
    return "";
}

Qt::ItemFlags SavedQueriesTableModel::flags(const QModelIndex& index) const {
    Qt::ItemFlags flags = QAbstractTableModel::flags(index);
    flags |= Qt::ItemIsSelectable;
    flags |= Qt::ItemIsEnabled;
    flags |= Qt::ItemIsEditable;
    
    if (index.column() == SavedQueryColumns::PINNED) {
        flags |= Qt::ItemIsUserCheckable;
    }
    return flags;
}

int SavedQueriesTableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return m_cachedData.length();
}

int SavedQueriesTableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return SavedQueryColumns::NUM_COLUMNS;
}

bool SavedQueriesTableModel::submit() {
    for (const SavedSearchQuery& sQuery : m_cachedData) {
        m_savedDao.updateSavedQuery(sQuery);
    }
}
