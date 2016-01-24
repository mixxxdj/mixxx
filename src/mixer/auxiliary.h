#ifndef MIXER_AUXILIARY_H
#define MIXER_AUXILIARY_H

#include <QObject>
#include <QString>

#include "effects/effectsmanager.h"
#include "engine/enginemaster.h"
#include "mixer/baseplayer.h"
#include "soundio/soundmanager.h"

class Auxiliary : public BasePlayer {
    Q_OBJECT
  public:
    Auxiliary(QObject* pParent,
              const QString& group,
              int index,
              std::shared_ptr<SoundManager> pSoundManager,
              std::shared_ptr<EngineMaster> pMixingEngine,
              std::shared_ptr<EffectsManager> pEffectsManager);
    virtual ~Auxiliary();
};

#endif /* MIXER_AUXILIARY_H */
