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
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  private:
    
    LibraryFeature* m_pFeature;
};

#endif // SAVEDQUERIESTABLEMODEL_H
