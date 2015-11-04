#ifndef EFFECTPARAMETERSLOT_H
#define EFFECTPARAMETERSLOT_H

#include <QObject>
#include <QVariant>
#include <QString>

#include "util.h"
#include "controlobject.h"
#include "effects/effect.h"

class ControlObject;
class ControlPushButton;
class ControlEffectKnob;

class EffectParameterSlot;
typedef QSharedPointer<EffectParameterSlot> EffectParameterSlotPointer;

class EffectParameterSlot : public QObject {
    Q_OBJECT
  public:
    EffectParameterSlot(const unsigned int iRackNumber,
                        const unsigned int iChainNumber,
                        const unsigned int iSlotNumber,
                        const unsigned int iParameterNumber);
    virtual ~EffectParameterSlot();

    static QString formatGroupString(const unsigned int iRackNumber,
                                     const unsigned int iChainNumber,
                                     const unsigned int iSlotNumber) {
        return QString("[EffectRack%1_EffectUnit%2_Effect%3]")
                .arg(QString::number(iRackNumber+1),
                     QString::number(iChainNumber+1),
                     QString::number(iSlotNumber+1));
    }

    // Load the parameter of the given effect into this EffectParameterSlot
    void loadEffect(EffectPointer pEffect);

    QString name() const;
    QString description() const;

  signals:
    // Signal that indicates that the EffectParameterSlot has been updated.
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
        return QString("EffectParameterSlot(%1,%2)").arg(m_group).arg(m_iParameterNumber);
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
    ControlEffectKnob* m_pControlValue;
    ControlObject* m_pControlType;

    DISALLOW_COPY_AND_ASSIGN(EffectParameterSlot);
};

#endif /* EFFECTPARAMETERSLOT_H */
