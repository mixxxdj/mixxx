#pragma once

#include <QPushButton>

/// Subclass of QPushButton that replaces a portion of the button label
/// with an ellipsis (according to Qt::TextElideMode) when the text would
/// otherwise overflow the available space.
///
/// See WPushButton when you need to connect to a ControlObject.
class WBasicPushButton : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(Qt::TextElideMode elideMode READ elideMode WRITE setElideMode);

  public:
    explicit WBasicPushButton(QWidget* pParent = nullptr);

    void setElideMode(Qt::TextElideMode elideMode);
    Qt::TextElideMode elideMode() const {
        return m_elideMode;
    };

    QSize minimumSizeHint() const override;

  protected:
    QString buildToolTip() const;
    bool event(QEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

    Qt::TextElideMode m_elideMode;
};
