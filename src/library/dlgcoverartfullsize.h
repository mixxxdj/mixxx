#ifndef DLGCOVERARTFULLSIZE_H
#define DLGCOVERARTFULLSIZE_H

#include <QDialog>

#include "library/ui_dlgcoverartfullsize.h"
#include "library/coverart.h"

class DlgCoverArtFullSize
        : public QDialog,
          public Ui::DlgCoverArtFullSize {
    Q_OBJECT
  public:
    DlgCoverArtFullSize(QWidget* parent=0);
    virtual ~DlgCoverArtFullSize();

    void init(QPixmap pixmap);
};

#endif // DLGCOVERARTFULLSIZE_H
