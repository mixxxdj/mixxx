#include "widget/wsettingscheckboxlabel.h"

#include <QCheckBox>
#include <QMouseEvent>

#include "moc_wsettingscheckboxlabel.cpp"

void WSettingsCheckBoxLabel::mousePressEvent(QMouseEvent* pEvent) {
    if (pEvent->buttons().testFlag(Qt::LeftButton)) {
        QCheckBox* pCB = qobject_cast<QCheckBox*>(buddy());
        if (pCB) {
            pCB->toggle();
            pCB->setFocus(Qt::MouseFocusReason);
        }
    }
    QLabel::mousePressEvent(pEvent);
}
