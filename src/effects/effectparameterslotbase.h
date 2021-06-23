#pragma once

#include <QObject>
#include <QVariant>
#include <QString>

#include "control/controlobject.h"
#include "effects/effect.h"
#include "util/class.h"

class ControlObject;
class ControlPushButton;

class EffectParameterSlotBase : public QObject {
    Q_OBJECT
  public:
    EffectParameterSlotBase(const QString& group, const unsigned int iParameterSlotNumber);
    virtual ~EffectParameterSlotBase();

    QString name() const;
    QString shortName() const;
    QString description() const;
    EffectManifestParameterPointer getManifest();

    virtual QDomElement toXml(QDomDocument* doc) const = 0;
    virtual void loadParameterSlotFromXml(const QDomElement& parameterElement) = 0;

  signals:
    // Signal that indicates that the EffectParameterSlotBase has been updated.
    void updated();

  protected:
    const unsigned int m_iParameterSlotNumber;
    const QString m_group;
    EffectPointer m_pEffect;
    EffectParameter* m_pEffectParameter;

    // Controls exposed to the rest of Mixxx
    ControlObject* m_pControlLoaded;
    ControlObject* m_pControlType;
    double m_dChainParameter;

    DISALLOW_COPY_AND_ASSIGN(EffectParameterSlotBase);
};
