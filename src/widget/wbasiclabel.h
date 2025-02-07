#pragma once

#include <QLabel>

/// Subclass of QLabel for DlgTrackInfo/~Multi and similar dialogs
/// that allows the Up and Down arrow keys to move keyboard focus
/// to the prev/next focusable widget.
///
/// See WLabel when you need to connect to a ControlObject.
class WBasicLabel : public QLabel {
    Q_OBJECT
  public:
    explicit WBasicLabel(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    explicit WBasicLabel(const QString& text,
            QWidget* parent = nullptr,
            Qt::WindowFlags f = Qt::WindowFlags());

  protected:
    void keyPressEvent(QKeyEvent* event) override;
};
