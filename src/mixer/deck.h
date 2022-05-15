#pragma once

#include <QObject>

#include "mixer/basetrackplayer.h"
#include "tensorflow/c/c_api.h"

class Deck : public BaseTrackPlayerImpl {
    Q_OBJECT
  public:
    Deck(PlayerManager* pParent,
            UserSettingsPointer pConfig,
            EngineMaster* pMixingEngine,
            EffectsManager* pEffectsManager,
            EngineChannel::ChannelOrientation defaultOrientation,
            const ChannelHandleAndGroup& handleGroup);
    ~Deck() override;
  private slots:
    void slotStemEnabled(double v);
  private:
    ControlObject* m_pStemControl;
    QString deckName;
    void Deallocator(void* data, size_t length, void* arg);
    TF_Status* status = TF_NewStatus();
    TF_Graph* graph = TF_NewGraph();
    TF_SessionOptions* SessionOpts = TF_NewSessionOptions();
    TF_Buffer* RunOpts = NULL;
    TF_Session* session;

    const char* tags = "serve";
    int ntags = 1;
    int num_dims = 2;
};
