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

class DlgMissing : public QWidget, public Ui::DlgMissing, public LibraryView {
    Q_OBJECT
  public:
    DlgMissing(QWidget* parent, UserSettingsPointer pConfig,
               Library* pLibrary, TrackCollection* pTrackCollection,
               KeyboardEventFilter* pKeyboard);
    virtual ~DlgMissing();

    void onShow();
    void onSearch(const QString& text);
    void setTrackTable(WTrackTableView* pTrackTableView, int paneId);
    inline void setFocusedPane(int focusedPane) { 
        m_focusedPane = focusedPane;
    }

  public slots:
    void clicked();
    void selectAll();
    void selectionChanged(const QItemSelection&, const QItemSelection&);
    void setTrackTableFont(const QFont& font);
    void setTrackTableRowHeight(int rowHeight);

  signals:
    void trackSelected(TrackPointer pTrack);

  private:
    void activateButtons(bool enable);
    MissingTableModel* m_pMissingTableModel;
    QHash<int, WTrackTableView*> m_trackTableView;
    int m_focusedPane;
};

#endif //DLGMISSING_H
