#ifndef WCOVERARTLABEL_H
#define WCOVERARTLABEL_H

#include <QLabel>
#include <QWidget>

#include "dlgcoverartfullsize.h"

class WCoverArtLabel : public QLabel {
  public:
    WCoverArtLabel(QWidget* parent = 0);
    virtual ~WCoverArtLabel();

    void setCoverArt(CoverInfo info, QPixmap pixmap);

  protected:
    void enterEvent(QEvent*);
    void leaveEvent(QEvent*);

  private:
    CoverInfo m_coverInfo;
};

#endif // WCOVERARTLABEL_H
