#pragma once

#include <QDataStream>
#include <QFile>

#include "audio/types.h"
#include "control/pollingcontrolproxy.h"
#include "encoder/encoder.h"
#include "encoder/encodercallback.h"
#include "engine/sidechain/sidechainworker.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"

class ControlProxy;

class EngineOsc : public QObject, public EncoderCallback, public SideChainWorker {
    Q_OBJECT
  public:
    EngineOsc(UserSettingsPointer pConfig);
    ~EngineOsc() override;

    void process(const CSAMPLE* pBuffer, const int iBufferSize) override;
    void shutdown() override {}

    // writes compressed audio to file
    void write(const unsigned char *header, const unsigned char *body, int headerLen, int bodyLen) override;
    // gets stream position
    int tell() override;
    // sets stream position
    void seek(int pos) override;
    // gets stream length
    int filelen()  override;

    // creates or opens an audio file
    // creates or opens an audio file
    bool openFile();
    // closes the audio file
    void closeFile();
    int updateFromPreferences();
    bool fileOpen();
    bool openCueFile();
    void closeCueFile();

  signals:
    // emitted to notify RecordingManager
    void bytesOsc(int bytes);

    // Emitted when recording state changes. 'recording' represents whether
    // recording is active and 'error' is true if an error occurred. Currently
    // only one error can occur: the specified file was unable to be opened for
    // writing.
    void isOsc(bool osc, bool error);
    void durationOsc(quint64 durationInt);

  private:
    int getActiveTracks();
    // Check if the metadata has changed since the previous check. We also check
    // when was the last check performed to avoid using too much CPU and as well
    // to avoid changing the metadata during scratches.
    bool metaDataHasChanged();

    void writeCueLine();

    UserSettingsPointer m_pConfig;
    EncoderPointer m_pEncoder;
    QString m_encoding;
    QString m_fileName;
    QString m_baTitle;
    QString m_baAuthor;
    QString m_baAlbum;

    QFile m_file;
    QFile m_cueFile;
    QDataStream m_dataStream;

    PollingControlProxy m_sampleRateControl;
    ControlProxy* m_pOscReady;
    quint64 m_frames;
    mixxx::audio::SampleRate m_sampleRate;
    quint64 m_oscDuration;
    QString getOscDurationStr();

    int m_iMetaDataLife;
    TrackPointer m_pCurrentTrack;

    QString m_cueFileName;
    quint64 m_cueTrack;
    bool m_bCueIsEnabled;
};
