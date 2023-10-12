#include "effects/backends/au/aubackend.h"

#import <AVFAudio/AVFAudio.h>
#import <AudioToolbox/AudioToolbox.h>
#import <Foundation/Foundation.h>

#include <QDebug>
#include <QHash>
#include <QList>
#include <QString>
#include <memory>

#include "effects/backends/au/aubackend.h"
#include "effects/backends/au/aueffectprocessor.h"
#include "effects/backends/au/aumanifest.h"
#include "effects/defs.h"

/// An effects backend for Audio Unit (AU) plugins. macOS-only.
class AUBackend : public EffectsBackend {
  public:
    AUBackend() : m_componentsById([[NSDictionary alloc] init]), m_nextId(0) {
        loadAudioUnits();
    }

    ~AUBackend() override {
    }

    EffectBackendType getType() const override {
        return EffectBackendType::AU;
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
        return std::make_unique<AUEffectProcessor>(pManifest);
    }

  private:
    NSDictionary<NSString*, AVAudioUnitComponent*>* m_componentsById;
    QHash<QString, EffectManifestPointer> m_manifestsById;
    int m_nextId;

    QString freshId() {
        return QString::number(m_nextId++);
    }

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

            QString effectId = freshId();
            componentsById[effectId.toNSString()] = component;
            manifestsById[effectId] =
                    EffectManifestPointer(new AUManifest(effectId, component));
        }

        m_componentsById = componentsById;
        m_manifestsById = manifestsById;
    }
};

EffectsBackendPointer createAUBackend() {
    return EffectsBackendPointer(new AUBackend());
}
