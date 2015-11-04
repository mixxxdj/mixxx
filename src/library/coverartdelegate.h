#ifndef COVERARTDELEGATE_H
#define COVERARTDELEGATE_H

#include <QStyledItemDelegate>
#include <QTableView>

#include "library/trackmodel.h"

class CoverArtDelegate : public QStyledItemDelegate {
  Q_OBJECT

  public:
    explicit CoverArtDelegate(QObject* parent = NULL);
    virtual ~CoverArtDelegate();

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;

  private slots:
    // If it is true, it must not try to load and search covers.
    //
    // It means that in this cases it will just draw
    // covers which are already in the pixmapcache.
    //
    // It is useful to handle cases when the user scoll down
    // very fast or when they hold an arrow key, because
    // in these cases 'paint()' would be called very often
    // and it might make CoverDelegate starts many searches,
    // which could bring performance issues.
    void slotOnlyCachedCoverArt(bool b);

  private:
    QTableView* m_pTableView;
    TrackModel* m_pTrackModel;
    bool m_bOnlyCachedCover;
    QString m_sDefaultCover;
    int m_iCoverLocationColumn;
    int m_iMd5Column;
};

#endif // COVERARTDELEGATE_H
