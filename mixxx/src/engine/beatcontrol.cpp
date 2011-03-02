#include <QtDebug>
#include <QObject>
#include <QSignalMapper>

#include "controlobject.h"
#include "configobject.h"
#include "controlobjectthread.h"
#include "controlpushbutton.h"
#include "engine/loopingcontrol.h"
#include "engine/enginecontrol.h"
#include "engine/beatcontrol.h"
#include "mathstuff.h"
#include "cachingreader.h"

#include "trackinfoobject.h"
#include "beats.h"


double BeatControl::s_dBeatSizes[] = { 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64, -1 };

// Used simply to generate the beatloop_%SIZE and beatseek_%SIZE CO ConfigKeys.
ConfigKey BeatControl::keyForControl(const char *_group, QString ctrlName, double num) {
    ConfigKey key;
    key.group = _group;
    key.item = QString("%1_%2").arg(ctrlName).arg(num);
    return key;
}


BeatControl::BeatControl(const char *_group, 
                    ConfigObject<ConfigValue> * _config, CachingReader *reader, double beats) : 
    EngineControl(_group, _config),
    m_iNextJump(-1),
    m_iJumpBeat(0)
{
    m_pReader = reader;
    m_iCurrentSample = 0.;


    m_smBeatLoop = new QSignalMapper(this);
    m_smBeatSeek = new QSignalMapper(this);
    int i;


    // Connect to the trackLoaded signal so we can update beat information.
    connect(m_pReader, SIGNAL(trackLoaded(TrackPointer, int, int)),
            this, SLOT(slotTrackLoaded(TrackPointer, int, int)),
            Qt::DirectConnection);

    // Connect beatloop, which can flexibly handle different values.
    // Using this CO directly is meant more for internal and script use.
    m_pCOBeatLoop = new ControlObject(ConfigKey(_group, "beatloop"), 0);
    connect(m_pCOBeatLoop, SIGNAL(valueChanged(double)), this, 
            SLOT(slotBeatLoop(double)),
            Qt::DirectConnection);

    // Here we create corresponding beatloop_(SIZE) CO's which all call the same
    // BeatControl, but with a set value; all thanks to QSignalMapper.
    for (i = 0; s_dBeatSizes[i] > 0; i++) {
        ControlPushButton* coBeatloop;
        coBeatloop = new ControlPushButton(keyForControl(_group, "beatloop", s_dBeatSizes[i]));

        connect(coBeatloop, SIGNAL(valueChanged(double)),
                m_smBeatLoop, SLOT(map()));

        m_smBeatLoop->setMapping(coBeatloop, i);
        m_pCOBeatLoops.append(coBeatloop);
    }

    // Connect the Signal Mapper, and here we complete the magic...
    connect(m_smBeatLoop, SIGNAL(mapped(int)), this, 
            SLOT(slotBeatLoopSize(int)),
            Qt::DirectConnection);

    // Connect beatseek, which can flexibly handle different values.
    // Using this CO directly is meant more for internal and script use.
    m_pCOBeatSeek = new ControlObject(ConfigKey(_group, "beatseek"), 0);
    connect(m_pCOBeatSeek, SIGNAL(valueChanged(double)), this, 
            SLOT(slotBeatSeek(double)),
            Qt::DirectConnection);

    // Here we create corresponding beatseek_(SIZE) CO's which all call the same
    // BeatControl, but with a set value (all thanks to QSignalMapper).
    for (i = 0; s_dBeatSizes[i] > 0; i++) {
        ControlPushButton* coBeatseek;
        coBeatseek = new ControlPushButton(keyForControl(_group, "beatseek", s_dBeatSizes[i]));
        
        connect(coBeatseek, SIGNAL(valueChanged(double)), m_smBeatSeek, 
                SLOT(map()));

        m_smBeatSeek->setMapping(coBeatseek, i);
        m_pCOBeatSeeks.append(coBeatseek);
    }

    // Connect the Signal Mapper, and here we complete the magic...
    connect(m_smBeatSeek, SIGNAL(mapped(int)), 
            this, SLOT(slotBeatSeekSize(int)),
            Qt::DirectConnection);

    // Piggy back on top of the existent loop control for this deck.
    m_pCOLoopStart = new ControlObjectThread(ControlObject::getControl(ConfigKey(_group, "loop_start_position")));
    m_pCOLoopEnd = new ControlObjectThread(ControlObject::getControl(ConfigKey(_group, "loop_end_position")));
    m_pCOLoopEnabled = new ControlObjectThread(ControlObject::getControl(ConfigKey(_group, "reloop_exit")));
}

BeatControl::~BeatControl()
{
    ControlPushButton* co;

    delete m_pCOBeatLoop;

    delete m_smBeatLoop;
    while (m_pCOBeatLoops.size() > 0)
    {
        co = m_pCOBeatLoops.takeLast();
        delete co;
    }

    delete m_smBeatSeek;
    while (m_pCOBeatSeeks.size() > 0)
    {
        co = m_pCOBeatSeeks.takeLast();
        delete co;
    }
}

void BeatControl::slotTrackLoaded(TrackPointer tio, 
                            int iTrackSampleRate, int iTrackNumSamples)
{
    m_pTrack = tio;
    m_pBeats = m_pTrack->getBeats();

    connect(m_pTrack.data(), SIGNAL(trackBeatsUpdated(int)),
                    this, SLOT(beatsUpdated(int)));
}

void BeatControl::slotUpdatedTrackBeats(int updated)
{
    m_pBeats = m_pTrack->getBeats();
    if ( !m_pBeats )
        return;
}

// Generate a loop of 'beats' length. It can also do fractions for a beatslicing
// effect.
void BeatControl::slotBeatLoop(double beats)
{
    int loop_in;
    int loop_out;


    if ( ! m_pBeats ) {
        qDebug() << "BeatLoop: No Beats to work with";
        return;
    }

    // For positive numbers we start from the beat before us and create the loop
    // around X beats from there.
    if ( beats > 0 )
    {
        loop_in = m_pBeats->findNthBeat(m_iCurrentSample/2, -1);
        if ( beats >= 1 )
            loop_out = m_pBeats->findNthBeat(m_iCurrentSample/2, (int)floor(beats));
        else
        {
            loop_out = m_pBeats->findNthBeat(m_iCurrentSample/2, 1);
            loop_out = loop_in + ((loop_out - loop_in) * beats);
        }
    }
    // For negative numbers we start from the beat after us and start the loop
    // around X beats before there.
    else
    {
        loop_out = m_pBeats->findNthBeat(m_iCurrentSample/2, 0);
        if ( beats <= -1 )
            loop_in = m_pBeats->findNthBeat(m_iCurrentSample/2, (int)floor(beats));
        else
        {
            loop_in = m_pBeats->findNthBeat(m_iCurrentSample/2, 1);
            loop_in += ((loop_out - loop_in) * beats);
        }
    }

    qDebug() << "Current:" << m_iCurrentSample << "IN:" << (loop_in*2) << "OUT:" << (loop_out*2);

    // TrackBeats generates mono sample numbers, so we have to account 
    // for that here. Probably should just generate stereo sample numbers
    // though, from the get go 
    // - Phillip Whelan
    m_pCOLoopStart->slotSet((double)loop_in * 2);
    m_pCOLoopEnd->slotSet((double)loop_out * 2);
    m_pCOLoopEnabled->slotSet(1);
}

void BeatControl::slotBeatLoopSize(int i)
{
    return slotBeatLoop(s_dBeatSizes[i]);
}

// Jump a number of beats back or forward as soon as we hit the next beat.
// ... Really lousy way to implement quantization.
void BeatControl::slotBeatSeek(double beats)
{
    if ( m_pBeats == NULL ) {
        qDebug() << "BeatSeek: No Beats to work with";
        return;
    }

    // Reset and turn off if we are passed 0
    if ( beats == 0 ) {
        m_iNextJump = -1;
        m_iJumpBeat = 0;
        
        return;
    }

    // Set some properties to refer to inside BeatControl::process()
    m_iNextJump = (double)m_pBeats->findNthBeat(m_iCurrentSample, 0);
    m_iJumpBeat = (double)m_pBeats->findNthBeat(m_iNextJump, (int)floor(beats));
}

void BeatControl::slotBeatSeekSize(int i)
{
    return slotBeatSeek(s_dBeatSizes[i]);
}

double BeatControl::process(const double dRate,
                               const double currentSample,
                               const double totalSamples,
                               const int iBufferSize)
{
    bool reverse = dRate < 0;


    m_iCurrentSample = (int)round(currentSample);
    if (!even(m_iCurrentSample))
        m_iCurrentSample--;
    
    if ( m_iNextJump >= 0 && 0) {
        if ( m_iCurrentSample >= m_iNextJump ) {

            // Do not Jump outside of an active loop
            if (m_pCOLoopEnabled->get() && (m_iNextJump > m_pCOLoopEnd->get()))
                return currentSample;

            m_iNextJump = -1;
            return m_iJumpBeat;
        }
    }

    return currentSample;
}
