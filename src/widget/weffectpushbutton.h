#ifndef WEFFECTPUSHBUTTON_H
#define WEFFECTPUSHBUTTON_H

#include <QMenu>
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
#include "controlpushbutton.h"
#include "skin/skincontext.h"
#include "controlwidgetconnection.h"
#include "effects/effectsmanager.h"

class WEffectPushButton : public WWidget {
    Q_OBJECT
  public:
    WEffectPushButton(QWidget* pParent, EffectsManager* pEffectsManager);
    // Used by WEffectPushButtonTest.
    WEffectPushButton(QWidget* pParent, ControlPushButton::ButtonMode leftButtonMode,
                ControlPushButton::ButtonMode rightButtonMode);
    virtual ~WEffectPushButton();

    Q_PROPERTY(bool pressed READ isPressed);

    bool isPressed() const {
        return m_bPressed;
    }

    // The displayValue property is used to restyle the pushbutton with CSS.
    // The declaration #MyButton[displayValue="0"] { } will define the style
    // when the widget is in state 0.  This allows for effects like reversing
    // background and foreground colors to indicate enabled/disabled state.
    Q_PROPERTY(int displayValue READ readDisplayValue)

    int readDisplayValue() const {
        double value = getControlParameterDisplay();
        int idx = static_cast<int>(value) % m_iNoStates;
        return idx;
    }

    void setup(QDomNode node, const SkinContext& context);

    // Sets the number of states associated with this button, and removes
    // associated pixmaps.
    void setStates(int iStatesW);

  public slots:
    void onConnectedControlChanged(double dParameter, double dValue);

  protected:
    virtual void paintEvent(QPaintEvent*);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);
    virtual void focusOutEvent(QFocusEvent* e);
    void fillDebugTooltip(QStringList* debug);

  private slots:
    void parameterUpdated();
    void slotActionChosen(QAction* action);

  private:
    // Associates a pixmap of a given state of the button with the widget
    void setPixmap(int iState, bool bPressed, const QString &filename);

    // Associates a background pixmap with the widget. This is only needed if
    // the button pixmaps contains alpha channel values.
    void setPixmapBackground(const QString &filename, Paintable::DrawMode mode);

    // True, if the button is currently pressed
    bool m_bPressed;

    // Array of associated pixmaps
    int m_iNoStates;
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

    EffectsManager* m_pEffectsManager;
    EffectParameterSlotBasePointer m_pEffectParameterSlot;
    QMenu* m_pButtonMenu;
};

#endif // WEFFECTPUSHBUTTON_H
