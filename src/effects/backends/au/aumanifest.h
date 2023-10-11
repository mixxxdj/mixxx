#pragma once

#import <AVFAudio/AVFAudio.h>

#include "effects/backends/effectmanifest.h"
#include "effects/defs.h"

class AUManifest : public EffectManifest {
  public:
    AUManifest(AVAudioUnitComponent* component);

  private:
    AVAudioUnitComponent* m_component;
};
