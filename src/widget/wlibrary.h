#pragma once

#include "widget/wlibrarybasewindow.h"

class WLibrary : public WLibraryBaseWindow {
    Q_OBJECT

  public:
  public:
    explicit WLibrary(QWidget* parent = nullptr);
    ~WLibrary() override = default;

    // private:
    //     QString callingParent;
};
