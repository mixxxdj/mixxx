#pragma once

#include <QLineEdit>

/// Subclass of QLineEdit that allows the Up and Down arrow keys
/// to be used like the Tab/Backtab keys.
class WBasicLineEdit : public QLineEdit {
    Q_OBJECT
  public:
    explicit WBasicLineEdit(QWidget* parent = nullptr);
    explicit WBasicLineEdit(const QString& text, QWidget* parent = nullptr);

  protected:
    void keyPressEvent(QKeyEvent* event) override;
};