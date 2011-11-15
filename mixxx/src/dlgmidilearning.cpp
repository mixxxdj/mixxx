/***************************************************************************
                          dlgmidilearning.cpp  -  description
                             -------------------
    begin                : Mon March 2 2009
    copyright            : (C) 2009 by Albert Santoni
    email                : alberts at mixxx dot org
 ***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "dlgmidilearning.h"
#include "mixxxcontrol.h"
#include "midi/midimessage.h"
#include "midi/midimapping.h"

DlgMidiLearning::DlgMidiLearning(QWidget * parent, MidiMapping* mapping) :  QDialog(parent), Ui::DlgMidiLearning()
{
    setupUi(this);
    m_pMidiMapping = mapping;
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
    connect(m_pMidiMapping, SIGNAL(midiLearningFinished(MidiMessage)), this, SLOT(controlMapped(MidiMessage)));
    connect(pushButtonFinish, SIGNAL(clicked()), this, SLOT(close()));


    // Master Controls
    setupControl("[Master]", "crossfader", tr("Crossfader"));
    setupControl("[Master]", "volume", tr("Master volume"));
    setupControl("[Master]", "balance", tr("Master balance"));
    setupControl("[Master]", "headVolume", tr("Headphones volume"));
    setupControl("[Master]", "headMix", tr("Headphones mix (pre/main)"));

    // Deck Controls
    setupDeckControl("cue_default", tr("Cue button for Player %1"));
    setupDeckControl("play", tr("Play button for Player %1"));
    setupDeckControl("back", tr("Fast rewind button for Player %1"));
    setupDeckControl("fwd", tr("Fast forward button for Player %1"));
    setupDeckControl("reverse", tr("Play reverse button for Player %1"));
    setupDeckControl("pfl", tr("Headphone listen button for Player %1"));
    setupDeckControl("beatsync", tr("Beat sync button for Player %1"));
    setupDeckControl("bpm", tr("BPM tap button for Player %1"));
    setupDeckControl("keylock", tr("Keylock button for Player %1"));
    setupDeckControl("rate", tr("Pitch control slider for Player %1"));
    setupDeckControl("flanger", tr("Flanger effect button for Player %1"));
    setupDeckControl("volume", tr("Channel %1 volume fader"));
    setupDeckControl("pregain", tr("Gain knob for Channel %1"));
    setupDeckControl("filterHigh", tr("High EQ knob for Channel %1"));
    setupDeckControl("filterMid", tr("Mid EQ knob for Channel %1"));
    setupDeckControl("filterLow", tr("Low EQ knob for Channel %1"));
    setupDeckControl("loop_in", tr("Loop In button for Player %1"));
    setupDeckControl("loop_out", tr("Loop Out button for Player %1"));
    setupDeckControl("reloop_exit", tr("Reloop / Exit button for Player %1"));
    setupDeckControl("beatloop_4", tr("Setup a loop over 4 beats for Player %1"));
    setupDeckControl("loop_halve", tr("Halves the current loop's length for Player %1"));
    setupDeckControl("loop_double", tr("Doubles the current loop's length for Player %1"));
    setupDeckControl("hotcue_1_activate", tr("Hotcue 1 button for Player %1"));
    setupDeckControl("hotcue_2_activate", tr("Hotcue 2 button for Player %1"));
    setupDeckControl("hotcue_3_activate", tr("Hotcue 3 button for Player %1"));
    setupDeckControl("hotcue_4_activate", tr("Hotcue 4 button for Player %1"));
    setupDeckControl("hotcue_1_clear", tr("Hotcue 1 delete button for Player %1"));
    setupDeckControl("hotcue_2_clear", tr("Hotcue 2 delete button for Player %1"));
    setupDeckControl("hotcue_3_clear", tr("Hotcue 3 delete button for Player %1"));
    setupDeckControl("hotcue_4_clear", tr("Hotcue 4 delete button for Player %1"));

    // Library Controls
    setupControl("[Playlist]", "SelectNextPlaylist", tr("Switch to the next view (library, playlist..)"));
    setupControl("[Playlist]", "SelectPrevPlaylist", tr("Switch to the previous view (library, playlist..)"));
    setupControl("[Playlist]", "SelectNextTrack", tr("Scroll to next track in library/playlist"));
    setupControl("[Playlist]", "SelectPrevTrack", tr("Scroll to previous track in library/playlist"));
    setupControl("[Playlist]", "LoadSelectedIntoFirstStopped", tr("Load selected track into first stopped player"));
    setupDeckControl("LoadSelectedTrack", tr("Load selected track into Player %1"));

    // Flanger Controls
    setupControl("[Flanger]", "lfoPeriod", tr("Adjusts the wavelength of the flange effect"));
    setupControl("[Flanger]", "lfoDepth", tr("Adjusts the intensity of the flange effect"));
    setupControl("[Flanger]", "lfoDelay", tr("Adjusts the phase delay of the flange effect"));

    // Microphone Controls
    setupControl("[Microphone]", "talkover", tr("Microphone on/off"));
    setupControl("[Microphone]", "volume", tr("Microphone volume"));
}

void DlgMidiLearning::setupControl(QString group, QString control, QString description) {
    m_controlsToBind.append(qMakePair(MixxxControl(group, control), description));
}

void DlgMidiLearning::setupDeckControl(QString control, QString description) {
    // TODO(rryan) get this from the PlayerManager
    const int iNumDecks = 2;
    for (int i = 1; i <= iNumDecks; ++i) {
        QString group = QString("[Channel%1]").arg(i);
        m_controlsToBind.append(qMakePair(MixxxControl(group, control), description.arg(i)));
    }
}

DlgMidiLearning::~DlgMidiLearning()
{
    //If there was any ongoing learning, cancel it (benign if there wasn't).
    m_pMidiMapping->cancelMidiLearn();

    delete m_pSkipShortcut;
}

void DlgMidiLearning::begin()
{
    //Switch pages in the stacked widget so that we show the mapping stuff.
    stackedWidget->setCurrentIndex(1);

    //Tell the MIDI mapping to start learning the first control.
    iCurrentControl = 0;
    QPair<MixxxControl, QString>& control = m_controlsToBind[iCurrentControl];
    m_pMidiMapping->beginMidiLearn(control.first);
    labelMixxxControl->setText(control.second);
}

void DlgMidiLearning::next()
{
    iCurrentControl++;
    if (iCurrentControl < m_controlsToBind.size())
    {

        QPair<MixxxControl, QString>& control = m_controlsToBind[iCurrentControl];
        m_pMidiMapping->beginMidiLearn(control.first);
        labelMixxxControl->setText(control.second);
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

void DlgMidiLearning::prev()
{
    iCurrentControl--;
    if (iCurrentControl >= 0)
    {
        QPair<MixxxControl, QString>& control = m_controlsToBind[iCurrentControl];
        m_pMidiMapping->beginMidiLearn(control.first);
        labelMixxxControl->setText(control.second);
        pushButtonSkip->setText(tr("Skip"));
        labelMappedTo->setText("");

        //We've hit the start, don't let the user go back anymore.
        if (iCurrentControl == 0)
            pushButtonPrevious->setEnabled(false);
    }

}

/** Gets called when a control has just been mapped successfully */
void DlgMidiLearning::controlMapped(MidiMessage message)
{
    //QThread::sleep(1);

    labelMappedTo->setText(tr("Successfully mapped to: ") + message.toString());

    //Set the label on the "Skip" button to "Next" because we're proceeding
    //instead of "skipping" now. Makes more sense, I think...
    pushButtonSkip->setText(tr("Next"));
    //next();
}
