/***************************************************************************
                          dlgbpmscheme.cpp  -  description
                             -------------------
    begin                : Thu Jun 7 2007
    copyright            : (C) 2007 by John Sully
    email                : jsully@scs.ryerson.ca
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtCore>

#include "dlgbpmscheme.h"
#include "bpm/bpmscheme.h"

DlgBpmScheme::DlgBpmScheme(BpmScheme *& bpmScheme) : QDialog(), Ui::DlgBpmSchemeDlg(), m_BpmScheme(bpmScheme)
{
    setupUi(this);
    
    connect(buttonBox, SIGNAL(accepted()), this,      SLOT(slotApply()));
    //connect(chkAnalyzeEntireSong,   SIGNAL(stateChanged(int)), this, SLOT(slotSetAnalyzeMode(int)));
    
    // Check to see if this is a new scheme. If so, create it with default values
    if(!bpmScheme)
    {
        bpmScheme = new BpmScheme("New Scheme Name", 50, 150, false);
    }
    
    // Populate the dialog values
    txtSchemeName->setText(bpmScheme->getName());
    spinBpmMin->setValue(bpmScheme->getMinBpm());
    spinBpmMax->setValue(bpmScheme->getMaxBpm());
    chkAnalyzeEntireSong->setChecked(bpmScheme->getAnalyzeEntireSong());    
}

DlgBpmScheme::~DlgBpmScheme()
{
}

void DlgBpmScheme::slotApply()
{
    m_BpmScheme->setName(txtSchemeName->text());
    m_BpmScheme->setMinBpm(spinBpmMin->value());
    m_BpmScheme->setMaxBpm(spinBpmMax->value());
    m_BpmScheme->setAnalyzeEntireSong(chkAnalyzeEntireSong->isChecked());
}

void DlgBpmScheme::slotUpdate()
{
}

