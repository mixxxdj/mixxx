#ifndef DLGPREFKEY_H
#define DLGPREFKEY_H

#include <QList>
#include <QWidget>

#include "ui_dlgprefkeydlg.h"
#include "configobject.h"

class DlgPrefKey : public QWidget, Ui::DlgPrefKeyDlg {
    Q_OBJECT
  public:
    DlgPrefKey(QWidget *parent, ConfigObject<ConfigValue> *_config);
    virtual ~DlgPrefKey();

  public slots:
    // Apply changes to widget
    void slotApply();
    void slotUpdate();

  private slots:
    //void pluginSelected(int i);
    void analyserEnabled(int i);
    void fastAnalysisEnabled(int i);
    void setDefaults();
    void reanalyzeEnabled(int i);

  signals:
    void apply(const QString &);

  private:
    void loadSettings();

    ConfigObject<ConfigValue>* m_pconfig;
    QList<QString> m_listName;
    QList<QString> m_listLibrary, m_listIdentifier;
    QString m_selectedAnalyser;
    bool m_bAnalyserEnabled;
    bool m_bFastAnalysisEnabled;
    bool m_bReanalyzeEnabled;
};

#endif
