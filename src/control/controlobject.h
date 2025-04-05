#pragma once

#include <QObject>
#include <QEvent>
#include <QSharedPointer>
#include <functional>

#include "preferences/usersettings.h"
#include "controllers/midi/midimessage.h"

// Forward declarations
class ControlDoublePrivate;
class ConfigKey;

enum class ControlFlag {
    None = 0
    // Add more flags here if needed later
};

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

    static ControlObject* getControl(const ConfigKey& key, ControlFlag flag = ControlFlag::None);
    static ControlObject* getControl(const QString& group,
                                     const QString& item,
                                     ControlFlag flag = ControlFlag::None) {
        return getControl(ConfigKey(group, item), flag);
    }

    static bool exists(const ConfigKey& key);

    QString name() const;
    void setName(const QString& name);

    QString description() const;
    void setDescription(const QString& description);

    void setKbdRepeatable(bool enable);
    bool getKbdRepeatable() const;

    void addAlias(const ConfigKey& aliasKey);
    ConfigKey getKey() const;

    double get() const {
        return m_pControl ? m_pControl->get() : defaultValue();
    }

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

    void triggerCallback() {
        if (m_callback) {
            m_callback();
        }
    }

    virtual void setValueFromMidi(MidiOpCode o, double v);
    virtual double getMidiParameter() const;

  signals:
    void valueChanged(double);

  protected:
    ConfigKey m_key;
    QSharedPointer<ControlDoublePrivate> m_pControl;

  private slots:
    void privateValueChanged(double value, QObject* pSetter);
    void readOnlyHandler(double v);

  private:
    bool m_isExecutingCallback = false;
    std::function<void()> m_callback;

    ControlObject(ControlObject&&) = delete;
    ControlObject(const ControlObject&) = delete;
    ControlObject& operator=(ControlObject&&) = delete;
    ControlObject& operator=(const ControlObject&) = delete;

    inline bool ignoreNops() const {
        return m_pControl ? m_pControl->ignoreNops() : true;
    }
};
