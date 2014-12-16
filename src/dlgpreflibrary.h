/***************************************************************************
                          dlgpreflibrary.h  -  description
                             -------------------
    begin                : Thu Apr 17 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGPREFLIBRARY_H
#define DLGPREFLIBRARY_H

#include <QStandardItemModel>
#include <QWidget>

#include "ui_dlgpreflibrarydlg.h"
#include "configobject.h"
#include "library/library.h"
#include "preferences/dlgpreferencepage.h"

/**
  *@author Tue & Ken Haste Andersen
  */

class DlgPrefLibrary : public DlgPreferencePage, public Ui::DlgPrefLibraryDlg  {
    Q_OBJECT
  public:
    enum TrackLoadAction {
        LOAD_TRACK_DECK,  // Load track to next available deck.
        ADD_TRACK_BOTTOM, // Add track to Auto-DJ Queue (bottom).
        ADD_TRACK_TOP     // Add track to Auto-DJ Queue (top).
    };

    DlgPrefLibrary(QWidget *parent, ConfigObject<ConfigValue> *config,
                   Library *pLibrary);
    virtual ~DlgPrefLibrary();

  public slots:
    // Common preference page slots.
    void slotUpdate();
    void slotShow();
    void slotHide();
    void slotResetToDefaults();
    void slotApply();

    // Dialog to browse for music file directory
    void slotAddDir();
    void slotRemoveDir();
    void slotRelocateDir();
    void slotExtraPlugins();

  signals:
    void apply();
    void scanLibrary();
    void requestAddDir(QString dir);
    void requestRemoveDir(QString dir, Library::RemovalType removalType);
    void requestRelocateDir(QString currentDir, QString newDir);

  private:
    void initialiseDirList();
    QStandardItemModel m_dirListModel;
    ConfigObject<ConfigValue>* m_pconfig;
    Library *m_pLibrary;
    bool m_baddedDirectory;
};

#endif
