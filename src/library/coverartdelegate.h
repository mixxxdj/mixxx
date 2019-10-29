#ifndef COVERARTDELEGATE_H
#define COVERARTDELEGATE_H

#include <QHash>
#include <QList>
#include <QLinkedList>

#include "library/tableitemdelegate.h"

class CoverInfoRelative;
class TrackModel;
class WLibraryTableView;

class CoverArtDelegate : public TableItemDelegate {
    Q_OBJECT
  public:
    explicit CoverArtDelegate(WLibraryTableView* parent);
    ~CoverArtDelegate() override = default;

    void paintItem(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const;

  signals:
    void coverReadyForCell(int row, int column);

  private slots:
    // If it is true, it must not try to load and search covers.
    //
    // It means that in this cases it will just draw
    // covers which are already in the pixmapcache.
    //
    // It is useful to handle cases when the user scroll down
    // very fast or when they hold an arrow key, because
    // in these cases 'paint()' would be called very often
    // and it might make CoverDelegate starts many searches,
    // which could bring performance issues.
    void slotOnlyCachedCoverArt(bool b);

    void slotCoverFound(const QObject* pRequestor,
                        const CoverInfoRelative& info,
                        QPixmap pixmap, bool fromCache);

  private:
    QTableView* m_pTableView;
    bool m_bOnlyCachedCover;
    int m_iCoverColumn;
    int m_iCoverSourceColumn;
    int m_iCoverTypeColumn;
    int m_iCoverLocationColumn;
    int m_iCoverHashColumn;
    int m_iTrackLocationColumn;
    int m_iIdColumn;

    // We need to record rows in paint() (which is const) so these are marked
    // mutable.
    mutable QList<int> m_cacheMissRows;
    mutable QHash<quint16, QLinkedList<int> > m_hashToRow;
};

#endif // COVERARTDELEGATE_H
