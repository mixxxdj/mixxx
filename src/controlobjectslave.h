#ifndef CONTROLOBJECTSLAVE_H
#define CONTROLOBJECTSLAVE_H

#include <qmutex.h>
#include <qobject.h>
#include <qmutex.h>
#include <qwaitcondition.h>
#include <QQueue>

#include "configobject.h"

class ControlDoublePrivate;

class ControlObjectSlave : public QObject {
    Q_OBJECT
  public:
    ControlObjectSlave(const QString& g, const QString& i, QObject* pParent=NULL);
    ControlObjectSlave(const char* g, const char* i, QObject* pParent=NULL);
    ControlObjectSlave(const ConfigKey& key, QObject* pParent=NULL);
    virtual ~ControlObjectSlave();

    void initialize(const ConfigKey& key);

    bool connectValueChanged(const QObject* receiver,
            const char* method, Qt::ConnectionType type = Qt::AutoConnection);
    bool connectValueChanged(
            const char* method, Qt::ConnectionType type = Qt::AutoConnection );


    /** Called from update(); */
    void emitValueChanged();

    inline ConfigKey getKey() const { return m_key; }
    inline bool valid() const { return m_pControl != NULL; }

    // Returns the value of the object. Thread safe, non-blocking.
    virtual double get();

  public slots:
    // Set the control to a new value. Non-blocking.
    virtual void slotSet(double v);
    // Sets the control value to v. Thread safe, non-blocking.
    virtual void set(double v);
    // Resets the control to its default value. Thread safe, non-blocking.
    virtual void reset();

  signals:
    void valueChanged(double);

  protected slots:
    // Receives the value from the master control and re-emits either
    // valueChanged(double) or valueChangedByThis(double) based on pSetter.
    virtual void slotValueChanged(double v, QObject* pSetter);

  protected:
    ConfigKey m_key;
    // Pointer to connected control.
    ControlDoublePrivate* m_pControl;
};

#endif // CONTROLOBJECTSLAVE_H
