#include "skin/propertybinder.h"
#include "controlobject.h"
#include "controlobjectthreadmain.h"

#include <QDebug>

PropertyBinder::PropertyBinder(QWidget* pWidget, QString propertyName,
        ControlObject* pControl, ConfigObject<ConfigValue>* pConfig)
        : QObject(pWidget),
          m_propertyName(propertyName),
          m_pWidget(pWidget),
          m_pControlThreadMain(new ControlObjectThreadMain(pControl)),
          m_pConfig(pConfig) {
    connect(m_pControlThreadMain, SIGNAL(valueChanged(double)),
            this, SLOT(slotValueChanged(double)));
    bool ok;
    double dValue = m_pConfig->getValueString(pControl->getKey()).toDouble(&ok);
    if (ok) {
        pControl->set(dValue);
    } else {
        dValue = m_pControlThreadMain->get();
    }
    slotValueChanged(dValue);
}

PropertyBinder::~PropertyBinder() {
    delete m_pControlThreadMain;
}

void PropertyBinder::slotValueChanged(double dValue) {
    //qDebug() << this << m_propertyName << "valueChanged" << dValue;
    QVariant value(dValue);
    QByteArray propertyAscii = m_propertyName.toAscii();
    if (!m_pWidget->setProperty(propertyAscii.constData(), value)) {
        qDebug() << "Setting property" << m_propertyName << "to widget failed. Value:" << value;
    }
    m_pConfig->set(m_pControlThreadMain->getControlObject()->getKey(),
            QString::number(dValue));
}
