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
#include "midimessage.h"
#include "midimapping.h"

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

    pushButtonSkip->setShortcut(QKeySequence(Qt::Key_Space));    

    connect(pushButtonBegin, SIGNAL(clicked()), this, SLOT(begin()));
    connect(pushButtonSkip, SIGNAL(clicked()), this, SLOT(next()));
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

    //Should be same number of controls as descriptions.
    Q_ASSERT(m_controlsToBind.size() == m_controlDescriptions.size());

}

DlgMidiLearning::~DlgMidiLearning()
{
    //If there was any ongoing learning, cancel it (benign if there wasn't).
    m_pMidiMapping->cancelMidiLearn();
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
    }
    else
    {
        //We've hit the end, show the congrats pane
        stackedWidget->setCurrentIndex(2);
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
