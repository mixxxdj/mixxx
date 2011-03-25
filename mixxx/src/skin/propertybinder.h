#ifndef PROPERTYBINDER_H
#define PROPERTYBINDER_H

#include <QObject>
#include <QWidget>
#include <QString>
#include <QVariant>

class ControlObject;
class ControlObjectThreadMain;

class PropertyBinder : QObject {
    Q_OBJECT
  public:
    PropertyBinder(QWidget* pWidget, QString propertyName, ControlObject* pControl);
    virtual ~PropertyBinder();
  signals:
    void setWidgetProperty(const char* name, const QVariant& value);
  private slots:
    void slotValueChanged(double dValue);
  private:
    QString m_propertyName;
    QWidget* m_pWidget;
    ControlObjectThreadMain* m_pControlThreadMain;
};

#endif /* PROPERTYBINDER_H */
