#pragma once

#include <QObject>
#include <QEvent>

#include "control/controlobject.h"
#include "widget/controlwidgetconnection.h"

struct ControlInfo {
    ControlInfo()
            : clickControl(NULL),
              emitOption(ControlParameterWidgetConnection::EMIT_ON_PRESS_AND_RELEASE),
              leftClickControl(NULL),
              leftEmitOption(ControlParameterWidgetConnection::EMIT_ON_PRESS_AND_RELEASE),
              rightClickControl(NULL),
              rightEmitOption(ControlParameterWidgetConnection::EMIT_ON_PRESS_AND_RELEASE) {
    }

    ControlObject* clickControl;
    ControlParameterWidgetConnection::EmitOption emitOption;
    ControlObject* leftClickControl;
    ControlParameterWidgetConnection::EmitOption leftEmitOption;
    ControlObject* rightClickControl;
    ControlParameterWidgetConnection::EmitOption rightEmitOption;
};

class ControllerLearningEventFilter : public QObject {
    Q_OBJECT
  public:
    ControllerLearningEventFilter(QObject* pParent = NULL);
    virtual ~ControllerLearningEventFilter();

    virtual bool eventFilter(QObject* pObject, QEvent* pEvent);

    void addWidgetClickInfo(QWidget* pWidget, Qt::MouseButton buttonState,
                            ControlObject* pControl,
                            ControlParameterWidgetConnection::EmitOption emitOption);

  public slots:
    void startListening();
    void stopListening();

  signals:
    void controlClicked(ControlObject* pControl);

  private:
    QHash<QWidget*, ControlInfo> m_widgetControlInfo;
    bool m_bListening;
};
