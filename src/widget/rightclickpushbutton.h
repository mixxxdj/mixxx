#pragma once

#include <QPushButton>

class RightClickPushButton : public QPushButton {
    Q_OBJECT
  public:
    explicit RightClickPushButton(const QString& text, QWidget* parent = nullptr);

  private slots:
    void mousePressEvent(QMouseEvent* event) override;

  signals:
    void rightClicked();
};
