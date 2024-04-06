#pragma once

#import <AVFAudio/AVFAudio.h>

#include "effects/backends/effectmanifest.h"
#include "effects/defs.h"

class AudioUnitManifest : public EffectManifest {
  public:
    AudioUnitManifest(const QString& id, AVAudioUnitComponent* component);
};
