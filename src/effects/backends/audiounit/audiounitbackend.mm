#include "effects/backends/audiounit/audiounitbackend.h"

#import <AVFAudio/AVFAudio.h>
#import <AudioToolbox/AudioToolbox.h>
#import <Foundation/Foundation.h>
#import <dispatch/dispatch.h>

#include <QDebug>
#include <QHash>
#include <QList>
#include <QMutex>
#include <QString>
#include <memory>

#include "effects/backends/audiounit/audiounitbackend.h"
#include "effects/backends/audiounit/audiouniteffectprocessor.h"
#include "effects/backends/audiounit/audiounitmanifest.h"
#include "effects/defs.h"
#include "util/compatibility/qmutex.h"

/// An effects backend for Audio Unit (AU) plugins. macOS-only.
class AudioUnitBackend : public EffectsBackend {
  public:
    AudioUnitBackend() : m_componentsById([[NSMutableDictionary alloc] init]) {
        loadAudioUnits();
    }

    ~AudioUnitBackend() override {
    }

    EffectBackendType getType() const override {
        return EffectBackendType::AudioUnit;
    };

    const QList<QString> getEffectIds() const override {
        QList<QString> effectIds;

        for (NSString* effectId in m_componentsById) {
            effectIds.append(QString::fromNSString(effectId));
        }

        return effectIds;
    }

    EffectManifestPointer getManifest(const QString& effectId) const override {
        return m_manifestsById[effectId];
    }

    const QList<EffectManifestPointer> getManifests() const override {
        return m_manifestsById.values();
    }

    bool canInstantiateEffect(const QString& effectId) const override {
        return [m_componentsById objectForKey:effectId.toNSString()] != nil;
    }

    std::unique_ptr<EffectProcessor> createProcessor(
            const EffectManifestPointer pManifest) const override {
        AVAudioUnitComponent* component =
                m_componentsById[pManifest->id().toNSString()];
        return std::make_unique<AudioUnitEffectProcessor>(component);
    }

  private:
    NSMutableDictionary<NSString*, AVAudioUnitComponent*>* m_componentsById;
    QHash<QString, EffectManifestPointer> m_manifestsById;
    QMutex m_mutex;

    void loadAudioUnits() {
        qDebug() << "Loading audio units...";

        loadAudioUnitsOfType(kAudioUnitType_Effect);
        loadAudioUnitsOfType(kAudioUnitType_MusicEffect);
    }

    void loadAudioUnitsOfType(OSType componentType) {
        // See
        // https://developer.apple.com/documentation/audiotoolbox/audio_unit_v3_plug-ins/incorporating_audio_effects_and_instruments?language=objc

        // Create a query for audio components
        AudioComponentDescription description = {
                .componentType = componentType,
                .componentSubType = 0,
                .componentManufacturer = 0,
                .componentFlags = 0,
                .componentFlagsMask = 0,
        };

        // Find the audio units
        auto manager =
                [AVAudioUnitComponentManager sharedAudioUnitComponentManager];
        auto components = [manager componentsMatchingDescription:description];

        // Load component manifests (parameters etc.) concurrently since this
        // requires instantiating the corresponding Audio Units. We use Grand
        // Central Dispatch (GCD) for this instead of Qt's threading facilities
        // since GCD is a bit more lightweight and generally preferred for
        // Apple API-related stuff.

        dispatch_group_t group = dispatch_group_create();

        for (AVAudioUnitComponent* component in components) {
            qDebug() << "Found audio unit" << [component name];

            QString effectId = QString::fromNSString(
                    [NSString stringWithFormat:@"%@~%@~%@",
                            [component manufacturerName],
                            [component name],
                            [component versionString]]);

            // Register component
            m_componentsById[effectId.toNSString()] = component;

            // Use a concurrent background GCD queue to load manifest
            dispatch_queue_t queue = dispatch_get_global_queue(
                    DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);

            dispatch_group_async(group, queue, ^{
                // Load manifest (potentially slow blocking operation)
                auto manifest = EffectManifestPointer(
                        new AudioUnitManifest(effectId, component));

                // Register manifest
                auto locker = lockMutex(&m_mutex);
                m_manifestsById[effectId] = manifest;
            });
        }

        const int64_t TIMEOUT_MS = 6000;

        qDebug() << "Waiting for audio unit manifests to be loaded...";
        if (dispatch_group_wait(group,
                    dispatch_time(DISPATCH_TIME_NOW, TIMEOUT_MS * 1000000)) ==
                0) {
            qDebug() << "Successfully loaded audio unit manifests";
        } else {
            qWarning() << "Timed out while loading audio unit manifests";
        }
    }
};

EffectsBackendPointer createAudioUnitBackend() {
    return EffectsBackendPointer(new AudioUnitBackend());
}
