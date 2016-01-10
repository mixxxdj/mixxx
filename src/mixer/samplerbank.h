#ifndef MIXER_SAMPLERBANK_H
#define MIXER_SAMPLERBANK_H

#include <QObject>

class ControlObject;
class PlayerManager;

class SamplerBank : public QObject {
    Q_OBJECT
  public:
    SamplerBank(PlayerManager* pPlayerManager);
    virtual ~SamplerBank();

  private slots:
    void slotSaveSamplerBank(double v);
    void slotLoadSamplerBank(double v);

  private:
    PlayerManager* m_pPlayerManager;
    ControlObject* m_pLoadControl;
    ControlObject* m_pSaveControl;
};

#endif /* MIXER_SAMPLERBANK_H */
