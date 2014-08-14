#ifndef DLGCOVERARTFULLSIZE_H
#define DLGCOVERARTFULLSIZE_H

#include <QDialog>

#include "ui_dlgcoverartfullsize.h"
#include "util/singleton.h"

class DlgCoverArtFullSize
        : public QDialog,
          public Ui::DlgCoverArtFullSize,
          public Singleton<DlgCoverArtFullSize>
{
    Q_OBJECT
  public:
    void init();

  protected:
    DlgCoverArtFullSize();
    virtual ~DlgCoverArtFullSize();
    friend class Singleton<DlgCoverArtFullSize>;

  private slots:
    void slotPixmapFound(int trackId, QPixmap pixmap);

  private:
    QPixmap m_cover;
};

#endif // DLGCOVERARTFULLSIZE_H
