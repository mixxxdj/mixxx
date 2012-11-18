#include "skin/propertybinder.h"
#include "controlobject.h"
#include "controlobjectthreadmain.h"

#include <QDebug>

PropertyBinder::PropertyBinder(QWidget* pWidget, QString propertyName, ControlObject* pControl)
        : QObject(pWidget),
          m_propertyName(propertyName),
          m_pWidget(pWidget),
          m_pControlThreadMain(new ControlObjectThreadMain(pControl)) {
    connect(m_pControlThreadMain, SIGNAL(valueChanged(double)),
            this, SLOT(slotValueChanged(double)));
    connect(this, SIGNAL(setWidgetProperty(const char*, const QVariant&)),
            pWidget, SLOT(setProperty(const char*, const QVariant&)));
    slotValueChanged(m_pControlThreadMain->get());
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
}
