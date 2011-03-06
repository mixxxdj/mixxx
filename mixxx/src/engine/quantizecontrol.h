#ifndef __QUANTIZE_CONTROL__
#define __QUANTIZE_CONTROL__

#include <QObject>
#include <QSignalMapper>

#include "configobject.h"
#include "engine/enginecontrol.h"

#include "trackinfoobject.h"
#include "track/beats.h"

class ControlObject;
class ControlPushButton;
class ControlObjectThread;
class CachingReader;


class QuantizeControl : public EngineControl {
    Q_OBJECT
public:
    QuantizeControl(const char * _group, ConfigObject<ConfigValue> * _config, CachingReader *);
    virtual ~QuantizeControl();
    double process(const double dRate,
                   const double currentSample,
                   const double totalSamples,
                   const int iBufferSize);

public slots:
    void slotTrackLoaded(TrackPointer tio, int iSampleRate, int iNumSamples);
    void slotBeatsUpdated();

private:
    int m_iCurrentSample;
    double m_dQuantizePrevBeat;

    ControlPushButton* m_pCOQuantizeEnabled;
    ControlObject* m_pCOQuantizeBeat;
    
    CachingReader *m_pReader;

    TrackPointer m_pTrack;
    BeatsPointer m_pBeats;
};


#endif // __QUANTIZE_CONTROL__

