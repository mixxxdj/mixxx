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
    
    bool isColumnInternal(int column);
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    
    int rowCount(const QModelIndex& parent) const;
    int columnCount(const QModelIndex& parent) const;
    
public slots:
    
    bool submit() override;
    
  private:
    QList<SavedSearchQuery> m_cachedData;
    
    LibraryFeature* m_pFeature;
    SavedQueriesDAO& m_savedDao;
};

#endif // SAVEDQUERIESTABLEMODEL_H
