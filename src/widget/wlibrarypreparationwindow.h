#pragma once
#include "widget/wlibrarybasewindow.h"

class WLibraryPreparationWindow : public WLibraryBaseWindow {
    Q_OBJECT

  public:
    explicit WLibraryPreparationWindow(QWidget* parent = nullptr);
    ~WLibraryPreparationWindow() override = default;

    // private:
    //     QString callingParent;
};
