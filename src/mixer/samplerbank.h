#ifndef MIXER_SAMPLERBANK_H
#define MIXER_SAMPLERBANK_H

#include <QObject>

class ControlObject;
class ControlProxy;
class PlayerManager;

// TODO(Be): Replace saving/loading an XML file with saving/loading to the database.
//           That should be part of a larger project to implement
//           remix decks/sampler groups/whatever we end up calling them.
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
