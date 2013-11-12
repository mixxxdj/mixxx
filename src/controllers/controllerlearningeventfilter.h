#ifndef CONTROLLERLEARNINGEVENTFILTER_H
#define CONTROLLERLEARNINGEVENTFILTER_H

#include <QObject>
#include <QEvent>

#include "controlobject.h"
#include "controlobjectthreadwidget.h"

struct ControlInfo {
    ControlInfo()
            : clickControl(NULL),
              emitOption(ControlObjectThreadWidget::EMIT_ON_PRESS_AND_RELEASE),
              leftClickControl(NULL),
              leftEmitOption(ControlObjectThreadWidget::EMIT_ON_PRESS_AND_RELEASE),
              rightClickControl(NULL),
              rightEmitOption(ControlObjectThreadWidget::EMIT_ON_PRESS_AND_RELEASE) {
    }

    ControlObject* clickControl;
    ControlObjectThreadWidget::EmitOption emitOption;
    ControlObject* leftClickControl;
    ControlObjectThreadWidget::EmitOption leftEmitOption;
    ControlObject* rightClickControl;
    ControlObjectThreadWidget::EmitOption rightEmitOption;
};

class ControllerLearningEventFilter : public QObject {
    Q_OBJECT
  public:
    ControllerLearningEventFilter(QObject* pParent = NULL);
    virtual ~ControllerLearningEventFilter();

    virtual bool eventFilter(QObject* pObject, QEvent* pEvent);

    void addWidgetClickInfo(QWidget* pWidget, Qt::MouseButton buttonState,
                            ControlObject* pControl,
                            ControlObjectThreadWidget::EmitOption emitOption);

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
