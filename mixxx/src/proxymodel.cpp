#include <proxymodel.h>
#include <QDebug>
SortFilterProxyModel::SortFilterProxyModel( QObject * parent ) :
    QSortFilterProxyModel( parent )
{
    setFilterCaseSensitivity( Qt::CaseInsensitive );
}

bool SortFilterProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex & sourceParent ) const
{
    QModelIndex index1 = sourceModel()->index(sourceRow, 0, sourceParent);
    QModelIndex index2 = sourceModel()->index(sourceRow, 1, sourceParent);
    QModelIndex index3 = sourceModel()->index(sourceRow, 6, sourceParent);

    return ( sourceModel()->data(index1).toString().contains(filterRegExp())
            || sourceModel()->data(index2).toString().contains(filterRegExp())
            || sourceModel()->data(index3).toString().contains(filterRegExp())
    );
}

bool SortFilterProxyModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  const QString invalidStartCharsExpr = "^[()]";

  QString leftStr = sourceModel()->data( left ).toString().trimmed().simplified().toLower().remove(QRegExp("^the "));;
  QString rightStr = sourceModel()->data( right ).toString().trimmed().simplified().toLower().remove(QRegExp("^the "));;

  leftStr = leftStr.replace(QRegExp(invalidStartCharsExpr, QRegExp::RegExp2), "~~");
  if (leftStr.contains(" - ")) { leftStr = "~"; }
  if (!leftStr.length()) { leftStr = "~~~"; }

  rightStr = rightStr.replace(QRegExp(invalidStartCharsExpr, QRegExp::RegExp2), "~~");
  if (rightStr.contains(" - ")) { rightStr = "~"; }
  if (!rightStr.length()) { rightStr = "~~~"; }

//  if (leftStr.startsWith("~") || rightStr.startsWith("~")) qDebug() << "left:" << leftStr <<"right:" << rightStr << " result:" << (leftStr < rightStr);

  return leftStr < rightStr;
}
