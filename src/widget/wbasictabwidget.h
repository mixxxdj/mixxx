#pragma once

#include <QTabWidget>

/// Subclass of QTabWidget for DlgTrackInfo/~Multi and similar dialogs
/// that allows the Up and Down arrow keys to move keyboard focus
/// to the prev/next focusable widget.
class WBasicTabWidget : public QTabWidget {
    Q_OBJECT
  public:
    explicit WBasicTabWidget(QWidget* parent = nullptr);

  protected:
    void keyPressEvent(QKeyEvent* event) override;
};
