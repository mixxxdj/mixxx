#pragma once

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

#include "control/controlproxy.h"
#include "library/dao/playlistdao.h"
#include "library/trackmodel.h" // Can't forward declare enums
#include "preferences/usersettings.h"
#include "util/duration.h"
#include "util/parented_ptr.h"
#include "widget/wlibrarytableview.h"

class ControlProxy;
class DlgTagFetcher;
class DlgTrackInfo;
class TrackCollectionManager;
class ExternalTrackCollection;
class WTrackMenu;

const QString WTRACKTABLEVIEW_VSCROLLBARPOS_KEY = "VScrollBarPos"; /** ConfigValue key for QTable vertical scrollbar position */
const QString LIBRARY_CONFIGVALUE = "[Library]"; /** ConfigValue "value" (wtf) for library stuff */

class WTrackTableView : public WLibraryTableView {
    Q_OBJECT
  public:
    WTrackTableView(
            QWidget* parent,
            UserSettingsPointer pConfig,
            TrackCollectionManager* pTrackCollectionManager,
            double backgroundColorOpacity,
            bool sorting);
    ~WTrackTableView() override;
    void contextMenuEvent(QContextMenuEvent * event) override;
    void onSearch(const QString& text) override;
    void onShow() override;
    bool hasFocus() const override;
    void keyPressEvent(QKeyEvent* event) override;
    void loadSelectedTrack() override;
    void loadSelectedTrackToGroup(QString group, bool play) override;
    void assignNextTrackColor() override;
    void assignPreviousTrackColor() override;
    QList<TrackId> getSelectedTrackIds() const;
    void setSelectedTracks(const QList<TrackId>& tracks);
    void saveCurrentVScrollBarPos();
    void restoreCurrentVScrollBarPos();

    double getBackgroundColorOpacity() const {
        return m_backgroundColorOpacity;
    }

  public slots:
    void loadTrackModel(QAbstractItemModel* model);
    void saveViewState();
    void slotMouseDoubleClicked(const QModelIndex &);
    void slotUnhide();
    void slotPurge();

    void slotAddToAutoDJBottom() override;
    void slotAddToAutoDJTop() override;
    void slotAddToAutoDJReplace() override;

  private slots:
    void doSortByColumn(int headerSection, Qt::SortOrder sortOrder);
    void applySortingIfVisible();
    void applySorting();

    // Signalled 20 times per second (every 50ms) by GuiTick.
    void slotGuiTick50ms(double);
    void slotScrollValueChanged(int);

    void slotSortingChanged(int headerSection, Qt::SortOrder order);
    void keyNotationChanged();

  private:
    void addToAutoDJ(PlaylistDAO::AutoDJSendLoc loc);
    void dragMoveEvent(QDragMoveEvent * event) override;
    void dragEnterEvent(QDragEnterEvent * event) override;
    void dropEvent(QDropEvent * event) override;

    void enableCachedOnly();
    void selectionChanged(const QItemSelection &selected,
                          const QItemSelection &deselected) override;

    // Mouse move event, implemented to hide the text and show an icon instead
    // when dragging.
    void mouseMoveEvent(QMouseEvent *pEvent) override;

    // Returns the current TrackModel, or returns NULL if none is set.
    TrackModel* getTrackModel() const;

    void initTrackMenu();

    const UserSettingsPointer m_pConfig;
    TrackCollectionManager* const m_pTrackCollectionManager;

    // Context menu container
    parented_ptr<WTrackMenu> m_pTrackMenu;

    const double m_backgroundColorOpacity;
    bool m_sorting;

    // Control the delay to load a cover art.
    mixxx::Duration m_lastUserAction;
    bool m_selectionChangedSinceLastGuiTick;
    bool m_loadCachedOnly;

    ControlProxy* m_pCOTGuiTick;
    ControlProxy* m_pKeyNotation;
    ControlProxy* m_pSortColumn;
    ControlProxy* m_pSortOrder;
};
