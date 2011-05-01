/*
 * dlgprefanalysers.h
 *
 *  Created on: 28/apr/2011
 *      Author: vittorio
 */

#ifndef DLGPREFANALYSERS_H_
#define DLGPREFANALYSERS_H_

#include "ui_dlgprefanalysersdlg.h"
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
    void pluginSelected(int i);
    void setDefaults();
signals:
    void apply(const QString &);
private:
    void populate();
    void loadSettings();
    /** Pointer to config object */
    ConfigObject<ConfigValue> *m_pconfig;
    QList<QString> m_listName, m_listVersion, m_listMaker,
        m_listCopyright, m_listOutput, m_listDescription;
    QList<QString> m_listLibrary, m_listIdentifier;
    int m_iselectedAnalyser;
};


#endif /* DLGPREFANALYSERS_H_ */
