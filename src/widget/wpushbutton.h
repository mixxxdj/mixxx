#pragma once

#include <QPaintEvent>
#include <QPixmap>
#include <QString>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QFocusEvent>
#include <QTimer>
#include <QVector>

#include "widget/wwidget.h"
#include "widget/wpixmapstore.h"
#include "control/controlpushbutton.h"
#include "skin/skincontext.h"
#include "widget/controlwidgetconnection.h"
#include "util/math.h"

class WPushButton : public WWidget {
    Q_OBJECT
  public:
    explicit WPushButton(QWidget* pParent = nullptr);
    // Used by WPushButtonTest.
    WPushButton(QWidget* pParent, ControlPushButton::ButtonMode leftButtonMode,
                ControlPushButton::ButtonMode rightButtonMode);

    Q_PROPERTY(bool pressed READ isPressed);

    bool isPressed() const {
        return m_bPressed;
    }

    // #MyPushButton:hover would affect most object styles except font color
    // so we use a custom property that allows to also change the font color
    // with #MyPushButton[hover="true"] {}
    Q_PROPERTY(bool hover READ isHovered);

    bool isHovered() const {
        return m_bHovered;
    }

    // The displayValue property is used to restyle the pushbutton with CSS.
    // The declaration #MyButton[displayValue="0"] { } will define the style
    // when the widget is in state 0.  This allows for effects like reversing
    // background and foreground colors to indicate enabled/disabled state.
    Q_PROPERTY(int displayValue READ readDisplayValue NOTIFY displayValueChanged)

    int readDisplayValue() const {
        double value = getControlParameterDisplay();
        if (!isnan(value) && m_iNoStates > 0) {
            return static_cast<int>(value) % m_iNoStates;
        }
        return 0;
    }

    virtual void setup(const QDomNode& node, const SkinContext& context);

    // Sets the number of states associated with this button, and removes
    // associated pixmaps.
    void setStates(int iStates);

  signals:
    void displayValueChanged(int value);

  public slots:
    void onConnectedControlChanged(double dParameter, double dValue) override;

  protected:
    bool event(QEvent* e) override;
    void paintEvent(QPaintEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void focusOutEvent(QFocusEvent* e) override;
    void fillDebugTooltip(QStringList* debug) override;

  protected:
    virtual void restyleAndRepaint();

    // Associates a pixmap of a given state of the button with the widget
    void setPixmap(int iState,
            bool bPressed,
            const PixmapSource& source,
            Paintable::DrawMode mode,
            double scaleFactor);

    // Associates a background pixmap with the widget. This is only needed if
    // the button pixmaps contains alpha channel values.
    void setPixmapBackground(
            const PixmapSource& source,
            Paintable::DrawMode mode,
            double scaleFactor);

    // True, if the button is currently pressed
    bool m_bPressed;
    // True, if the button is pointer is above button
    bool m_bHovered;

    // Array of associated pixmaps
    int m_iNoStates;
    Qt::TextElideMode m_elideMode;
    QVector<QString> m_text;
    QVector<PaintablePointer> m_pressedPixmaps;
    QVector<PaintablePointer> m_unpressedPixmaps;

    // Associated background pixmap
    PaintablePointer m_pPixmapBack;

    // short click toggle button long click push button
    ControlPushButton::ButtonMode m_leftButtonMode;
    ControlPushButton::ButtonMode m_rightButtonMode;
    QTimer m_clickTimer;
    QVector<int> m_align;
};
