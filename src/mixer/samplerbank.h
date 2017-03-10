#ifndef MIXER_SAMPLERBANK_H
#define MIXER_SAMPLERBANK_H

#include <QObject>

class ControlObject;
class ControlProxy;
class PlayerManager;

class SamplerBank : public QObject {
    Q_OBJECT
  public:
    SamplerBank(PlayerManager* pPlayerManager);
    virtual ~SamplerBank();

    bool saveSamplerBankToPath(const QString& samplerBankPath);
    bool loadSamplerBankFromPath(const QString& samplerBankPath);

  private slots:
    void slotSaveSamplerBank(double v);
    void slotLoadSamplerBank(double v);

  private:
    PlayerManager* m_pPlayerManager;
    ControlObject* m_pCOLoadBank;
    ControlObject* m_pCOSaveBank;
    ControlProxy* m_pCONumSamplers;
};

#endif /* MIXER_SAMPLERBANK_H */
