// BeatControl.cpp
// Author: pwhelan

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
#include "track/beats.h"


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
    EngineControl(_group, _config)
{
    m_pReader = reader;
    m_iCurrentSample = 0.;
    m_smBeatLoop = new QSignalMapper(this);

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

    // Piggy back on top of the existent loop control for this deck.
    m_pCOLoopStart = ControlObject::getControl(ConfigKey(_group, "loop_start_position"));
    m_pCOLoopEnd = ControlObject::getControl(ConfigKey(_group, "loop_end_position"));
    m_pCOLoopEnabled = ControlObject::getControl(ConfigKey(_group, "loop_enabled"));
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
}

void BeatControl::slotTrackLoaded(TrackPointer tio, 
                            int iTrackSampleRate, int iTrackNumSamples)
{
    m_pTrack = tio;
    m_pBeats = m_pTrack->getBeats();

    connect(m_pTrack.data(), SIGNAL(beatsUpdated()),
                    this, SLOT(slotUpdatedTrackBeats()));
}

void BeatControl::slotUpdatedTrackBeats()
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
        loop_in = m_pBeats->findNthBeat(m_iCurrentSample, -1);
        if ( beats >= 1 )
            loop_out = m_pBeats->findNthBeat(m_iCurrentSample, (int)floor(beats));
        else
        {
            loop_out = m_pBeats->findNthBeat(m_iCurrentSample, 1);
            loop_out = loop_in + ((loop_out - loop_in) * beats);
        }
    }
    // For negative numbers we start from the beat after us and start the loop
    // around X beats before there.
    else
    {
        loop_out = m_pBeats->findNthBeat(m_iCurrentSample, 0);
        if ( beats <= -1 )
            loop_in = m_pBeats->findNthBeat(m_iCurrentSample, (int)floor(beats));
        else
        {
            loop_in = m_pBeats->findNthBeat(m_iCurrentSample, 1);
            loop_in += ((loop_out - loop_in) * beats);
        }
    }

    qDebug() << "Current:" << m_iCurrentSample << "IN:" << (loop_in) << "OUT:" << (loop_out);

    // TrackBeats generates mono sample numbers, so we have to account 
    // for that here. Probably should just generate stereo sample numbers
    // though, from the get go 
    // - Phillip Whelan
    m_pCOLoopStart->set((double)loop_in);
    m_pCOLoopEnd->set((double)loop_out);
    m_pCOLoopEnabled->set(1);
}

void BeatControl::slotBeatLoopSize(int i)
{
    return slotBeatLoop(s_dBeatSizes[i]);
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

    return currentSample;
}

