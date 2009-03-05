#include <proxymodel.h>
#include <QDebug>

// These 3 includes are required to get the sortOrder
#include <mixxxview.h>
#include <wtracktableview.h>
#include <QHeaderView>

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
  const Qt::SortOrder sortOrder = ((MixxxView *)this->parent())->m_pTrackTableView->horizontalHeader()->sortIndicatorOrder();

  // Do the strings look like they are BPM counts?
  const QString bpmPattern = "^[0-9]+\\.[0-9]+$";
  // TODO: this takes about 2 seconds to sort on a Core2 6600, should probably do something to optimize it a bit.
  if (sourceModel()->data( left ).toString().trimmed().indexOf(QRegExp(bpmPattern)) == 0
    && sourceModel()->data( right ).toString().trimmed().indexOf(QRegExp(bpmPattern)) == 0) {
    double leftBPM = sourceModel()->data( left ).toString().trimmed().toDouble();
    double rightBPM = sourceModel()->data( right ).toString().trimmed().toDouble();

    if (leftBPM == 0.0 && sortOrder == Qt::AscendingOrder) rightBPM = -1 * rightBPM;
    if (rightBPM == 0.0 && sortOrder == Qt::AscendingOrder) leftBPM = -1 * leftBPM;
//    qDebug() << "BPM Comparasion leftBPM =" << leftBPM << "rightBPM =" << rightBPM;
    return leftBPM < rightBPM;
  }

  const QString invalidStartCharsExpr = "^[()]";

  QString leftStr = sourceModel()->data( left ).toString().trimmed().simplified().toLower().remove(QRegExp("^the "));;
  QString rightStr = sourceModel()->data( right ).toString().trimmed().simplified().toLower().remove(QRegExp("^the "));;

  leftStr = leftStr.replace(QRegExp(invalidStartCharsExpr, Qt::CaseInsensitive, QRegExp::RegExp2), "~~");
  if (leftStr.contains(" - ")) { leftStr = "~"; }
  if (!leftStr.length()) { leftStr = "~~~"; }

  rightStr = rightStr.replace(QRegExp(invalidStartCharsExpr, Qt::CaseInsensitive, QRegExp::RegExp2), "~~");
  if (rightStr.contains(" - ")) { rightStr = "~"; }
  if (!rightStr.length()) { rightStr = "~~~"; }

  if (sortOrder == Qt::DescendingOrder && (leftStr.indexOf("~") == 0 || rightStr.indexOf("~") == 0)) {
    return rightStr < leftStr;
  }
  return leftStr < rightStr;
}
