/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGLADSPA_H
#define DLGLADSPA_H

#include <QtGui>

class MixxxApp;
class LADSPAView;

class DlgLADSPA : public QDialog
{
    Q_OBJECT

public:
    DlgLADSPA(QWidget* parent);
    ~DlgLADSPA();

private:
    LADSPAView *m_pView;
};

#endif
