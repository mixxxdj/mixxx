#pragma once

#include <QByteArrayData>
#include <QItemSelection>
#include <QString>
#include <QWidget>

#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/library.h"
#include "library/libraryview.h"
#include "library/ui_dlgmissing.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"

class WLibrary;
class WTrackTableView;
class MissingTableModel;
class KeyboardEventFilter;
class Library;
class QItemSelection;
class QObject;

class DlgMissing : public QWidget, public Ui::DlgMissing, public LibraryView {
    Q_OBJECT

  public:
    DlgMissing(WLibrary* parent, UserSettingsPointer pConfig,
               Library* pLibrary,
               KeyboardEventFilter* pKeyboard);
    ~DlgMissing() override;

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
    MissingTableModel* m_pMissingTableModel;
};
