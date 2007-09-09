#include <proxymodel.h>

SortFilterProxyModel::SortFilterProxyModel( QObject * parent ) :
    QSortFilterProxyModel( parent )
{
    setFilterCaseSensitivity( Qt::CaseInsensitive );
}

bool SortFilterProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex & sourceParent ) const
{
    QModelIndex index1 = sourceModel()->index(sourceRow, 1, sourceParent);
    QModelIndex index2 = sourceModel()->index(sourceRow, 2, sourceParent);
    QModelIndex index3 = sourceModel()->index(sourceRow, 7, sourceParent);

    return ( sourceModel()->data(index1).toString().contains(filterRegExp())
            || sourceModel()->data(index2).toString().contains(filterRegExp())
            || sourceModel()->data(index3).toString().contains(filterRegExp())
    );
}
