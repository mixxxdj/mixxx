/**
* @file dlgcontrollerlearning.cpp
* @author Sean M. Pappalardo  spappalardo@mixxx.org
* @date Thu 12 Apr 2012
* @brief The controller mapping learning wizard
*
*/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "dlgcontrollerlearning.h"
#include "mixxxcontrol.h"

DlgControllerLearning::DlgControllerLearning(QWidget * parent,
                                             ControllerPreset* preset)
    : QDialog(parent), Ui::DlgControllerLearning() {
    setupUi(this);
    m_pPreset = preset;
    labelMappedTo->setText("");
    iCurrentControl = 0;
    stackedWidget->setCurrentIndex(0); //Ensure the first page is always shown regardless
                                       //of the last page shown when the .ui file was saved.

    //Delete this dialog when its closed. We don't want any persistence.
    QWidget::setAttribute(Qt::WA_DeleteOnClose);

    m_pSkipShortcut = new QShortcut(QKeySequence(Qt::Key_Space), this);
    connect(m_pSkipShortcut, SIGNAL(triggered()), pushButtonSkip, SLOT(click()));
    //pushButtonSkip->setShortcut(QKeySequence(Qt::Key_Space));

    connect(pushButtonBegin, SIGNAL(clicked()), this, SLOT(begin()));
    connect(pushButtonSkip, SIGNAL(clicked()), this, SLOT(next()));
    connect(pushButtonPrevious, SIGNAL(clicked()), this, SLOT(prev()));
    connect(m_pPreset, SIGNAL(midiLearningFinished(QString)), this, SLOT(controlMapped(QString)));
    connect(pushButtonFinish, SIGNAL(clicked()), this, SLOT(close()));


    // Master Controls
    addControl("[Master]", "crossfader", tr("Crossfader"));
    addControl("[Master]", "volume", tr("Master volume"));
    addControl("[Master]", "balance", tr("Master balance"));
    addControl("[Master]", "headVolume", tr("Headphones volume"));
    addControl("[Master]", "headMix", tr("Headphones mix (pre/main)"));

    // Deck Controls
    addDeckControl("cue_default", tr("Cue button for Deck %1"));
    addDeckControl("play", tr("Play button for Deck %1"));
    addDeckControl("back", tr("Fast rewind button for Deck %1"));
    addDeckControl("fwd", tr("Fast forward button for Deck %1"));
    addDeckControl("reverse", tr("Play reverse button for Deck %1"));
    addDeckControl("pfl", tr("Headphone listen button for Deck %1"));
    addDeckControl("beatsync", tr("Beat sync button for Deck %1"));
    addDeckControl("bpm", tr("BPM tap button for Deck %1"));
    addDeckControl("keylock", tr("Keylock button for Deck %1"));
    addDeckControl("rate", tr("Pitch control slider for Deck %1"));
    addDeckControl("flanger", tr("Flanger effect button for Deck %1"));
    addDeckControl("volume", tr("Channel %1 volume fader"));
    addDeckControl("pregain", tr("Gain knob for Channel %1"));
    addDeckControl("filterHigh", tr("High EQ knob for Channel %1"));
    addDeckControl("filterMid", tr("Mid EQ knob for Channel %1"));
    addDeckControl("filterLow", tr("Low EQ knob for Channel %1"));
    addDeckControl("loop_in", tr("Loop In button for Deck %1"));
    addDeckControl("loop_out", tr("Loop Out button for Deck %1"));
    addDeckControl("reloop_exit", tr("Reloop / Exit button for Deck %1"));
    addDeckControl("beatloop_4", tr("Set up a loop over 4 beats for Deck %1"));
    addDeckControl("loop_halve", tr("Halves the current loop's length for Deck %1"));
    addDeckControl("loop_double", tr("Doubles the current loop's length for Deck %1"));
    addDeckControl("hotcue_1_activate", tr("Hotcue 1 button for Deck %1"));
    addDeckControl("hotcue_2_activate", tr("Hotcue 2 button for Deck %1"));
    addDeckControl("hotcue_3_activate", tr("Hotcue 3 button for Deck %1"));
    addDeckControl("hotcue_4_activate", tr("Hotcue 4 button for Deck %1"));
    addDeckControl("hotcue_1_clear", tr("Hotcue 1 delete button for Deck %1"));
    addDeckControl("hotcue_2_clear", tr("Hotcue 2 delete button for Deck %1"));
    addDeckControl("hotcue_3_clear", tr("Hotcue 3 delete button for Deck %1"));
    addDeckControl("hotcue_4_clear", tr("Hotcue 4 delete button for Deck %1"));

    addSamplerControl("play", tr("Play button for Sampler %1"));
    addSamplerControl("pregain", tr("Gain knob for Sampler %1"));
    addSamplerControl("pfl", tr("Headphone listen button for Sampler %1"));
    addSamplerControl("bpm", tr("BPM tap button for Sampler %1"));
    addSamplerControl("keylock", tr("Keylock button for Sampler %1"));
    addSamplerControl("rate", tr("Pitch control slider for Sampler %1"));
    addSamplerControl("hotcue_1_activate", tr("Hotcue 1 button for Sampler %1"));
    addSamplerControl("hotcue_2_activate", tr("Hotcue 2 button for Sampler %1"));
    addSamplerControl("hotcue_3_activate", tr("Hotcue 3 button for Sampler %1"));
    addSamplerControl("hotcue_4_activate", tr("Hotcue 4 button for Sampler %1"));
    addSamplerControl("hotcue_1_clear", tr("Hotcue 1 delete button for Sampler %1"));
    addSamplerControl("hotcue_2_clear", tr("Hotcue 2 delete button for Sampler %1"));
    addSamplerControl("hotcue_3_clear", tr("Hotcue 3 delete button for Sampler %1"));
    addSamplerControl("hotcue_4_clear", tr("Hotcue 4 delete button for Sampler %1"));

    // Library Controls
    addControl("[Playlist]", "SelectNextPlaylist", tr("Switch to the next view (library, playlist..)"));
    addControl("[Playlist]", "SelectPrevPlaylist", tr("Switch to the previous view (library, playlist..)"));
    addControl("[Playlist]", "SelectNextTrack", tr("Scroll to next track in library/playlist"));
    addControl("[Playlist]", "SelectPrevTrack", tr("Scroll to previous track in library/playlist"));
    addControl("[Playlist]", "LoadSelectedIntoFirstStopped", tr("Load selected track into first stopped player"));
    addDeckControl("LoadSelectedTrack", tr("Load selected track into Player %1"));

    // Flanger Controls
    addControl("[Flanger]", "lfoPeriod", tr("Adjusts the wavelength of the flange effect"));
    addControl("[Flanger]", "lfoDepth", tr("Adjusts the intensity of the flange effect"));
    addControl("[Flanger]", "lfoDelay", tr("Adjusts the phase delay of the flange effect"));

    // Microphone Controls
    addControl("[Microphone]", "talkover", tr("Microphone on/off"));
    addControl("[Microphone]", "volume", tr("Microphone volume"));
}

void DlgControllerLearning::addControl(QString group, QString control, QString description) {
    m_controlsToMap.append(MixxxControl(group, control, description));
}

void DlgControllerLearning::addDeckControl(QString control, QString description) {
    // TODO(rryan) get this from the PlayerManager
    const int iNumDecks = 2;
    for (int i = 1; i <= iNumDecks; ++i) {
        QString group = QString("[Channel%1]").arg(i);
        m_controlsToMap.append(MixxxControl(group, control, description.arg(i)));
    }
}

void DlgControllerLearning::addSamplerControl(QString control, QString description) {
    // TODO(rryan) get this from the PlayerManager
    const int iNumSamplers = 4;
    for (int i = 1; i <= iNumSamplers; ++i) {
        QString group = QString("[Sampler%1]").arg(i);
        m_controlsToMap.append(MixxxControl(group, control, description.arg(i)));
    }
}

DlgControllerLearning::~DlgControllerLearning()
{
    //If there was any ongoing learning, cancel it (benign if there wasn't).
    m_pPreset->cancelMidiLearn();

    delete m_pSkipShortcut;
}

void DlgControllerLearning::begin()
{
    //Switch pages in the stacked widget so that we show the mapping stuff.
    stackedWidget->setCurrentIndex(1);

    //Tell the MIDI mapping to start learning the first control.
    iCurrentControl = 0;
    MixxxControl control = m_controlsToMap[iCurrentControl];
    m_pPreset->beginMidiLearn(control);
    labelMixxxControl->setText(control.description());
}

void DlgControllerLearning::next()
{
    iCurrentControl++;
    if (iCurrentControl < m_controlsToMap.size())
    {

        MixxxControl control = m_controlsToMap[iCurrentControl];
        m_pPreset->beginMidiLearn(control);
        labelMixxxControl->setText(control.description());
        pushButtonSkip->setText(tr("Skip"));

        labelMappedTo->setText("");
        pushButtonPrevious->setEnabled(true);
    }
    else
    {
        //We've hit the end, show the congrats pane
        stackedWidget->setCurrentIndex(2);
    }
}

void DlgControllerLearning::prev()
{
    iCurrentControl--;
    if (iCurrentControl >= 0)
    {
        MixxxControl control = m_controlsToMap[iCurrentControl];
        m_pPreset->beginMidiLearn(control);
        labelMixxxControl->setText(control.description());
        pushButtonSkip->setText(tr("Skip"));
        labelMappedTo->setText("");

        //We've hit the start, don't let the user go back anymore.
        if (iCurrentControl == 0)
            pushButtonPrevious->setEnabled(false);
    }

}

/** Gets called when a control has just been mapped successfully */
void DlgControllerLearning::controlMapped(QString message)
{
    //QThread::sleep(1);

    labelMappedTo->setText(tr("Successfully mapped to: ") + message);

    //Set the label on the "Skip" button to "Next" because we're proceeding
    //instead of "skipping" now. Makes more sense, I think...
    pushButtonSkip->setText(tr("Next"));
    //next();
}
