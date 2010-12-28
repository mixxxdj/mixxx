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

    m_controlsToBind.append(MixxxControl("[Master]", "crossfader"));
    m_controlDescriptions.append("Crossfader");

    m_controlsToBind.append(MixxxControl("[Master]", "volume"));
    m_controlDescriptions.append("Master volume");

    m_controlsToBind.append(MixxxControl("[Master]", "headVolume"));
    m_controlDescriptions.append("Headphones volume");

    m_controlsToBind.append(MixxxControl("[Master]", "headMix"));
    m_controlDescriptions.append("Headphones mix (pre/main)");

    m_controlsToBind.append(MixxxControl("[Channel1]", "play"));
    m_controlDescriptions.append("Play button for Player 1");

    m_controlsToBind.append(MixxxControl("[Channel2]", "play"));
    m_controlDescriptions.append("Play button for Player 2");

    m_controlsToBind.append(MixxxControl("[Channel1]", "back"));
    m_controlDescriptions.append("Rewind button for Player 1");

    m_controlsToBind.append(MixxxControl("[Channel2]", "back"));
    m_controlDescriptions.append("Rewind button for Player 2");

    m_controlsToBind.append(MixxxControl("[Channel1]", "fwd"));
    m_controlDescriptions.append("Seek forwards button for Player 1");

    m_controlsToBind.append(MixxxControl("[Channel2]", "fwd"));
    m_controlDescriptions.append("Seek forward button for Player 2");

    m_controlsToBind.append(MixxxControl("[Channel1]", "cue_default"));
    m_controlDescriptions.append("Cue button for Player 1");

    m_controlsToBind.append(MixxxControl("[Channel2]", "cue_default"));
    m_controlDescriptions.append("Cue button for Player 2");

    m_controlsToBind.append(MixxxControl("[Channel1]", "volume"));
    m_controlDescriptions.append("Channel 1 volume");

    m_controlsToBind.append(MixxxControl("[Channel2]", "volume"));
    m_controlDescriptions.append("Channel 2 volume");

    m_controlsToBind.append(MixxxControl("[Channel1]", "rate"));
    m_controlDescriptions.append("Pitch control for Player 1");

    m_controlsToBind.append(MixxxControl("[Channel2]", "rate"));
    m_controlDescriptions.append("Pitch control for Player 2");

    m_controlsToBind.append(MixxxControl("[Channel1]", "pfl"));
    m_controlDescriptions.append("Headphone listen button for Player 1");

    m_controlsToBind.append(MixxxControl("[Channel2]", "pfl"));
    m_controlDescriptions.append("Headphone listen button for Player 2");

    m_controlsToBind.append(MixxxControl("[Channel1]", "filterLow"));
    m_controlDescriptions.append("Low EQ knob for Channel 1");

    m_controlsToBind.append(MixxxControl("[Channel2]", "filterLow"));
    m_controlDescriptions.append("Low EQ knob for Channel 2");

    m_controlsToBind.append(MixxxControl("[Channel1]", "filterMid"));
    m_controlDescriptions.append("Mid EQ knob for Channel 1");

    m_controlsToBind.append(MixxxControl("[Channel2]", "filterMid"));
    m_controlDescriptions.append("Mid EQ knob for Channel 2");

    m_controlsToBind.append(MixxxControl("[Channel1]", "filterHigh"));
    m_controlDescriptions.append("High EQ knob for Channel 1");

    m_controlsToBind.append(MixxxControl("[Channel2]", "filterHigh"));
    m_controlDescriptions.append("High EQ knob for Channel 2");
    
    m_controlsToBind.append(MixxxControl("[Channel1]", "pregain"));
    m_controlDescriptions.append("Gain for Channel 1");

    m_controlsToBind.append(MixxxControl("[Channel2]", "pregain"));
    m_controlDescriptions.append("Gain for Channel 2");
    
    m_controlsToBind.append(MixxxControl("[Channel1]", "loop_in"));
    m_controlDescriptions.append("Loop In for Player 1");

    m_controlsToBind.append(MixxxControl("[Channel2]", "loop_in"));
    m_controlDescriptions.append("Loop In for Player 2");

    m_controlsToBind.append(MixxxControl("[Channel1]", "loop_out"));
    m_controlDescriptions.append("Loop Out for Player 1");

    m_controlsToBind.append(MixxxControl("[Channel2]", "loop_out"));
    m_controlDescriptions.append("Loop Out for Player 2");

    m_controlsToBind.append(MixxxControl("[Channel1]", "reloop_exit"));
    m_controlDescriptions.append("Reloop / Exit for Player 1");

    m_controlsToBind.append(MixxxControl("[Channel2]", "reloop_exit"));
    m_controlDescriptions.append("Reloop / Exit for Player 2");

    //Should be same number of controls as descriptions.
    Q_ASSERT(m_controlsToBind.size() == m_controlDescriptions.size());

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
    m_pMidiMapping->beginMidiLearn(m_controlsToBind[iCurrentControl]);
    labelMixxxControl->setText(m_controlDescriptions[iCurrentControl]);
}

void DlgMidiLearning::next()
{
    iCurrentControl++;
    if (iCurrentControl < m_controlsToBind.size())
    {
        m_pMidiMapping->beginMidiLearn(m_controlsToBind[iCurrentControl]);
        labelMixxxControl->setText(m_controlDescriptions[iCurrentControl]);
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
        m_pMidiMapping->beginMidiLearn(m_controlsToBind[iCurrentControl]);
        labelMixxxControl->setText(m_controlDescriptions[iCurrentControl]);
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
