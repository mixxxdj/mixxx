#ifndef DLGPREFSELECTOR_H
#define DLGPREFSELECTOR_H

#include <QWidget>

#include "ui_dlgprefselectordlg.h"
#include "configobject.h"


class DlgPrefSelector : public QWidget, Ui::DlgPrefSelectorDlg {
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
    void setTimbreCoefficient(int value);
    void setRhythmCoefficient(int value);
    void setLastFmCoefficient(int value);
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
    int m_iTimbreCoefficient;
    int m_iRhythmCoefficient;
    int m_iLastFmCoefficient;
};

#endif // DLGPREFSELECTOR_H
