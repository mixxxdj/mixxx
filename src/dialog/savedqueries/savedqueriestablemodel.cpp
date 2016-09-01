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
