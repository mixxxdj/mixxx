#import <AudioToolbox/AudioToolbox.h>
#include "effects/backends/effectmanifestparameter.h"

#include <QElapsedTimer>
#include <QThread>
#include <memory>

#include "effects/backends/audiounit/audiounitmanager.h"
#include "effects/backends/audiounit/audiounitmanifest.h"
#include "effects/backends/audiounit/audiounitutils.h"
#include "effects/defs.h"

AudioUnitManifest::AudioUnitManifest(
        const QString& id, AVAudioUnitComponent* component) {
    setBackendType(EffectBackendType::AudioUnit);

    setId(id);
    setName(QString::fromNSString([component name]));
    setVersion(QString::fromNSString([component versionString]));
    setDescription(QString::fromNSString([component typeName]));
    setAuthor(QString::fromNSString([component manufacturerName]));

    // Instantiate audio unit (out-of-process) to load parameters
    AudioUnitManagerPointer pManager = AudioUnitManager::create(component);

    const int TIMEOUT_MS = 2000;
    if (!pManager->waitForAudioUnit(TIMEOUT_MS)) {
        qWarning() << name() << "took more than" << TIMEOUT_MS
                   << "ms to initialize, skipping manifest initialization "
                      "for this effect. This means this effect will not "
                      "display any parameters and likely not be useful!";
        return;
    }

    AudioUnit audioUnit = pManager->getAudioUnit();

    if (audioUnit) {
        // Fetch number of parameters
        UInt32 paramListBytes = 0;
        AUDIO_UNIT_INFO(audioUnit, ParameterList, Global, 0, &paramListBytes);

        // Fetch parameter ids
        UInt32 paramCount = paramListBytes / sizeof(AudioUnitParameterID);
        std::unique_ptr<AudioUnitParameterID[]> paramIds{
                new AudioUnitParameterID[paramCount]};
        AUDIO_UNIT_GET(audioUnit,
                ParameterList,
                Global,
                0,
                paramIds.get(),
                &paramListBytes);

        // Resolve parameters
        AudioUnitParameterInfo paramInfo;
        UInt32 paramInfoSize = sizeof(AudioUnitParameterInfo);
        bool hasLinkedParam = false;
        for (UInt32 i = 0; i < paramCount; i++) {
            AudioUnitParameterID paramId = paramIds[i];

            AUDIO_UNIT_GET(audioUnit,
                    ParameterInfo,
                    Global,
                    paramId,
                    &paramInfo,
                    &paramInfoSize);

            QString paramName = QString::fromUtf8(paramInfo.name);
            auto paramFlags = paramInfo.flags;

            qDebug() << QString::fromNSString([component name])
                     << "has parameter" << paramName;

            // TODO: Check CanRamp too?
            if (paramFlags & kAudioUnitParameterFlag_IsWritable) {
                EffectManifestParameterPointer manifestParam = addParameter();
                manifestParam->setId(QString::number(paramId));
                manifestParam->setName(paramName);
                manifestParam->setRange(paramInfo.minValue,
                        paramInfo.defaultValue,
                        paramInfo.maxValue);

                // Link the first parameter
                // TODO: Figure out if AU plugins provide a better way to figure
                // out the "default" parameter
                if (!hasLinkedParam) {
                    manifestParam->setDefaultLinkType(
                            EffectManifestParameter::LinkType::Linked);
                    hasLinkedParam = true;
                }

                // TODO: Support more modes, e.g. squared, square root in Mixxx
                if (paramFlags & kAudioUnitParameterFlag_DisplayLogarithmic) {
                    manifestParam->setValueScaler(
                            EffectManifestParameter::ValueScaler::Logarithmic);
                } else {
                    manifestParam->setValueScaler(
                            EffectManifestParameter::ValueScaler::Linear);
                }
            }
        }
    }
}
