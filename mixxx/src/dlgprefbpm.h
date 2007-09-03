

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGPREFBPM_H
#define DLGPREFBPM_H

#include "ui_dlgprefbpmdlg.h"
#include "configobject.h"

class QWidget;

class DlgPrefBPM : public QWidget, Ui::DlgPrefBPMDlg  {
    Q_OBJECT
public:
    DlgPrefBPM(QWidget *parent, ConfigObject<ConfigValue> *_config);
    ~DlgPrefBPM();
public slots:

    void slotSetBPMDetectOnImport(int);
    void slotSetWriteID3Tag(int);
    void slotSetAnalyzeMode(int);
    void slotSetBPMRangeStart(int);
    void slotSetBPMRangeEnd(int);

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
