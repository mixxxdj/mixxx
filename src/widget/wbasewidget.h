#ifndef WBASEWIDGET_H
#define WBASEWIDGET_H

#include <QString>
#include <QWidget>
#include <QList>
#include <QObject>
#include <QScopedPointer>
#include <QDomNode>

class ControlObjectSlave;

class WBaseWidget;
class ControlWidgetConnection : public QObject {
    Q_OBJECT
  public:
    enum EmitOption {
        EMIT_NEVER                = 0x00,
        EMIT_ON_PRESS             = 0x01,
        EMIT_ON_RELEASE           = 0x02,
        EMIT_ON_PRESS_AND_RELEASE = 0x03
    };

    // Takes ownership of pControl.
    ControlWidgetConnection(WBaseWidget* pBaseWidget,
                            ControlObjectSlave* pControl);
    virtual ~ControlWidgetConnection();

    double getControlParameter() const;

    virtual void reset() = 0;
    virtual void setControlParameterDown(double v) = 0;
    virtual void setControlParameterUp(double v) = 0;

  protected slots:
    virtual void slotControlValueChanged(double v) = 0;

  protected:
    WBaseWidget* m_pWidget;
    QScopedPointer<ControlObjectSlave> m_pControl;
};

class ValueControlWidgetConnection : public ControlWidgetConnection {
    Q_OBJECT
  public:
    ValueControlWidgetConnection(WBaseWidget* pBaseWidget,
                                 ControlObjectSlave* pControl,
                                 bool connectValueFromWidget,
                                 bool connectValueToWidget,
                                 EmitOption emitOption);
    virtual ~ValueControlWidgetConnection();

  protected:
    void reset();
    void setControlParameterDown(double v);
    void setControlParameterUp(double v);

  protected slots:
    void slotControlValueChanged(double v);

  private:
    bool m_bConnectValueFromWidget;
    bool m_bConnectValueToWidget;
    EmitOption m_emitOption;
};

class DisabledControlWidgetConnection : public ControlWidgetConnection {
    Q_OBJECT
  public:
    DisabledControlWidgetConnection(WBaseWidget* pBaseWidget,
                                    ControlObjectSlave* pControl);
    virtual ~DisabledControlWidgetConnection();

  protected:
    void reset();
    void setControlParameterDown(double v);
    void setControlParameterUp(double v);

  protected slots:
    void slotControlValueChanged(double v);
};

class WBaseWidget {
  public:
    WBaseWidget(QWidget* pWidget);
    virtual ~WBaseWidget();

    QWidget* toQWidget() {
        return m_pWidget;
    }

    void setBaseTooltip(const QString& tooltip) {
        m_baseTooltip = tooltip;
        m_pWidget->setToolTip(tooltip);
    }

    QString baseTooltip() const {
        return m_baseTooltip;
    }

    void setControlDisabled(bool disabled) {
        m_bDisabled = disabled;
    }

    bool controlDisabled() const {
        return m_bDisabled;
    }

    void addLeftConnection(ControlWidgetConnection* pConnection);
    void addRightConnection(ControlWidgetConnection* pConnection);
    void addConnection(ControlWidgetConnection* pConnection);
    void setDisplayConnection(ControlWidgetConnection* pConnection);

    double getConnectedControl() const;
    double getConnectedControlLeft() const;
    double getConnectedControlRight() const;
    double getConnectedDisplayValue() const;

  protected:
    virtual void onConnectedControlValueChanged(double v) {
        Q_UNUSED(v);
    }

    void resetConnectedControls();
    void setConnectedControlDown(double v);
    void setConnectedControlUp(double v);
    void setConnectedControlLeftDown(double v);
    void setConnectedControlLeftUp(double v);
    void setConnectedControlRightDown(double v);
    void setConnectedControlRightUp(double v);

  private:
    QWidget* m_pWidget;
    bool m_bDisabled;
    QString m_baseTooltip;
    QList<ControlWidgetConnection*> m_connections;
    ControlWidgetConnection* m_pDisplayConnection;
    QList<ControlWidgetConnection*> m_leftConnections;
    QList<ControlWidgetConnection*> m_rightConnections;

    friend class ValueControlWidgetConnection;
    friend class DisabledControlWidgetConnection;
};

#endif /* WBASEWIDGET_H */
