#ifndef SAVEDQUERIESTABLEMODEL_H
#define SAVEDQUERIESTABLEMODEL_H

#include <QAbstractTableModel>

#include "library/libraryfeature.h"

class SavedQueriesTableModel : public QAbstractTableModel
{
  public:
    SavedQueriesTableModel(LibraryFeature* pFeature,
                           SavedQueriesDAO& savedDao,
                           QObject* parent = nullptr);
    
    bool isColumnInternal(int column) const;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());
    
public slots:
    
    bool submit() override;
    void removeQuery(const QModelIndex& index);
    
  private:
    
    QList<SavedSearchQuery> m_cachedData;
    QList<int> m_removedQueries;
    
    LibraryFeature* m_pFeature;
    SavedQueriesDAO& m_savedDao;
};

#endif // SAVEDQUERIESTABLEMODEL_H
