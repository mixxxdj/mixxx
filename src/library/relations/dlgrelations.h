#pragma once

#include "library/libraryview.h"
#include "library/ui_dlgrelations.h"

class WLibrary;
class WRelationTableView;
class RelationsTableModel;
class Relation;
class DlgRelationInfo;
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
    void selectionChanged(const QItemSelection&, const QItemSelection&);
    void slotShowDlgRelationInfo();
    void slotLoadRelationToDecks();
    void slotDeleteRelation();

  signals:
    void trackSelected(TrackPointer pTrack);
    void loadTrack(TrackPointer pTrack);
    void loadTrackToPlayer(TrackPointer pTrack, const QString& player);

  private:
    void activateButtons(bool enable);

    RelationsTableModel* m_pRelationTableModel;
    WRelationTableView* m_pRelationTableView;

    Library* m_pLibrary;

    std::unique_ptr<DlgRelationInfo> m_pDlgRelationInfo;

    bool m_bRelationPairView;
};
