#ifndef DLGMISSING_H
#define DLGMISSING_H

#include "ui_dlgmissing.h"
#include "configobject.h"
#include "library/libraryview.h"
#include "library/trackcollection.h"
#include "mixxxkeyboard.h"

class WTrackTableView;
class MissingTableModel;

class DlgMissing : public QWidget, public Ui::DlgMissing, public LibraryView {
    Q_OBJECT
  public:
    DlgMissing(QWidget *parent, ConfigObject<ConfigValue>* pConfig,
              TrackCollection* pTrackCollection, MixxxKeyboard* pKeyboard);
    virtual ~DlgMissing();

    void onShow();
    void onSearch(const QString& text);

  public slots:
    void clicked();
    void selectAll();
    void selectionChanged(const QItemSelection&, const QItemSelection&);

  private:
    void activateButtons(bool enable);
    TrackCollection* m_pTrackCollection;
    WTrackTableView* m_pTrackTableView;
    MissingTableModel* m_pMissingTableModel;
};

#endif //DLGMISSING_H
