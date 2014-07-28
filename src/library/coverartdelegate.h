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
    // If the CoverDelegate is locked, it must not try
    // to load and search covers.
    // It means that in this cases it will just draw
    // covers which are already in the pixmapcache.
    // It is very important when the user scoll down
    // very fast or when they hold an arrow key, because
    // in these cases paint() would be called MANY times
    // and it would be doing tons of "requestPixmaps",
    // which could easily freeze the whole UI.
    void slotLock(bool lock);

  private:
    QTableView* m_pTableView;
    TrackModel* m_pTrackModel;
    bool m_bIsLocked;
};

#endif // COVERARTDELEGATE_H
