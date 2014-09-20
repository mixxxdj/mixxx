#ifndef WCOVERARTLABEL_H
#define WCOVERARTLABEL_H

#include <QLabel>
#include <QMouseEvent>
#include <QWidget>

#include "dlgcoverartfullsize.h"
#include "wcoverartmenu.h"

class WCoverArtLabel : public QLabel {
    Q_OBJECT
  public:
    WCoverArtLabel(QWidget* parent = 0);
    virtual ~WCoverArtLabel();

    void setCoverArt(TrackPointer track, CoverInfo info, QPixmap pixmap);

  signals:
    void coverLocationUpdated(const QString&, const QString&, QPixmap);

  protected:
    void leaveEvent(QEvent*);
    void mousePressEvent(QMouseEvent* event);

  private slots:
      void slotCoverMenu(const QPoint& pos);

  private:
    TrackPointer m_pTrack;
    CoverInfo m_coverInfo;
    QPixmap m_pixmap;
    WCoverArtMenu* m_pCoverMenu;
};

#endif // WCOVERARTLABEL_H
