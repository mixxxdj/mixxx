#ifndef EFFECTBUTTONPARAMETERSLOT_H
#define EFFECTBUTTONPARAMETERSLOT_H

#include <QObject>
#include <QVariant>
#include <QString>

#include "util.h"
#include "controlobject.h"
#include "effects/effect.h"

class ControlObject;
class ControlPushButton;
class ControlEffectKnob;

class EffectButtonParameterSlot;
typedef QSharedPointer<EffectButtonParameterSlot> EffectButtonParameterSlotPointer;

class EffectButtonParameterSlot : public QObject {
    Q_OBJECT
  public:
    EffectButtonParameterSlot(const unsigned int iRackNumber,
                        const unsigned int iChainNumber,
                        const unsigned int iSlotNumber,
                        const unsigned int iParameterNumber);
    virtual ~EffectButtonParameterSlot();

    static QString formatGroupString(const unsigned int iRackNumber,
                                     const unsigned int iChainNumber,
                                     const unsigned int iSlotNumber) {
        return QString("[EffectRack%1_EffectUnit%2_Effect%3]")
                .arg(QString::number(iRackNumber+1),
                     QString::number(iChainNumber+1),
                     QString::number(iSlotNumber+1));
    }

    static QString formatItemPrefix(const unsigned int iParameterNumber) {
        return QString("button_parameter%1").arg(iParameterNumber + 1);
    }

    // Load the parameter of the given effect into this EffectButtonParameterSlot
    void loadEffect(EffectPointer pEffect);

    QString name() const;
    QString description() const;

    void onChainParameterChanged(double parameter);

  signals:
    // Signal that indicates that the EffectButtonParameterSlot has been updated.
    void updated();

  private slots:
    // Solely for handling control changes
    void slotLoaded(double v);
    void slotLinkType(double v);
    void slotValueChanged(double v);
    void slotValueType(double v);

    void slotParameterValueChanged(QVariant value);

  private:
    QString debugString() const {
        return QString("EffectButtonParameterSlot(%1,%2)").arg(m_group).arg(m_iParameterNumber);
    }

    // Clear the currently loaded effect
    void clear();

    const unsigned int m_iRackNumber;
    const unsigned int m_iChainNumber;
    const unsigned int m_iSlotNumber;
    const unsigned int m_iParameterNumber;
    const QString m_group;
    EffectPointer m_pEffect;
    EffectParameter* m_pEffectParameter;

    ////////////////////////////////////////////////////////////////////////////////
    // Controls exposed to the rest of Mixxx
    ////////////////////////////////////////////////////////////////////////////////

    ControlObject* m_pControlLoaded;
    ControlPushButton* m_pControlLinkType;
    ControlPushButton* m_pControlValue;
    ControlObject* m_pControlType;
    double m_dChainParameter;

    DISALLOW_COPY_AND_ASSIGN(EffectButtonParameterSlot);
};

#endif // EFFECTBUTTONPARAMETERSLOT_H
