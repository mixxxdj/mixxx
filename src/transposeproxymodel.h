#include <QAbstractProxyModel>
#include <QModelIndex>

class TransposeProxyModel : public QAbstractProxyModel{
public:
  TransposeProxyModel(QObject *p = 0) : QAbstractProxyModel(p){}
  QModelIndex mapFromSource ( const QModelIndex & sourceIndex ) const{
    return index(sourceIndex.column(), sourceIndex.row());
  }
  QModelIndex mapToSource ( const QModelIndex & proxyIndex ) const{
    return sourceModel()->index(proxyIndex.column(), proxyIndex.row());
  }
  QModelIndex index(int r, int c, const QModelIndex &ind=QModelIndex()) const{
    Q_UNUSED(ind);
    return createIndex(r,c);
  }
  QModelIndex parent(const QModelIndex&) const {
    return QModelIndex();
  }
  int rowCount(const QModelIndex &) const{
    return sourceModel()->columnCount();
  }
  int columnCount(const QModelIndex &) const{
    return sourceModel()->rowCount();
  }
  QVariant data(const QModelIndex &ind, int role) const {
    return sourceModel()->data(mapToSource(ind), role);
  }
};
