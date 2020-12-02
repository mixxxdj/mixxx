#pragma once

#include <QByteArrayData>
#include <QItemSelection>
#include <QString>
#include <QWidget>

#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/library.h"
#include "library/libraryview.h"
#include "library/ui_dlghidden.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"

class WLibrary;
class WTrackTableView;
class HiddenTableModel;
class KeyboardEventFilter;
class Library;
class QItemSelection;
class QObject;

class DlgHidden : public QWidget, public Ui::DlgHidden, public LibraryView {
    Q_OBJECT

  public:
    DlgHidden(WLibrary* parent, UserSettingsPointer pConfig,
              Library* pLibrary,
              KeyboardEventFilter* pKeyboard);
    ~DlgHidden() override;

    void onShow() override;
    bool hasFocus() const override;
    void onSearch(const QString& text) override;
    QString currentSearch();

  public slots:
    void clicked();
    void selectAll();
    void selectionChanged(const QItemSelection&, const QItemSelection&);

  signals:
    void trackSelected(TrackPointer pTrack);

  private:
    void activateButtons(bool enable);
    WTrackTableView* m_pTrackTableView;
    HiddenTableModel* m_pHiddenTableModel;
};
