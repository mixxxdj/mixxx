#pragma once

#include <QTextBrowser>

#include "library/library_decl.h"
#include "library/libraryview.h"

class WLibraryTextBrowser : public QTextBrowser, public LibraryView {
    Q_OBJECT
  public:
    explicit WLibraryTextBrowser(QWidget* parent = nullptr);
    void onShow() override {}
    bool hasFocus() const override;
    void setFocus() override;
    void keyPressEvent(QKeyEvent* event) override;
  signals:
    FocusWidget setLibraryFocus(FocusWidget newFocus);
};
