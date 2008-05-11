

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

#include <qlist.h>

class QWidget;
class BpmScheme;

class DlgPrefBpm : public QWidget, Ui::DlgPrefBPMDlg  {
    Q_OBJECT
public:
    DlgPrefBpm(QWidget *parent, ConfigObject<ConfigValue> *_config);
    ~DlgPrefBpm();
public slots:

    void slotSetBpmDetectOnImport(int);
    void slotSetWriteID3Tag(int);
    void slotSetBpmEnabled(int);
    void slotSetBpmRangeStart(int);
    void slotSetBpmRangeEnd(int);
    void slotSetAboveRange(int);
   
    void slotEditBpmScheme();
    void slotAddBpmScheme();
    void slotDeleteBpmScheme();
    void slotDefaultBpmScheme();

     /** Apply changes to widget */
    void slotApply();
    void slotUpdate();
signals:
    void apply(const QString &);
private:

    void clearListIcons();

    // Determines whether or not to gray out the preferences
    void updateBpmEnabled();

    // Private methods for loading and saving the BPM schemes
    // to and from the file system.
    void loadBpmSchemes();
    void saveBpmSchemes();
    
    // Method for filling in the list of BPM schemes on the dialog
    void populateBpmSchemeList();

      /** Pointer to config object */
    ConfigObject<ConfigValue> *config;
    QList<BpmScheme*> m_BpmSchemes;
};

#endif
