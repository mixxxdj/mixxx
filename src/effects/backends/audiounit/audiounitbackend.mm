#include "effects/backends/audiounit/audiounitbackend.h"

#import <AVFAudio/AVFAudio.h>
#import <AudioToolbox/AudioToolbox.h>
#import <Foundation/Foundation.h>

#include <QDebug>
#include <QHash>
#include <QList>
#include <QString>
#include <memory>

#include "effects/backends/audiounit/audiounitbackend.h"
#include "effects/backends/audiounit/audiouniteffectprocessor.h"
#include "effects/backends/audiounit/audiounitmanifest.h"
#include "effects/defs.h"

/// An effects backend for Audio Unit (AU) plugins. macOS-only.
class AudioUnitBackend : public EffectsBackend {
  public:
    AudioUnitBackend() : m_componentsById([[NSDictionary alloc] init]) {
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
    NSDictionary<NSString*, AVAudioUnitComponent*>* m_componentsById;
    QHash<QString, EffectManifestPointer> m_manifestsById;

    void loadAudioUnits() {
        qDebug() << "Loading audio units...";

        // See
        // https://developer.apple.com/documentation/audiotoolbox/audio_unit_v3_plug-ins/incorporating_audio_effects_and_instruments?language=objc

        // Create a query for audio components
        AudioComponentDescription description = {
                .componentType = kAudioUnitType_Effect,
                .componentSubType = 0,
                .componentManufacturer = 0,
                .componentFlags = 0,
                .componentFlagsMask = 0,
        };

        // Find the audio units
        // TODO: Should we perform this asynchronously (e.g. using Qt's
        // threading or GCD)?
        auto manager =
                [AVAudioUnitComponentManager sharedAudioUnitComponentManager];
        auto components = [manager componentsMatchingDescription:description];

        // Assign ids to the components
        NSMutableDictionary<NSString*, AVAudioUnitComponent*>* componentsById =
                [[NSMutableDictionary alloc] init];
        QHash<QString, EffectManifestPointer> manifestsById;

        for (AVAudioUnitComponent* component in components) {
            qDebug() << "Found audio unit" << [component name];

            QString effectId = QString::fromNSString(
                    [NSString stringWithFormat:@"%@~%@~%@",
                              [component manufacturerName],
                              [component name],
                              [component versionString]]);
            componentsById[effectId.toNSString()] = component;
            manifestsById[effectId] = EffectManifestPointer(
                    new AudioUnitManifest(effectId, component));
        }

        m_componentsById = componentsById;
        m_manifestsById = manifestsById;
    }
};

EffectsBackendPointer createAudioUnitBackend() {
    return EffectsBackendPointer(new AudioUnitBackend());
}
