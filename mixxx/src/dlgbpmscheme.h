

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGBPMSCHEME_H
#define DLGBPMSCHEME_H

#include "ui_dlgbpmschemedlg.h"
#include "configobject.h"
#include <qstring.h>

class BpmScheme;

class QWidget;

class DlgBpmScheme : public QDialog, Ui::DlgBpmSchemeDlg  {
    Q_OBJECT
public:
    DlgBpmScheme(BpmScheme *& bpmScheme);
    ~DlgBpmScheme();
public slots:
     /** Apply changes to widget */
    void slotApply();
    void slotUpdate();
signals:
    void apply(const QString &);
private:
    BpmScheme *& m_BpmScheme;
};

#endif
