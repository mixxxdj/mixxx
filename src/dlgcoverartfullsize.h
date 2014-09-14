#ifndef DLGCOVERARTFULLSIZE_H
#define DLGCOVERARTFULLSIZE_H

#include <QDialog>

#include "ui_dlgcoverartfullsize.h"
#include "library/coverartcache.h"
#include "util/singleton.h"

class DlgCoverArtFullSize
        : public QDialog,
          public Ui::DlgCoverArtFullSize,
          public Singleton<DlgCoverArtFullSize>
{
    Q_OBJECT
  public:
    void init(QPixmap pixmap);

  protected:
    DlgCoverArtFullSize();
    virtual ~DlgCoverArtFullSize();
    friend class Singleton<DlgCoverArtFullSize>;
};

#endif // DLGCOVERARTFULLSIZE_H
