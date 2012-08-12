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
#include "vamp/vamppluginloader.h"
#include <qlist.h>

class QWidget;

class DlgPrefKey : public QWidget, Ui::DlgPrefKEYDlg {
    Q_OBJECT
public:
    DlgPrefKey(QWidget *parent, ConfigObject<ConfigValue> *_config);
    ~DlgPrefKey();
public slots:

     /** Apply changes to widget */
    void slotApply();
    void slotUpdate();

private slots:
   // void pluginSelected(int i);
    void analyserEnabled(int i);
    //void writeTagsEnabled(int i);
    void fastAnalysisEnabled(int i);
    void firstLastEnabled(int i);
    void setDefaults();
    void reanalyzeEnabled(int i);
    void skipRelevantEnabled(int i);
   // void minBpmRangeChanged(int value);
   // void maxBpmRangeChanged(int value);
  //  void slotReanalyzeChanged(int value);

//    void on_bfirstLastEnabled_released();

  //  void on_bfirstLastEnabled_clicked();

signals:
    void apply(const QString &);
private:
   // void populate();
    void loadSettings();
    /** Pointer to config object */
    ConfigObject<ConfigValue>* m_pconfig;
    QList<QString> m_listName;
    QList<QString> m_listLibrary, m_listIdentifier;
    QString m_selectedAnalyser;
   // int m_minBpm;
    //int m_maxBpm;
    bool m_banalyserEnabled, m_bfastAnalysisEnabled, m_bfirstLastEnabled, m_breanalyzeEnabled, m_bskipRelevantEnabled;// m_bReanalyze;

      /** Pointer to config object */
 //   ConfigObject<ConfigValue> *config;
};

#endif
