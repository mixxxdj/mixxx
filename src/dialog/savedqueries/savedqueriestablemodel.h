#ifndef SAVEDQUERIESTABLEMODEL_H
#define SAVEDQUERIESTABLEMODEL_H

#include <QSqlTableModel>

#include "library/libraryfeature.h"

class SavedQueriesTableModel : public QSqlTableModel
{
  public:
    SavedQueriesTableModel(LibraryFeature* pFeature,
                           QObject* parent = nullptr,
                           QSqlDatabase db = QSqlDatabase());
    
    bool isColumnInternal(int column);
    
  private:
    
    LibraryFeature* m_pFeature;
};

#endif // SAVEDQUERIESTABLEMODEL_H
