/*
 * dlgprefanalysers.h
 *
 *  Created on: 28/apr/2011
 *      Author: vittorio
 */

#ifndef DLGPREFANALYSERS_H_
#define DLGPREFANALYSERS_H_

#include "ui_dlgprefanalysersdlg.h"
#include "vamp/vamppluginloader.h"
#include "configobject.h"
#include <qstring.h>
#include <qlist.h>


class QWidget;

class DlgPrefAnalysers: public QWidget, public Ui::DlgAnalysersDlg  {
    Q_OBJECT
public:
    DlgPrefAnalysers(QWidget *parent, ConfigObject<ConfigValue> *_config);
    ~DlgPrefAnalysers();
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

signals:
    void apply(const QString &);
private:
    void populate();
    void loadSettings();
    /** Pointer to config object */
    ConfigObject<ConfigValue> *m_pconfig;
    QList<QString> m_listName;
    QList<QString> m_listLibrary, m_listIdentifier;
    QString m_selectedAnalyser;
    bool m_banalyserEnabled, m_bfixedtempoEnabled, m_boffsetEnabled, m_FastAnalysisEnabled;
};


#endif /* DLGPREFANALYSERS_H_ */
