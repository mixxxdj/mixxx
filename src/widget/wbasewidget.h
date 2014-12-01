#ifndef WBASEWIDGET_H
#define WBASEWIDGET_H

#include <QString>
#include <QWidget>
#include <QList>

class ControlWidgetPropertyConnection;
class ControlParameterWidgetConnection;

class WBaseWidget {
  public:
    WBaseWidget(QWidget* pWidget);
    virtual ~WBaseWidget();

    virtual void Init();

    QWidget* toQWidget() {
        return m_pWidget;
    }

    void appendBaseTooltip(const QString& tooltip) {
        m_baseTooltip.append(tooltip);
        m_pWidget->setToolTip(m_baseTooltip);
    }

    void prependBaseTooltip(const QString& tooltip) {
        m_baseTooltip.prepend(tooltip);
        m_pWidget->setToolTip(m_baseTooltip);
    }

    void setBaseTooltip(const QString& tooltip) {
        m_baseTooltip = tooltip;
        m_pWidget->setToolTip(tooltip);
    }

    QString baseTooltip() const {
        return m_baseTooltip;
    }

    void addLeftConnection(ControlParameterWidgetConnection* pConnection);
    void addRightConnection(ControlParameterWidgetConnection* pConnection);
    void addConnection(ControlParameterWidgetConnection* pConnection);

    void addPropertyConnection(ControlWidgetPropertyConnection* pConnection);

    // Set a ControlWidgetConnection to be the display connection for the
    // widget. The connection should also be added via an addConnection method
    // or it will not be deleted or receive updates.
    void setDisplayConnection(ControlParameterWidgetConnection* pConnection);

    double getControlParameter() const;
    double getControlParameterLeft() const;
    double getControlParameterRight() const;
    double getControlParameterDisplay() const;

  protected:
    // Whenever a connected control is changed, onConnectedControlChanged is
    // called. This allows the widget implementor to respond to the change and
    // gives them both the parameter and its corresponding value.
    virtual void onConnectedControlChanged(double dParameter, double dValue) {
        Q_UNUSED(dParameter);
        Q_UNUSED(dValue);
    }

    void resetControlParameter();
    void setControlParameter(double v);
    void setControlParameterDown(double v);
    void setControlParameterUp(double v);
    void setControlParameterLeftDown(double v);
    void setControlParameterLeftUp(double v);
    void setControlParameterRightDown(double v);
    void setControlParameterRightUp(double v);

    // Tooltip handling. We support "debug tooltips" which are basically a way
    // to expose debug information about widgets via the tooltip. To enable
    // this, when widgets should call updateTooltip before they are about to
    // display a tooltip.
    void updateTooltip();
    virtual void fillDebugTooltip(QStringList* debug);

    QList<ControlParameterWidgetConnection*> m_connections;
    ControlParameterWidgetConnection* m_pDisplayConnection;
    QList<ControlParameterWidgetConnection*> m_leftConnections;
    QList<ControlParameterWidgetConnection*> m_rightConnections;

    QList<ControlWidgetPropertyConnection*> m_propertyConnections;

  private:
    QWidget* m_pWidget;
    QString m_baseTooltip;

    friend class ControlParameterWidgetConnection;
};

#endif /* WBASEWIDGET_H */
