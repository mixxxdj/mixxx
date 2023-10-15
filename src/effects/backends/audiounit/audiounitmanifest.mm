#import <AudioToolbox/AudioToolbox.h>
#include "effects/backends/effectmanifestparameter.h"

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

    // Try instantiating the unit in-process to fetch its properties quickly

    AudioUnitManager manager{component, AudioUnitInstantiationType::Sync};
    AudioUnit audioUnit = manager.getAudioUnit();

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
        for (UInt32 i = 0; i < paramCount; i++) {
            AUDIO_UNIT_GET(audioUnit,
                    ParameterInfo,
                    Global,
                    paramIds[i],
                    &paramInfo,
                    &paramInfoSize);

            QString paramName = QString::fromUtf8(paramInfo.name);
            qDebug() << QString::fromNSString([component name])
                     << "has parameter" << paramName;

            if (paramInfo.flags & kAudioUnitParameterFlag_IsWritable) {
                EffectManifestParameterPointer manifestParam = addParameter();
                manifestParam->setId(paramName);
                manifestParam->setName(paramName);
                manifestParam->setRange(paramInfo.minValue,
                        paramInfo.defaultValue,
                        paramInfo.maxValue);
            }
        }
    }
}
