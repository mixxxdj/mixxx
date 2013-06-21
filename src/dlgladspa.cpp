/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "dlgladspa.h"
#include "ladspaview.h"

DlgLADSPA::DlgLADSPA(QWidget* parent) : QDialog(parent)
{
    setWindowTitle("LADSPA");
    //resize(580, 280);

    m_pView = new LADSPAView(this);
}

DlgLADSPA::~DlgLADSPA()
{
    delete m_pView;
}
