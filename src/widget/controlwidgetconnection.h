#ifndef CONTROLWIDGETCONNECTION_H
#define CONTROLWIDGETCONNECTION_H

#include <QObject>
#include <QString>
#include <QScopedPointer>
#include <QByteArray>

#include "configobject.h"

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

    static QString emitOptionToString(EmitOption option) {
        switch (option) {
            case EMIT_NEVER:
                return "NEVER";
            case EMIT_ON_PRESS:
                return "PRESS";
            case EMIT_ON_RELEASE:
                return "RELEASE";
            case EMIT_ON_PRESS_AND_RELEASE:
                return "PRESS_AND_RELEASE";
            default:
                return "UNKNOWN";
        }
    }

    // Takes ownership of pControl.
    ControlWidgetConnection(WBaseWidget* pBaseWidget,
                            ControlObjectSlave* pControl);
    virtual ~ControlWidgetConnection();

    double getControlParameter() const;

    virtual void resetControl() = 0;
    virtual void setControlParameterDown(double v) = 0;
    virtual void setControlParameterUp(double v) = 0;

    virtual QString toDebugString() const = 0;

  protected slots:
    virtual void slotControlValueChanged(double v) = 0;

  protected:
    WBaseWidget* m_pWidget;
    QScopedPointer<ControlObjectSlave> m_pControl;
};

class ControlParameterWidgetConnection : public ControlWidgetConnection {
    Q_OBJECT
  public:
    ControlParameterWidgetConnection(WBaseWidget* pBaseWidget,
                                     ControlObjectSlave* pControl,
                                     bool connectValueFromWidget,
                                     bool connectValueToWidget,
                                     EmitOption emitOption);
    virtual ~ControlParameterWidgetConnection();

    QString toDebugString() const;

  protected:
    void resetControl();
    void setControlParameterDown(double v);
    void setControlParameterUp(double v);

  private slots:
    void slotControlValueChanged(double v);

  private:
    bool m_bConnectValueFromWidget;
    bool m_bConnectValueToWidget;
    EmitOption m_emitOption;
};

class ControlWidgetPropertyConnection : public ControlWidgetConnection {
    Q_OBJECT
  public:
    ControlWidgetPropertyConnection(WBaseWidget* pBaseWidget,
                                    ControlObjectSlave* pControl,
                                    ConfigObject<ConfigValue>* pConfig,
                                    const QString& property);
    virtual ~ControlWidgetPropertyConnection();

    QString toDebugString() const;

  protected:
    void resetControl();
    void setControlParameterDown(double v);
    void setControlParameterUp(double v);

  private slots:
    void slotControlValueChanged(double v);

  private:
    ConfigObject<ConfigValue>* m_pConfig;
    QByteArray m_propertyName;
};



#endif /* CONTROLWIDGETCONNECTION_H */
