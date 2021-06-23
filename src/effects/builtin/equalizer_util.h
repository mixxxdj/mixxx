#pragma once

#include <QObject>

#include "effects/effectmanifest.h"

class EqualizerUtil {
  public:
    // Creates common EQ parameters like low/mid/high gain and kill buttons.
    static void createCommonParameters(EffectManifest* pManifest, bool linear) {
        EffectManifestParameter::ControlHint controlHint =
                EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC;
        double maximum = 4.0;
        if (linear) {
            controlHint = EffectManifestParameter::ControlHint::KNOB_LINEAR;
            maximum = 2.0;
        }

        EffectManifestParameterPointer low = pManifest->addParameter();
        low->setId("low");
        low->setName(QObject::tr("Low"));
        low->setDescription(QObject::tr("Gain for Low Filter"));
        low->setControlHint(controlHint);
        low->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
        low->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
        low->setNeutralPointOnScale(0.5);
        low->setDefault(1.0);
        low->setMinimum(0);
        low->setMaximum(maximum);

        EffectManifestParameterPointer killLow = pManifest->addParameter();
        killLow->setId("killLow");
        killLow->setName(QObject::tr("Kill Low"));
        killLow->setDescription(QObject::tr("Kill the Low Filter"));
        killLow->setControlHint(EffectManifestParameter::ControlHint::TOGGLE_STEPPING);
        killLow->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
        killLow->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
        killLow->setDefault(0);
        killLow->setMinimum(0);
        killLow->setMaximum(1);

        EffectManifestParameterPointer mid = pManifest->addParameter();
        mid->setId("mid");
        mid->setName(QObject::tr("Mid"));
        mid->setDescription(QObject::tr("Gain for Mid Filter"));
        mid->setControlHint(controlHint);
        mid->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
        mid->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
        mid->setNeutralPointOnScale(0.5);
        mid->setDefault(1.0);
        mid->setMinimum(0);
        mid->setMaximum(maximum);

        EffectManifestParameterPointer killMid = pManifest->addParameter();
        killMid->setId("killMid");
        killMid->setName(QObject::tr("Kill Mid"));
        killMid->setDescription(QObject::tr("Kill the Mid Filter"));
        killMid->setControlHint(EffectManifestParameter::ControlHint::TOGGLE_STEPPING);
        killMid->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
        killMid->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
        killMid->setDefault(0);
        killMid->setMinimum(0);
        killMid->setMaximum(1);

        EffectManifestParameterPointer high = pManifest->addParameter();
        high->setId("high");
        high->setName(QObject::tr("High"));
        high->setDescription(QObject::tr("Gain for High Filter"));
        high->setControlHint(controlHint);
        high->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
        high->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
        high->setNeutralPointOnScale(0.5);
        high->setDefault(1.0);
        high->setMinimum(0);
        high->setMaximum(maximum);

        EffectManifestParameterPointer killHigh = pManifest->addParameter();
        killHigh->setId("killHigh");
        killHigh->setName(QObject::tr("Kill High"));
        killHigh->setDescription(QObject::tr("Kill the High Filter"));
        killHigh->setControlHint(EffectManifestParameter::ControlHint::TOGGLE_STEPPING);
        killHigh->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
        killHigh->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
        killHigh->setDefault(0);
        killHigh->setMinimum(0);
        killHigh->setMaximum(1);
    }

    static QString adjustFrequencyShelvesTip() {
        return QObject::tr(
            "To adjust frequency shelves, go to Preferences -> Equalizers.");
    }
};
