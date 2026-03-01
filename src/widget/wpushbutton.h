#pragma once

#include <QString>
#include <QTimer>
#include <QVector>
#include <memory>

#include "control/controlbuttonmode.h"
#include "util/fpclassify.h"
#include "util/performancetimer.h"
#include "widget/wpixmapstore.h"
#include "widget/wwidget.h"

class QDomNode;
class SkinContext;

class WPushButton : public WWidget {
    Q_OBJECT
  public:
    explicit WPushButton(QWidget* pParent = nullptr);
    // Used by WPushButtonTest.
    WPushButton(QWidget* pParent,
            mixxx::control::ButtonMode leftButtonMode,
            mixxx::control::ButtonMode rightButtonMode);

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
        if (!util_isnan(value) && m_iNoStates > 0) {
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

  private slots:
    void updateSlot();

  protected:
    bool event(QEvent* e) override;
    void paintEvent(QPaintEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void fillDebugTooltip(QStringList* debug) override;

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

    void paintOnDevice(QPaintDevice* pd, int idx);

    // True, if the button is currently pressed
    bool m_bPressed;
    // True, if the button is pointer is above button
    bool m_bHovered;
    // Set true by WHotcueButton while it's being dragged
    bool m_dragging;

    // Array of associated pixmaps
    int m_iNoStates;
    Qt::TextElideMode m_elideMode;
    QVector<QString> m_text;
    QVector<PaintablePointer> m_pressedPixmaps;
    QVector<PaintablePointer> m_unpressedPixmaps;

    // Associated background pixmap
    PaintablePointer m_pPixmapBack;

    // short click toggle button long click push button
    mixxx::control::ButtonMode m_leftButtonMode;
    mixxx::control::ButtonMode m_rightButtonMode;
    QTimer m_clickTimer;
    QVector<int> m_align;

    // Animates long press latching by storing the off state of the
    // WPushButton in a pixmap and gradually (from left to right)
    // drawing less of the off state on top of the on state, to
    // give a visual indication that the long press latching is in
    // progress.
    class LongPressLatching {
      public:
        LongPressLatching(WPushButton* pButton);
        void paint(QPainter* p);
        void start();
        void stop();

      private:
        WPushButton* m_pButton;
        QPixmap m_preLongPressPixmap;
        PerformanceTimer m_sinceStart;
        QTimer m_animTimer;
    };

    // Only assigned for WPushButtons that use long press latching
    std::unique_ptr<LongPressLatching> m_pLongPressLatching;
};
