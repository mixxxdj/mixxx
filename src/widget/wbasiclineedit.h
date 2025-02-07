#pragma once

#include <QLineEdit>

/// Subclass of QLineEdit for DlgTrackInfo/~Multi and similar dialogs
/// that allows the Up and Down arrow keys to move keyboard focus
/// to the prev/next focusable widget.
class WBasicLineEdit : public QLineEdit {
    Q_OBJECT
  public:
    explicit WBasicLineEdit(QWidget* parent = nullptr);
    explicit WBasicLineEdit(const QString& text, QWidget* parent = nullptr);

  protected:
    void keyPressEvent(QKeyEvent* event) override;
};
