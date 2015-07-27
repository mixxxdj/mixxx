#ifndef CONTROLLERLEARNINGEVENTFILTER_H
#define CONTROLLERLEARNINGEVENTFILTER_H

#include <QObject>
#include <QEvent>

#include "controlobject.h"
#include "widget/controlwidgetconnection.h"

struct ControlInfo {
    ControlInfo()
            : clickControl(NULL),
              emitOption(ControlWidgetConnection::EMIT_ON_PRESS_AND_RELEASE),
              leftClickControl(NULL),
              leftEmitOption(ControlWidgetConnection::EMIT_ON_PRESS_AND_RELEASE),
              rightClickControl(NULL),
              rightEmitOption(ControlWidgetConnection::EMIT_ON_PRESS_AND_RELEASE) {
    }

    ControlObject* clickControl;
    ControlWidgetConnection::EmitOption emitOption;
    ControlObject* leftClickControl;
    ControlWidgetConnection::EmitOption leftEmitOption;
    ControlObject* rightClickControl;
    ControlWidgetConnection::EmitOption rightEmitOption;
};

class ControllerLearningEventFilter : public QObject {
    Q_OBJECT
  public:
    ControllerLearningEventFilter(QObject* pParent = NULL);
    virtual ~ControllerLearningEventFilter();

    virtual bool eventFilter(QObject* pObject, QEvent* pEvent);

    void addWidgetClickInfo(QWidget* pWidget, Qt::MouseButton buttonState,
                            ControlObject* pControl,
                            ControlWidgetConnection::EmitOption emitOption);

  public slots:
    void startListening();
    void stopListening();

  signals:
    void controlClicked(ControlObject* pControl);

  private:
    QHash<QWidget*, ControlInfo> m_widgetControlInfo;
    bool m_bListening;
};


#endif /* CONTROLLERLEARNINGEVENTFILTER_H */
