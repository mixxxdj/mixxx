#ifndef DLGMISSING_H
#define DLGMISSING_H

#include "library/ui_dlgmissing.h"
#include "preferences/usersettings.h"
#include "library/library.h"
#include "library/libraryview.h"
#include "library/trackcollection.h"
#include "controllers/keyboard/keyboardeventfilter.h"

class WTrackTableView;
class MissingTableModel;

class DlgMissing : public QFrame, public Ui::DlgMissing {
    Q_OBJECT
  public:
    DlgMissing(QWidget* parent);
    virtual ~DlgMissing();

    // The indexes are always from the Focused pane
    void setSelectedIndexes(const QModelIndexList& selectedIndexes);
    void setTableModel(MissingTableModel* pTableModel);

  public slots:
    void onShow();

  signals:
    void purge();
    void selectAll();
    void trackSelected(TrackPointer pTrack);

  private:
    void activateButtons(bool enable);
    
    MissingTableModel* m_pMissingTableModel;
};

#endif //DLGMISSING_H
