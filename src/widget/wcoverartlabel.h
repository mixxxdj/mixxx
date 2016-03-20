#ifndef WCOVERARTLABEL_H
#define WCOVERARTLABEL_H

#include <QLabel>
#include <QMouseEvent>
#include <QWidget>
#include <QPixmap>

#include "widget/wcoverartmenu.h"

class DlgCoverArtFullSize;

class WCoverArtLabel : public QLabel {
    Q_OBJECT
  public:
    WCoverArtLabel(QWidget* parent = 0);
    virtual ~WCoverArtLabel();

    void setCoverArt(const QString& trackLocation, const CoverInfo& coverInfo, QPixmap px);

  signals:
    void coverArtSelected(const CoverArt& art);
    void reloadCoverArt();

  protected:
    void leaveEvent(QEvent*);
    void mousePressEvent(QMouseEvent* event);

  private slots:
      void slotCoverMenu(const QPoint& pos);

  private:
    CoverInfo m_coverInfo;
    WCoverArtMenu* m_pCoverMenu;
    DlgCoverArtFullSize* m_pDlgFullSize;
    QPixmap m_defaultCover;
};

#endif // WCOVERARTLABEL_H
