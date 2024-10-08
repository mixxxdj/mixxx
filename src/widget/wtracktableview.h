#pragma once

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

#include "control/controlproxy.h"
#include "control/pollingcontrolproxy.h"
#include "library/dao/playlistdao.h"
#include "library/trackmodel.h" // Can't forward declare enums
#include "preferences/usersettings.h"
#include "util/duration.h"
#include "util/parented_ptr.h"
#include "widget/wlibrarytableview.h"

class ControlProxy;
class DlgTagFetcher;
class DlgTrackInfo;
class ExternalTrackCollection;
class Library;
class WTrackMenu;

class WTrackTableView : public WLibraryTableView {
    Q_OBJECT
  public:
    WTrackTableView(
            QWidget* parent,
            UserSettingsPointer pConfig,
            Library* pLibrary,
            double backgroundColorOpacity,
            bool sorting);
    ~WTrackTableView() override;
    void contextMenuEvent(QContextMenuEvent * event) override;
    void onSearch(const QString& text) override;
    void onShow() override;
    bool hasFocus() const override;
    void setFocus() override;
    void pasteFromSidebar() override;
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void activateSelectedTrack();
    void loadSelectedTrackToGroup(const QString& group, bool play);
    void selectTrackColor(int steps);
    TrackModel::SortColumnId getColumnIdFromCurrentIndex() override;
    QList<TrackId> getSelectedTrackIds() const;
    bool isTrackInCurrentView(const TrackId& trackId);
    void setSelectedTracks(const QList<TrackId>& tracks);
    TrackId getCurrentTrackId() const;
    bool setCurrentTrackId(const TrackId& trackId, int column = 0, bool scrollToTrack = false);

    void addToAutoDJBottom();
    void addToAutoDJTop();
    void addToAutoDJReplace();
    void selectTrack(const TrackId&);

    void removeSelectedTracks();
    void cutSelectedTracks();
    void copySelectedTracks();
    void pasteTracks(const QModelIndex& index);

    void moveSelection(int delta);
    void moveRows(QList<int> selectedRows, int destRow);
    void moveSelectedTracks(QKeyEvent* event);
    void selectTracksById(const QList<TrackId>& tracks, int prevColumn);
    void selectTracksByPosition(const QList<int>& positions, int prevColum);

    double getBackgroundColorOpacity() const {
        return m_backgroundColorOpacity;
    }

    Q_PROPERTY(QColor focusBorderColor
                    MEMBER m_focusBorderColor
                            NOTIFY focusBorderColorChanged
                                    DESIGNABLE true);
    QColor getFocusBorderColor() const {
        return m_focusBorderColor;
    }

    // Default color for played tracks' text color. #555555, bit darker than Qt::darkgray.
    // BaseTrackTableModel uses this for the ForegroundRole of played tracks.
    static constexpr const char* kDefaultTrackPlayedColor = "#555555";
    Q_PROPERTY(QColor trackPlayedColor
                    MEMBER m_trackPlayedColor
                            NOTIFY trackPlayedColorChanged
                                    DESIGNABLE true);
    QColor getTrackPlayedColor() const {
        return m_trackPlayedColor;
    }
    // Default color for missing tracks' text color. #ee0000, bit darker than Qt::red.
    // BaseTrackTableModel uses this for the ForegroundRole of missing tracks.
    static constexpr const char* kDefaultTrackMissingColor = "#ff0000";
    Q_PROPERTY(QColor trackMissingColor
                    MEMBER m_trackMissingColor
                            NOTIFY trackMissingColorChanged
                                    DESIGNABLE true);
    QColor getTrackMissingColor() const {
        return m_trackMissingColor;
    }

  signals:
    void trackMenuVisible(bool visible);
    void focusBorderColorChanged(QColor col);
    void trackPlayedColorChanged(QColor col);
    void trackMissingColorChanged(QColor col);

  public slots:
    void loadTrackModel(QAbstractItemModel* model, bool restoreState = false);
    void slotMouseDoubleClicked(const QModelIndex &);
    void slotUnhide();
    void slotPurge();
    void slotDeleteTracksFromDisk();
    void slotShowHideTrackMenu(bool show);

    void slotSaveCurrentViewState() {
        saveCurrentViewState();
    };
    bool slotRestoreCurrentViewState() {
        return restoreCurrentViewState();
    };
    void slotrestoreCurrentIndex() {
        restoreCurrentIndex();
    }

  private slots:
    void doSortByColumn(int headerSection, Qt::SortOrder sortOrder);
    void applySortingIfVisible();
    void applySorting();

    // Signalled 20 times per second (every 50ms) by GuiTick.
    void slotGuiTick50ms(double);
    void slotScrollValueChanged(int);

    void slotSortingChanged(int headerSection, Qt::SortOrder order);
    void keyNotationChanged();

  protected:
    QString getModelStateKey() const override;

  private:
    void addToAutoDJ(PlaylistDAO::AutoDJSendLoc loc);
    void dragMoveEvent(QDragMoveEvent * event) override;
    void dragEnterEvent(QDragEnterEvent * event) override;
    void dropEvent(QDropEvent * event) override;

    void enableCachedOnly();
    void selectionChanged(const QItemSelection &selected,
                          const QItemSelection &deselected) override;

    void mousePressEvent(QMouseEvent* pEvent) override;
    // Mouse move event, implemented to hide the text and show an icon instead
    // when dragging.
    void mouseMoveEvent(QMouseEvent *pEvent) override;

    // Returns the list of selected row indices, or an empty list if none are selected.
    QModelIndexList getSelectedRows() const;
    // Returns the list of selected row numbers, or an empty list if none are selected.
    QList<int> getSelectedRowNumbers() const;

    // Returns the current TrackModel, or returns NULL if none is set.
    TrackModel* getTrackModel() const;

    void initTrackMenu();

    void hideOrRemoveSelectedTracks();

    const UserSettingsPointer m_pConfig;
    Library* const m_pLibrary;

    // Context menu container
    parented_ptr<WTrackMenu> m_pTrackMenu;

    const double m_backgroundColorOpacity;
    QColor m_focusBorderColor;
    QColor m_trackPlayedColor;
    QColor m_trackMissingColor;
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
