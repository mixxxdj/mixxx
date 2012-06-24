/*
 *  Created on: 28/apr/2011
 *      Author: vittorio
 */

#ifndef DLGPREFBEATS_H_
#define DLGPREFBEATS_H_

#include <QWidget>
#include <QString>
#include <QList>

#include "ui_dlgprefbeatsdlg.h"
#include "vamp/vamppluginloader.h"
#include "configobject.h"

class DlgPrefBeats : public QWidget, public Ui::DlgBeatsDlg {
    Q_OBJECT
  public:
    DlgPrefBeats(QWidget *parent, ConfigObject<ConfigValue> *_config);
    virtual ~DlgPrefBeats();

  public slots:
    /** Apply changes to widget */
    void slotApply();
    void slotUpdate();

  private slots:
    void pluginSelected(int i);
    void analyserEnabled(int i);
    void fixedtempoEnabled(int i);
    void offsetEnabled(int i);
    void setDefaults();
    void fastAnalysisEnabled(int i);
    void minBpmRangeChanged(int value);
    void maxBpmRangeChanged(int value);
    void slotReanalyzeChanged(int value);

  signals:
    void apply(const QString &);

  private:
    void populate();
    void loadSettings();
    /** Pointer to config object */
    ConfigObject<ConfigValue>* m_pconfig;
    QList<QString> m_listName;
    QList<QString> m_listLibrary, m_listIdentifier;
    QString m_selectedAnalyser;
    int m_minBpm;
    int m_maxBpm;
    bool m_banalyserEnabled, m_bfixedtempoEnabled, m_boffsetEnabled, m_FastAnalysisEnabled, m_bReanalyze;
};

#endif /* DLGPREFBEATS_H_ */
