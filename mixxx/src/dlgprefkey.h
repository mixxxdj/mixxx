

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGPREFKEY_H
#define DLGPREFKEY_H

#include "ui_dlgprefkeydlg.h"
#include "configobject.h"

#include <qlist.h>

class QWidget;

class DlgPrefKeyNotationFormat : public QWidget, Ui::DlgPrefKeyNotationFormatDlg  {
    Q_OBJECT
public:
    DlgPrefKeyNotationFormat(QWidget *parent, ConfigObject<ConfigValue> *_config);
    ~DlgPrefKeyNotationFormat();
public slots:

     /** Apply changes to widget */
    void slotApply();
    void slotUpdate();
signals:
    void apply(const QString &);
private:

      /** Pointer to config object */
    ConfigObject<ConfigValue> *config;
};

#endif
