#pragma once

#include <QWidget>
#include <QEvent>
#include <QString>

#include "preferences/usersettings.h"
#include "util/color/color.h"
#include "widget/wbasewidget.h"

class ControlProxy;

/** Abstract class used in widgets connected to ControlObjects. Derived
  * widgets can implement the signal and slot for manipulating the widgets
  * value. The widgets internal value should match that of a MIDI control.
  * The ControlObject can contain another mapping of the MIDI/Widget
  * value, but the mapping should always be done in the ControlObject. */
class WWidget : public QWidget, public WBaseWidget {
   Q_OBJECT
  public:
    explicit WWidget(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    ~WWidget() override;

    Q_PROPERTY(double value READ getControlParameterDisplay);

  protected:
    bool touchIsRightButton();
    bool event(QEvent* e) override;
    void setScaleFactor(double value) {
        m_scaleFactor = value;
    }
    double scaleFactor() {
        return m_scaleFactor;
    }

    enum Qt::MouseButton m_activeTouchButton;

  private:
    ControlProxy* m_pTouchShift;
    double m_scaleFactor;
};
