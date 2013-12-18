#ifndef DLGPREFSELECTOR_H
#define DLGPREFSELECTOR_H

#include <QWidget>
#include <QHash>

#include "ui_dlgprefselectordlg.h"
#include "preferences/dlgpreferencepage.h"
#include "configobject.h"


class DlgPrefSelector : public DlgPreferencePage, public Ui::DlgPrefSelectorDlg {
    Q_OBJECT

  public:
    DlgPrefSelector(QWidget *parent, ConfigObject<ConfigValue> *pConfig);
    ~DlgPrefSelector();

  public slots:
    // Apply changes to preference widget
    void slotApply();
    void slotUpdate();
    void setDefaults();

  signals:
    void apply(const QString &);

  private slots:
    void filterGenreEnabled(int value);
    void filterBpmEnabled(int value);
    void filterBpmRange(int value);
    void filterKeyEnabled(int value);
    void filterKey4thEnabled(int value);
    void filterKey5thEnabled(int value);
    void filterKeyRelativeEnabled(int value);
    void setTimbreContribution(int value);
    void setRhythmContribution(int value);
    void setLastFmContribution(int value);
    // sets similarity contribution, adjusts sliders so that they sum to 100%
    void setContribution(QString key, int value);
    void displayTimbreDescription();
    void displayRhythmDescription();
    void displayLastFmDescription();

  private:
    void loadSettings();

    ConfigObject<ConfigValue>* m_pConfig;
    bool m_bFilterGenre;
    bool m_bFilterBpm;
    int m_iFilterBpmRange;
    bool m_bFilterKey;
    bool m_bFilterKey4th;
    bool m_bFilterKey5th;
    bool m_bFilterKeyRelative;
    QHash<QString, double> m_similarityContributions;
    QHash<QString, QSlider*> m_similaritySliders;
};

#endif // DLGPREFSELECTOR_H
