#ifndef EFFECTPARAMETERSLOTBASE_H
#define EFFECTPARAMETERSLOTBASE_H

#include <QObject>
#include <QVariant>
#include <QString>

#include "util.h"
#include "controlobject.h"
#include "effects/effect.h"

class ControlObject;
class ControlPushButton;

class EffectParameterSlotBase;
typedef QSharedPointer<EffectParameterSlotBase> EffectParameterSlotBasePointer;

class EffectParameterSlotBase : public QObject {
    Q_OBJECT
  public:
    EffectParameterSlotBase(const QString& group, const unsigned int iParameterSlotNumber);
    virtual ~EffectParameterSlotBase();

    static QString formatGroupString(const unsigned int iRackNumber,
                                     const unsigned int iChainNumber,
                                     const unsigned int iSlotNumber) {
        return QString("[EffectRack%1_EffectUnit%2_Effect%3]")
                .arg(QString::number(iRackNumber+1),
                     QString::number(iChainNumber+1),
                     QString::number(iSlotNumber+1));
    }

    QString name() const;
    QString description() const;
    const EffectManifestParameter getManifest();

  signals:
    // Signal that indicates that the EffectParameterSlotBase has been updated.
    void updated();

  protected slots:
    // Solely for handling control changes
    void slotLoaded(double v);
    void slotValueType(double v);

  protected:
    const unsigned int m_iParameterSlotNumber;
    QString m_group;
    EffectPointer m_pEffect;
    EffectParameter* m_pEffectParameter;

    // Controls exposed to the rest of Mixxx
    ControlObject* m_pControlLoaded;
    ControlObject* m_pControlType;
    double m_dChainParameter;

    DISALLOW_COPY_AND_ASSIGN(EffectParameterSlotBase);
};

#endif /* EFFECTPARAMETERSLOTBASE_H */
