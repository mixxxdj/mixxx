#ifndef EFFECTS_NATIVE_EQUALIZER_UTIL_H
#define EFFECTS_NATIVE_EQUALIZER_UTIL_H

#include <QObject>

#include "effects/effectmanifest.h"

class EqualizerUtil {
  public:
    // Creates common EQ parameters like low/mid/high gain and kill buttons.
    static void createCommonParameters(EffectManifest* manifest) {
        EffectManifestParameter* low = manifest->addParameter();
        low->setId("low");
        low->setName(QObject::tr("Low"));
        low->setDescription(QObject::tr("Gain for Low Filter"));
        low->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
        low->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
        low->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
        low->setNeutralPointOnScale(0.5);
        low->setDefault(1.0);
        low->setMinimum(0);
        low->setMaximum(4.0);

        EffectManifestParameter* killLow = manifest->addParameter();
        killLow->setId("killLow");
        killLow->setName(QObject::tr("Kill Low"));
        killLow->setDescription(QObject::tr("Kill the Low Filter"));
        killLow->setControlHint(EffectManifestParameter::ControlHint::TOGGLE_STEPPING);
        killLow->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
        killLow->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
        killLow->setDefault(0);
        killLow->setMinimum(0);
        killLow->setMaximum(1);

        EffectManifestParameter* mid = manifest->addParameter();
        mid->setId("mid");
        mid->setName(QObject::tr("Mid"));
        mid->setDescription(QObject::tr("Gain for Mid Filter"));
        mid->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
        mid->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
        mid->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
        mid->setNeutralPointOnScale(0.5);
        mid->setDefault(1.0);
        mid->setMinimum(0);
        mid->setMaximum(4.0);

        EffectManifestParameter* killMid = manifest->addParameter();
        killMid->setId("killMid");
        killMid->setName(QObject::tr("Kill Mid"));
        killMid->setDescription(QObject::tr("Kill the Mid Filter"));
        killMid->setControlHint(EffectManifestParameter::ControlHint::TOGGLE_STEPPING);
        killMid->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
        killMid->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
        killMid->setDefault(0);
        killMid->setMinimum(0);
        killMid->setMaximum(1);

        EffectManifestParameter* high = manifest->addParameter();
        high->setId("high");
        high->setName(QObject::tr("High"));
        high->setDescription(QObject::tr("Gain for High Filter"));
        high->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
        high->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
        high->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
        high->setNeutralPointOnScale(0.5);
        high->setDefault(1.0);
        high->setMinimum(0);
        high->setMaximum(4.0);

        EffectManifestParameter* killHigh = manifest->addParameter();
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
            "To adjust frequency shelves see the Equalizer preferences.");
    }
};


#endif /* EFFECTS_NATIVE_EQUALIZER_UTIL_H */
