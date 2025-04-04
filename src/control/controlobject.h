#pragma once

#include <QObject>
#include <QEvent>

#include "preferences/usersettings.h"
#include "controllers/midi/midimessage.h"
#include "control/control.h"

class ControlObject : public QObject {
    Q_OBJECT

  public:
    ControlObject();

    ControlObject(const ConfigKey& key,
                  bool bIgnoreNops = true,
                  bool bTrack = false,
                  bool bPersist = false,
                  double defaultValue = 0.0);

    virtual ~ControlObject();

    static ControlObject* getControl(const ConfigKey& key, ControlFlags flags = ControlFlag::None);
    static ControlObject* getControl(const QString& group,
                                     const QString& item,
                                     ControlFlags flags = ControlFlag::None) {
        return getControl(ConfigKey(group, item), flags);
    }

    static bool exists(const ConfigKey& key);

    QString name() const;
    void setName(const QString& name);

    QString description() const;
    void setDescription(const QString& description);

    void setKbdRepeatable(bool enable);
    bool getKbdRepeatable() const;

    void addAlias(const ConfigKey& aliasKey) const;

    ConfigKey getKey() const;

    double get() const;
    bool toBool() const;
    static double get(const ConfigKey& key);
    static bool toBool(const ConfigKey& key);

    void set(double value);
    void setAndConfirm(double value);
    void forceSet(double value);
    static void set(const ConfigKey& key, const double& value);

    void reset();
    void setDefaultValue(double dValue);
    double defaultValue() const;

    virtual double getParameter() const;
    virtual double getParameterForValue(double value) const;
    virtual double getParameterForMidi(double midiValue) const;
    virtual void setParameter(double v);
    virtual void setParameterFrom(double v, QObject* pSender = nullptr);

    template <typename Receiver, typename Slot>
    bool connectValueChangeRequest(Receiver receiver, Slot func,
                                   Qt::ConnectionType type = Qt::AutoConnection) {
        return m_pControl ? m_pControl->connectValueChangeRequest(receiver, func, type) : false;
    }

    void setReadOnly();
    void triggerCallback();  // Public declaration

  signals:
    void valueChanged(double);

  public:
    virtual void setValueFromMidi(MidiOpCode o, double v);
    virtual double getMidiParameter() const;

  protected:
    ConfigKey m_key;
    QSharedPointer<ControlDoublePrivate> m_pControl;

  private slots:
    void privateValueChanged(double value, QObject* pSetter);
    void readOnlyHandler(double v);

  private:
    bool m_isExecutingCallback = false;

    ControlObject(ControlObject&&) = delete;
    ControlObject(const ControlObject&) = delete;
    ControlObject& operator=(ControlObject&&) = delete;
    ControlObject& operator=(const ControlObject&) = delete;

    inline bool ignoreNops() const {
        return m_pControl ? m_pControl->ignoreNops() : true;
    }

    std::function<void()> m_callback;  // Optional callback
};
