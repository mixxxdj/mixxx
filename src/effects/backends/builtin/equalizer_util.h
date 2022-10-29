#pragma once

#include <QObject>

#include "control/pollingcontrolproxy.h"
#include "effects/backends/effectmanifest.h"

class EqualizerUtil {
  public:
    // Creates common EQ parameters like low/mid/high gain and kill buttons.
    static void createCommonParameters(EffectManifest* pManifest, bool linear) {
        EffectManifestParameter::ValueScaler valueScaler =
                EffectManifestParameter::ValueScaler::Logarithmic;
        double maximum = 4.0;
        if (linear) {
            valueScaler = EffectManifestParameter::ValueScaler::Linear;
            maximum = 2.0;
        }

        PollingControlProxy eqButtonMode(ConfigKey("[Mixer Profile]", "EQButtonMode"));

        EffectManifestParameterPointer low = pManifest->addParameter();
        low->setId("low");
        low->setName(QObject::tr("Low"));
        low->setDescription(QObject::tr("Gain for Low Filter"));
        low->setValueScaler(valueScaler);
        low->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
        low->setNeutralPointOnScale(0.5);
        low->setRange(0, 1.0, maximum);

        EffectManifestParameterPointer killLow = pManifest->addParameter();
        killLow->setId("killLow");
        killLow->setName(QObject::tr("Kill Low"));
        killLow->setDescription([eqButtonMode]() {
            return QObject::tr("Kill the Low Filter");
        });
        killLow->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
        killLow->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
        killLow->setRange(0, 0, 1);

        EffectManifestParameterPointer bypassLow = pManifest->addParameter();
        bypassLow->setId("bypassLow");
        bypassLow->setName(QObject::tr("Bypass Low"));
        bypassLow->setShortName(QObject::tr("Byp. Low"));
        bypassLow->setDescription([eqButtonMode]() {
            return QObject::tr("Bypass the Low Filter");
        });
        bypassLow->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
        bypassLow->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
        bypassLow->setRange(0, 0, 1);

        EffectManifestParameterPointer mid = pManifest->addParameter();
        mid->setId("mid");
        mid->setName(QObject::tr("Mid"));
        mid->setDescription(QObject::tr("Gain for Mid Filter"));
        mid->setValueScaler(valueScaler);
        mid->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
        mid->setNeutralPointOnScale(0.5);
        mid->setRange(0, 1.0, maximum);

        EffectManifestParameterPointer killMid = pManifest->addParameter();
        killMid->setId("killMid");
        killMid->setName(QObject::tr("Kill Mid"));
        killMid->setDescription([eqButtonMode]() {
            return QObject::tr("Kill the Mid Filter");
        });
        killMid->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
        killMid->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
        killMid->setRange(0, 0, 1);

        EffectManifestParameterPointer bypassMid = pManifest->addParameter();
        bypassMid->setId("bypassMid");
        bypassMid->setName(QObject::tr("Bypass Mid"));
        bypassMid->setShortName(QObject::tr("Byp. Mid"));
        bypassMid->setDescription([eqButtonMode]() {
            return QObject::tr("Bypass the Mid Filter");
        });
        bypassMid->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
        bypassMid->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
        bypassMid->setRange(0, 0, 1);

        EffectManifestParameterPointer high = pManifest->addParameter();
        high->setId("high");
        high->setName(QObject::tr("High"));
        high->setDescription(QObject::tr("Gain for High Filter"));
        high->setValueScaler(valueScaler);
        high->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
        high->setNeutralPointOnScale(0.5);
        high->setRange(0, 1.0, maximum);

        EffectManifestParameterPointer killHigh = pManifest->addParameter();
        killHigh->setId("killHigh");
        killHigh->setName(QObject::tr("Kill High"));
        killHigh->setDescription([eqButtonMode]() {
            return QObject::tr("Kill the High Filter");
        });
        killHigh->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
        killHigh->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
        killHigh->setRange(0, 0, 1);

        EffectManifestParameterPointer bypassHigh = pManifest->addParameter();
        bypassHigh->setId("bypassHigh");
        bypassHigh->setName(QObject::tr("Bypass High"));
        bypassHigh->setShortName(QObject::tr("Byp. Hi."));
        bypassHigh->setDescription([eqButtonMode]() {
            // TODO: do we need a closure here still?
            return QObject::tr("Bypass the High Filter");
        });
        bypassHigh->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
        bypassHigh->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
        bypassHigh->setRange(0, 0, 1);
    }

    static QString adjustFrequencyShelvesTip() {
        return QObject::tr(
                "To adjust frequency shelves, go to Preferences -> Equalizers.");
    }
};
