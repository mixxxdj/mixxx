#pragma once

#include <QLabel>

/// This is a QLabel for use in controller settings.
/// It is expected to have a QCheckBox buddy assigned. If so, left-click on the
/// label will toggle the checkbox and set focus on it.
class WSettingsCheckBoxLabel : public QLabel {
    Q_OBJECT
  public:
    explicit WSettingsCheckBoxLabel(QWidget* pParent = nullptr,
            Qt::WindowFlags flags = Qt::WindowFlags())
            : QLabel(pParent, flags) {

              };
    explicit WSettingsCheckBoxLabel(const QString& text,
            QWidget* pParent = nullptr,
            Qt::WindowFlags flags = Qt::WindowFlags())
            : QLabel(text, pParent, flags) {
              };

  protected:
    void mousePressEvent(QMouseEvent* pEvent) override;
};
