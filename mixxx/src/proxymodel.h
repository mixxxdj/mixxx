#ifndef PROXYMODEL_H
#define PROXYMODEL_H

#include <QSortFilterProxyModel>

class SortFilterProxyModel : public QSortFilterProxyModel
{
    public:
        SortFilterProxyModel( QObject * parent = 0 );
    protected:
        bool filterAcceptsRow( int, const QModelIndex & ) const;
        bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;
};

#endif 
