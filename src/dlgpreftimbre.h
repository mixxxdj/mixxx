#ifndef DLGPREFTIMBRE_H
#define DLGPREFTIMBRE_H

#include <QWidget>
#include <QList>

#include "configobject.h"
#include "preferences/dlgpreferencepage.h"
#include "ui_dlgpreftimbredlg.h"

class DlgPrefTimbre : public DlgPreferencePage, public Ui::DlgPrefTimbreDlg {
    Q_OBJECT
  public:
    DlgPrefTimbre(QWidget *parent, ConfigObject<ConfigValue> *pConfig);
    ~DlgPrefTimbre();

  public slots:
    void slotApply();
    void slotUpdate();

  private slots:
    void pluginSelected(int i);
    void analyserEnabled(int i);
    void setDefaults();

  signals:
    void apply(const QString &);

  private:
    void populate();
    void loadSettings();

    ConfigObject<ConfigValue>* m_pConfig;
    QList<QString> m_listName;
    QList<QString> m_listLibrary, m_listIdentifier;
    QString m_selectedAnalyser;
    bool m_bAnalyserEnabled;
};

#endif // DLGPREFTIMBRE_H
