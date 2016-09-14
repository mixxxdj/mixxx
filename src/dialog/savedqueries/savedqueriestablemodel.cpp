#include <QString>

#include "library/dao/savedqueriesdao.h"

#include "dialog/savedqueries/savedqueriestablemodel.h"

SavedQueriesTableModel::SavedQueriesTableModel(LibraryFeature* pFeature, 
                                               QObject* parent,
                                               QSqlDatabase db) 
        : QSqlTableModel(parent, db),
          m_pFeature(pFeature) {
    QString filter = "libraryFeature='%1'";
    filter = filter.arg(m_pFeature->getSettingsName());
    
    setTable(SAVEDQUERYTABLE);
    setEditStrategy(QSqlTableModel::OnManualSubmit);
    setFilter(filter);
    
    select();
}

bool SavedQueriesTableModel::isColumnInternal(int column) {
    return column != SavedQueryColums::QUERY &&
            column != SavedQueryColums::TITLE &&
            column != SavedQueryColums::PINNED;
}

QVariant SavedQueriesTableModel::data(const QModelIndex& index, int role) const {  
    if (index.column() == SavedQueryColums::PINNED) {
        if (role == Qt::DisplayRole) return QVariant();
        else if (role == Qt::CheckStateRole) {
            QVariant baseData = QSqlTableModel::data(index, Qt::DisplayRole);
            return baseData.toBool() ? Qt::Checked : Qt::Unchecked;
        }
    }
    
    return QSqlTableModel::data(index, role);
}

QVariant SavedQueriesTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
        return QSqlTableModel::headerData(section, orientation, role);
    }
    
    switch (section) {
        case SavedQueryColums::QUERY:
            return tr("Query");
        case SavedQueryColums::TITLE:
            return tr("Title");
        case SavedQueryColums::PINNED:
            return tr("Pinned");
    }
    return "";
}

