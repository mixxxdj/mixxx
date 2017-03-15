#ifndef DLGHIDDEN_H
#define DLGHIDDEN_H

#include "library/ui_dlghidden.h"
#include "preferences/usersettings.h"
#include "library/library.h"
#include "library/libraryview.h"
#include "library/trackcollection.h"
#include "controllers/keyboard/keyboardeventfilter.h"

class WTrackTableView;
class HiddenTableModel;
class QItemSelection;

class DlgHidden : public QWidget, public Ui::DlgHidden, public LibraryView {
    Q_OBJECT
  public:
    DlgHidden(QWidget* parent, UserSettingsPointer pConfig,
              Library* pLibrary, TrackCollection* pTrackCollection,
              KeyboardEventFilter* pKeyboard);
    ~DlgHidden() override;

    void onShow() override;
    bool hasFocus() const override;
    void onSearch(const QString& text) override;

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
    WTrackTableView* m_pTrackTableView;
    HiddenTableModel* m_pHiddenTableModel;
};

#endif //DLGHIDDEN_H
