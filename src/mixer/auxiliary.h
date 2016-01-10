#ifndef MIXER_AUXILIARY_H
#define MIXER_AUXILIARY_H

#include <QObject>
#include <QString>

#include "mixer/baseplayer.h"

class EffectsManager;
class EngineMaster;
class SoundManager;

class Auxiliary : public BasePlayer {
    Q_OBJECT
  public:
    Auxiliary(QObject* pParent,
              const QString& group,
              int index,
              SoundManager* pSoundManager,
              EngineMaster* pMixingEngine,
              EffectsManager* pEffectsManager);
    virtual ~Auxiliary();
};

#endif /* MIXER_AUXILIARY_H */
