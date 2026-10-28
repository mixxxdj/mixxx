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
    AudioUnitBackend() : m_componentsById([NSMutableDictionary dictionary]) {
        loadAudioUnits();
    }

    ~AudioUnitBackend() override {
    }

    EffectBackendType getType() const override {
        return EffectBackendType::AudioUnit;
    };

    const QList<QString> getEffectIds() const override {
        // Only report effects whose manifest actually finished loading.
        // If loadAudioUnitsOfType() timed out, some components in
        // m_componentsById may not have a corresponding manifest yet, and
        // callers would get a null EffectManifestPointer from getManifest(),
        // which crashes downstream (e.g. EffectManifest::sortLexigraphically).
        return m_manifestsById.keys();
    }

    EffectManifestPointer getManifest(const QString& effectId) const override {
        return m_manifestsById.value(effectId);
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
    /// State shared between loadAudioUnits() and the GCD blocks loading the
    /// manifests. The blocks may still be running after loadAudioUnits() has
    /// timed out, or even after the backend has been destroyed, so they must
    /// only access this shared state and never `this`.
    struct ManifestLoadState {
        QMutex mutex;
        QHash<QString, EffectManifestPointer> manifestsById;
        bool loadingFinished = false;
    };

    NSMutableDictionary<NSString*, AVAudioUnitComponent*>* m_componentsById;
    // Only written in loadAudioUnits(), read-only afterwards.
    QHash<QString, EffectManifestPointer> m_manifestsById;

    void loadAudioUnits() {
        qDebug() << "Loading audio units...";

        // See
        // https://developer.apple.com/documentation/audiotoolbox/audio_unit_v3_plug-ins/incorporating_audio_effects_and_instruments?language=objc

        // Discover all AU components of both types first, then load all
        // manifests in a single parallel batch. This avoids the performance
        // penalty of two sequential discovery passes each with their own
        // blocking wait.
        auto manager =
                [AVAudioUnitComponentManager sharedAudioUnitComponentManager];

        NSMutableArray<AVAudioUnitComponent*>* allComponents =
                [NSMutableArray array];

        for (OSType componentType :
                {kAudioUnitType_Effect, kAudioUnitType_MusicEffect}) {
            AudioComponentDescription description = {
                    .componentType = componentType,
                    .componentSubType = 0,
                    .componentManufacturer = 0,
                    .componentFlags = 0,
                    .componentFlagsMask = 0,
            };
            auto components =
                    [manager componentsMatchingDescription:description];
            [allComponents addObjectsFromArray:components];
        }

        // Load component manifests (parameters etc.) concurrently since this
        // requires instantiating the corresponding Audio Units. We use Grand
        // Central Dispatch (GCD) for this instead of Qt's threading facilities
        // since GCD is a bit more lightweight and generally preferred for
        // Apple API-related stuff.
        dispatch_group_t group = dispatch_group_create();

        // Limit concurrent manifest loads to avoid exhausting the GCD thread
        // pool. Each manifest load blocks its thread in waitForAudioUnit for
        // up to 2 seconds, so without a limit, having 64+ AUs would hit
        // the macOS dispatch thread soft limit.
        const long MAX_CONCURRENT_LOADS = 8;
        dispatch_semaphore_t semaphore =
                dispatch_semaphore_create(MAX_CONCURRENT_LOADS);

        auto pLoadState = std::make_shared<ManifestLoadState>();

        for (AVAudioUnitComponent* component in allComponents) {
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

            // Note: The block must not capture `this`, see ManifestLoadState
            dispatch_group_async(group, queue, ^{
                dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);

                bool loadingFinished;
                {
                    auto locker = lockMutex(&pLoadState->mutex);
                    loadingFinished = pLoadState->loadingFinished;
                }

                // Don't even start loading if we already timed out
                if (!loadingFinished) {
                    // Load manifest (potentially slow blocking operation)
                    auto pManifest = EffectManifestPointer(
                            new AudioUnitManifest(effectId, component));

                    // Register manifest, unless we timed out in the meantime
                    auto locker = lockMutex(&pLoadState->mutex);
                    if (pLoadState->loadingFinished) {
                        qWarning() << "Audio Unit" << pManifest->name()
                                   << "finished loading after the timeout, "
                                      "ignoring it";
                    } else {
                        pLoadState->manifestsById.insert(effectId, pManifest);
                    }
                }

                // Every block must signal the semaphore exactly once,
                // otherwise libdispatch crashes when the semaphore is freed.
                dispatch_semaphore_signal(semaphore);
            });
        }

        const int64_t TIMEOUT_MS = 6000;

        qDebug() << "Waiting for audio unit manifests to be loaded...";
        const dispatch_time_t timeout =
                dispatch_time(DISPATCH_TIME_NOW, TIMEOUT_MS * NSEC_PER_MSEC);
        const bool timedOut = dispatch_group_wait(group, timeout) != 0;

        // Take a snapshot of the loaded manifests. Manifests that finish
        // loading after this point are ignored, so the set of effects
        // reported by this backend can't change after construction.
        auto locker = lockMutex(&pLoadState->mutex);
        pLoadState->loadingFinished = true;
        m_manifestsById = pLoadState->manifestsById;
        locker.unlock();

        if (timedOut) {
            qWarning() << "Timed out while loading audio unit manifests, only"
                       << m_manifestsById.size() << "of"
                       << static_cast<int>(allComponents.count)
                       << "Audio Units are available";
        } else {
            qDebug() << "Successfully loaded audio unit manifests";
        }
    }
};

EffectsBackendPointer createAudioUnitBackend() {
    return EffectsBackendPointer(new AudioUnitBackend());
}
