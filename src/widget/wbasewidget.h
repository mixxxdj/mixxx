#ifndef WBASEWIDGET_H
#define WBASEWIDGET_H

#include <QString>
#include <QWidget>
#include <QList>

class ControlWidgetConnection;

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

    void addLeftConnection(ControlWidgetConnection* pConnection);
    void addRightConnection(ControlWidgetConnection* pConnection);
    void addConnection(ControlWidgetConnection* pConnection);

    // Set a ControlWidgetConnection to be the display connection for the
    // widget. The connection should also be added via an addConnection method
    // or it will not be deleted or receive updates.
    void setDisplayConnection(ControlWidgetConnection* pConnection);

    double getControlParameter() const;
    double getControlParameterLeft() const;
    double getControlParameterRight() const;
    double getControlParameterDisplay() const;

  protected:
    virtual void onConnectedControlValueChanged(double v) {
        Q_UNUSED(v);
    }

    void resetControlParameters();
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

  private:
    QWidget* m_pWidget;
    QString m_baseTooltip;
    QList<ControlWidgetConnection*> m_connections;
    ControlWidgetConnection* m_pDisplayConnection;
    QList<ControlWidgetConnection*> m_leftConnections;
    QList<ControlWidgetConnection*> m_rightConnections;

    friend class ControlParameterWidgetConnection;
};

#endif /* WBASEWIDGET_H */
