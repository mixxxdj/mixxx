#pragma once

#include "library/libraryview.h"
#include "library/ui_dlgrelations.h"

class WLibrary;
class WRelationTableView;
class RelationsTableModel;
class QItemSelection;
class Library;
class KeyboardEventFilter;

class DlgRelations : public QWidget, public Ui::DlgRelations, public LibraryView {
    Q_OBJECT

  public:
    DlgRelations(WLibrary* parent,
            UserSettingsPointer pConfig,
            Library* pLibrary,
            KeyboardEventFilter* pKeyboard,
            bool relationPairView);
    ~DlgRelations() override;

    void onShow() override;
    bool hasFocus() const override;
    void setFocus() override;
    void onSearch(const QString& text) override;
    QString currentSearch();
    void saveCurrentViewState() override;
    bool restoreCurrentViewState() override;

  public slots:
    void installEventFilter(QObject* pFilter);
    void showAllRelations();
    void showRelatedTracks(TrackPointer pTrack);

  signals:
    void trackSelected(TrackPointer pTrack);

  private:
    WRelationTableView* m_pRelationTableView;
    RelationsTableModel* m_pRelationTableModel;

    bool m_bRelationPairView;
};
