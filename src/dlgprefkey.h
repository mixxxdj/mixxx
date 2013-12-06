#ifndef DLGPREFKEY_H
#define DLGPREFKEY_H

#include <QList>
#include <QWidget>
#include <QMap>

#include "ui_dlgprefkeydlg.h"
#include "configobject.h"
#include "track/keyutils.h"
#include "preferences/dlgpreferencepage.h"

class DlgPrefKey : public DlgPreferencePage, Ui::DlgPrefKeyDlg {
    Q_OBJECT
  public:
    DlgPrefKey(QWidget *parent, ConfigObject<ConfigValue> *_config);
    virtual ~DlgPrefKey();

  public slots:
    // Apply changes to widget
    void slotApply();
    void slotUpdate();

  private slots:
    void pluginSelected(int i);
    void analyserEnabled(int i);
    void fastAnalysisEnabled(int i);
    void setDefaults();
    void reanalyzeEnabled(int i);

    void setNotation(KeyUtils::KeyNotation notation);
    void setNotationOpenKey(bool);
    void setNotationLancelot(bool);
    void setNotationTraditional(bool);
    void setNotationCustom(bool);

  private:
    void populate();
    void loadSettings();

    ConfigObject<ConfigValue>* m_pConfig;
    QMap<mixxx::track::io::key::ChromaticKey, QLineEdit*> m_keyLineEdits;
    QList<QString> m_listName;
    QList<QString> m_listLibrary, m_listIdentifier;
    QString m_selectedAnalyser;
    bool m_bAnalyserEnabled;
    bool m_bFastAnalysisEnabled;
    bool m_bReanalyzeEnabled;
};

#endif
