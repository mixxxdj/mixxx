#pragma once

#include <QTabWidget>

/// Subclass of QTabWidget that allows the Up and Down arrow keys
/// to be used like the Tab/Backtab keys.
class WBasicTabWidget : public QTabWidget {
    Q_OBJECT
  public:
    explicit WBasicTabWidget(QWidget* parent = nullptr);

  protected:
    void keyPressEvent(QKeyEvent* event) override;
};
