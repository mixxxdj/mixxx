#include "widget/wtracktableview.h"

#include <QDrag>
#include <QModelIndex>
#include <QScrollBar>
#include <QShortcut>
#include <QUrl>

#include "control/controlobject.h"
#include "library/dao/trackschema.h"
#include "library/library.h"
#include "library/librarytablemodel.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "mixer/playermanager.h"
#include "preferences/colorpalettesettings.h"
#include "preferences/dialog/dlgpreflibrary.h"
#include "sources/soundsourceproxy.h"
#include "track/track.h"
#include "track/trackref.h"
#include "util/assert.h"
#include "util/dnd.h"
#include "util/time.h"
#include "widget/wtrackmenu.h"
#include "widget/wtracktableviewheader.h"

namespace {

const ConfigKey kConfigKeyAllowTrackLoadToPlayingDeck("[Controls]", "AllowTrackLoadToPlayingDeck");

}

WTrackTableView::WTrackTableView(QWidget* parent,
        UserSettingsPointer pConfig,
        Library* pLibrary,
        double backgroundColorOpacity,
        bool sorting)
        : WLibraryTableView(parent,
                  pConfig,
                  ConfigKey(LIBRARY_CONFIGVALUE,
                          WTRACKTABLEVIEW_VSCROLLBARPOS_KEY)),
          m_pConfig(pConfig),
          m_pLibrary(pLibrary),
          m_backgroundColorOpacity(backgroundColorOpacity),
          m_sorting(sorting),
          m_selectionChangedSinceLastGuiTick(true),
          m_loadCachedOnly(false) {
    // Connect slots and signals to make the world go 'round.
    connect(this, &WTrackTableView::doubleClicked, this, &WTrackTableView::slotMouseDoubleClicked);

    m_pCOTGuiTick = new ControlProxy("[Master]", "guiTick50ms", this);
    m_pCOTGuiTick->connectValueChanged(this, &WTrackTableView::slotGuiTick50ms);

    m_pKeyNotation = new ControlProxy("[Library]", "key_notation", this);
    m_pKeyNotation->connectValueChanged(this, &WTrackTableView::keyNotationChanged);

    m_pSortColumn = new ControlProxy("[Library]", "sort_column", this);
    m_pSortColumn->connectValueChanged(this, &WTrackTableView::applySortingIfVisible);
    m_pSortOrder = new ControlProxy("[Library]", "sort_order", this);
    m_pSortOrder->connectValueChanged(this, &WTrackTableView::applySortingIfVisible);

    connect(this,
            &WTrackTableView::scrollValueChanged,
            this,
            &WTrackTableView::slotScrollValueChanged);

    QShortcut* setFocusShortcut =
            new QShortcut(QKeySequence(tr("ESC", "Focus")), this);
    connect(setFocusShortcut,
            &QShortcut::activated,
            this,
            QOverload<>::of(&WTrackTableView::setFocus));
}

WTrackTableView::~WTrackTableView() {
    WTrackTableViewHeader* pHeader =
            dynamic_cast<WTrackTableViewHeader*>(horizontalHeader());
    if (pHeader) {
        pHeader->saveHeaderState();
    }
}

void WTrackTableView::enableCachedOnly() {
    if (!m_loadCachedOnly) {
        // don't try to load and search covers, drawing only
        // covers which are already in the QPixmapCache.
        emit onlyCachedCoverArt(true);
        m_loadCachedOnly = true;
    }
    m_lastUserAction = mixxx::Time::elapsed();
}

void WTrackTableView::slotScrollValueChanged(int /*unused*/) {
    enableCachedOnly();
}

void WTrackTableView::selectionChanged(
        const QItemSelection& selected, const QItemSelection& deselected) {
    m_selectionChangedSinceLastGuiTick = true;
    enableCachedOnly();
    QTableView::selectionChanged(selected, deselected);
}

void WTrackTableView::slotGuiTick50ms(double /*unused*/) {
    // if the user is stopped in the same row for more than 0.1 s,
    // we load un-cached cover arts as well.
    mixxx::Duration timeDelta = mixxx::Time::elapsed() - m_lastUserAction;
    if (m_loadCachedOnly && timeDelta > mixxx::Duration::fromMillis(100)) {
        // Show the currently selected track in the large cover art view and
        // highlights crate and playlists. Doing this in selectionChanged
        // slows down scrolling performance so we wait until the user has
        // stopped interacting first.
        if (m_selectionChangedSinceLastGuiTick) {
            const QModelIndexList indices = selectionModel()->selectedRows();
            if (indices.size() == 1 && indices.first().isValid()) {
                // A single track has been selected
                TrackModel* trackModel = getTrackModel();
                if (trackModel) {
                    TrackPointer pTrack = trackModel->getTrack(indices.first());
                    if (pTrack) {
                        emit trackSelected(pTrack);
                    }
                }
            } else {
                // None or multiple tracks have been selected
                emit trackSelected(TrackPointer());
            }
            m_selectionChangedSinceLastGuiTick = false;
        }

        // This allows CoverArtDelegate to request that we load covers from disk
        // (as opposed to only serving them from cache).
        emit onlyCachedCoverArt(false);
        m_loadCachedOnly = false;
    }
}

// slot
void WTrackTableView::loadTrackModel(QAbstractItemModel* model) {
    qDebug() << "WTrackTableView::loadTrackModel()" << model;

    TrackModel* trackModel = dynamic_cast<TrackModel*>(model);

    VERIFY_OR_DEBUG_ASSERT(model) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(trackModel) {
        return;
    }

    TrackModel* newModel = 0;

    /* If the model has not changed
     * there's no need to exchange the headers
     * this will cause a small GUI freeze
     */
    if (getTrackModel() == trackModel) {
        // Re-sort the table even if the track model is the same. This triggers
        // a select() if the table is dirty.
        doSortByColumn(horizontalHeader()->sortIndicatorSection(),
                horizontalHeader()->sortIndicatorOrder());
        return;
    } else {
        newModel = trackModel;
        saveVScrollBarPos(getTrackModel());
        //saving current vertical bar position
        //using address of track model as key
    }

    setVisible(false);

    // Save the previous track model's header state
    WTrackTableViewHeader* oldHeader =
            dynamic_cast<WTrackTableViewHeader*>(horizontalHeader());
    if (oldHeader) {
        oldHeader->saveHeaderState();
    }

    // rryan 12/2009 : Due to a bug in Qt, in order to switch to a model with
    // different columns than the old model, we have to create a new horizontal
    // header. Also, for some reason the WTrackTableView has to be hidden or
    // else problems occur. Since we parent the WtrackTableViewHeader's to the
    // WTrackTableView, they are automatically deleted.
    auto header = new WTrackTableViewHeader(Qt::Horizontal, this);

    // WTF(rryan) The following saves on unnecessary work on the part of
    // WTrackTableHeaderView. setHorizontalHeader() calls setModel() on the
    // current horizontal header. If this happens on the old
    // WTrackTableViewHeader, then it will save its old state, AND do the work
    // of initializing its menus on the new model. We create a new
    // WTrackTableViewHeader, so this is wasteful. Setting a temporary
    // QHeaderView here saves on setModel() calls. Since we parent the
    // QHeaderView to the WTrackTableView, it is automatically deleted.
    auto tempHeader = new QHeaderView(Qt::Horizontal, this);
    /* Tobias Rafreider: DO NOT SET SORTING TO TRUE during header replacement
     * Otherwise, setSortingEnabled(1) will immediately trigger sortByColumn()
     * For some reason this will cause 4 select statements in series
     * from which 3 are redundant --> expensive at all
     *
     * Sorting columns, however, is possible because we
     * enable clickable sorting indicators some lines below.
     * Furthermore, we connect signal 'sortIndicatorChanged'.
     *
     * Fixes Bug #672762
     */

    setSortingEnabled(false);
    setHorizontalHeader(tempHeader);

    setModel(model);
    setHorizontalHeader(header);
    header->setSectionsMovable(true);
    header->setSectionsClickable(true);
    header->setHighlightSections(true);
    header->setSortIndicatorShown(m_sorting);
    header->setDefaultAlignment(Qt::AlignLeft);

    // Initialize all column-specific things
    for (int i = 0; i < model->columnCount(); ++i) {
        // Setup delegates according to what the model tells us
        QAbstractItemDelegate* delegate =
                trackModel->delegateForColumn(i, this);
        // We need to delete the old delegates, since the docs say the view will
        // not take ownership of them.
        QAbstractItemDelegate* old_delegate = itemDelegateForColumn(i);
        // If delegate is NULL, it will unset the delegate for the column
        setItemDelegateForColumn(i, delegate);
        delete old_delegate;

        // Show or hide the column based on whether it should be shown or not.
        if (trackModel->isColumnInternal(i)) {
            //qDebug() << "Hiding column" << i;
            horizontalHeader()->hideSection(i);
        }
        /* If Mixxx starts the first time or the header states have been cleared
         * due to database schema evolution we gonna hide all columns that may
         * contain a potential large number of NULL values.  This will hide the
         * key column by default unless the user brings it to front
         */
        if (trackModel->isColumnHiddenByDefault(i) &&
                !header->hasPersistedHeaderState()) {
            //qDebug() << "Hiding column" << i;
            horizontalHeader()->hideSection(i);
        }
    }

    if (m_sorting) {
        // NOTE: Should be a UniqueConnection but that requires Qt 4.6
        // But Qt::UniqueConnections do not work for lambdas, non-member functions
        // and functors; they only apply to connecting to member functions.
        // https://doc.qt.io/qt-5/qobject.html#connect
        connect(horizontalHeader(),
                &QHeaderView::sortIndicatorChanged,
                this,
                &WTrackTableView::slotSortingChanged,
                Qt::AutoConnection);

        int sortColumn;
        Qt::SortOrder sortOrder;

        // Stupid hack that assumes column 0 is never visible, but this is a weak
        // proxy for "there was a saved column sort order"
        if (horizontalHeader()->sortIndicatorSection() > 0) {
            // Sort by the saved sort section and order.
            sortColumn = horizontalHeader()->sortIndicatorSection();
            sortOrder = horizontalHeader()->sortIndicatorOrder();
        } else {
            // No saved order is present. Use the TrackModel's default sort order.
            sortColumn = trackModel->defaultSortColumn();
            sortOrder = trackModel->defaultSortOrder();

            // If the TrackModel has an invalid or internal column as its default
            // sort, find the first non-internal column and sort by that.
            while (sortColumn < 0 || trackModel->isColumnInternal(sortColumn)) {
                sortColumn++;
            }
        }

        m_pSortColumn->set(static_cast<int>(trackModel->sortColumnIdFromColumnIndex(sortColumn)));
        m_pSortOrder->set(sortOrder);
        applySorting();
    }

    // Set up drag and drop behavior according to whether or not the track
    // model says it supports it.

    // Defaults
    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::DragOnly);
    // Always enable drag for now (until we have a model that doesn't support
    // this.)
    setDragEnabled(true);

    if (trackModel->hasCapabilities(TrackModel::Capability::ReceiveDrops)) {
        setDragDropMode(QAbstractItemView::DragDrop);
        setDropIndicatorShown(true);
        setAcceptDrops(true);
        //viewport()->setAcceptDrops(true);
    }

    // Possible giant fuckup alert - It looks like Qt has something like these
    // caps built-in, see http://doc.trolltech.com/4.5/qt.html#ItemFlag-enum and
    // the flags(...) function that we're already using in LibraryTableModel. I
    // haven't been able to get it to stop us from using a model as a drag
    // target though, so my hax above may not be completely unjustified.

    setVisible(true);

    restoreVScrollBarPos(newModel);
    // restoring scrollBar position using model pointer as key
    // scrollbar positions with respect to different models are backed by map
    initTrackMenu();
}

void WTrackTableView::initTrackMenu() {
    auto trackModel = getTrackModel();
    DEBUG_ASSERT(trackModel);

    if (m_pTrackMenu) {
        m_pTrackMenu->deleteLater();
    }

    m_pTrackMenu = make_parented<WTrackMenu>(this,
            m_pConfig,
            m_pLibrary,
            WTrackMenu::Feature::All,
            trackModel);
    connect(m_pTrackMenu.get(),
            &WTrackMenu::loadTrackToPlayer,
            this,
            &WTrackTableView::loadTrackToPlayer);
}

// slot
void WTrackTableView::slotMouseDoubleClicked(const QModelIndex& index) {
    // Read the current TrackLoadAction settings
    int doubleClickActionConfigValue =
            m_pConfig->getValue(ConfigKey("[Library]", "TrackLoadAction"),
                    static_cast<int>(DlgPrefLibrary::TrackDoubleClickAction::LoadToDeck));
    DlgPrefLibrary::TrackDoubleClickAction doubleClickAction =
            static_cast<DlgPrefLibrary::TrackDoubleClickAction>(
                    doubleClickActionConfigValue);

    if (doubleClickAction == DlgPrefLibrary::TrackDoubleClickAction::Ignore) {
        return;
    }

    auto trackModel = getTrackModel();
    VERIFY_OR_DEBUG_ASSERT(trackModel) {
        return;
    }

    if (doubleClickAction == DlgPrefLibrary::TrackDoubleClickAction::LoadToDeck &&
            trackModel->hasCapabilities(
                    TrackModel::Capability::LoadToDeck)) {
        TrackPointer pTrack = trackModel->getTrack(index);
        if (pTrack) {
            emit loadTrack(pTrack);
        }
    } else if (doubleClickAction == DlgPrefLibrary::TrackDoubleClickAction::AddToAutoDJBottom &&
            trackModel->hasCapabilities(
                    TrackModel::Capability::AddToAutoDJ)) {
        addToAutoDJ(PlaylistDAO::AutoDJSendLoc::BOTTOM);
    } else if (doubleClickAction == DlgPrefLibrary::TrackDoubleClickAction::AddToAutoDJTop &&
            trackModel->hasCapabilities(
                    TrackModel::Capability::AddToAutoDJ)) {
        addToAutoDJ(PlaylistDAO::AutoDJSendLoc::TOP);
    }
}

TrackModel::SortColumnId WTrackTableView::getColumnIdFromCurrentIndex() {
    TrackModel* trackModel = getTrackModel();
    VERIFY_OR_DEBUG_ASSERT(trackModel) {
        return TrackModel::SortColumnId::Invalid;
    }
    return trackModel->sortColumnIdFromColumnIndex(currentIndex().column());
}

void WTrackTableView::assignPreviousTrackColor() {
    QModelIndexList indices = selectionModel()->selectedRows();
    if (indices.size() <= 0) {
        return;
    }

    TrackModel* trackModel = getTrackModel();
    if (!trackModel) {
        return;
    }

    QModelIndex index = indices.at(0);
    TrackPointer pTrack = trackModel->getTrack(index);
    if (pTrack) {
        ColorPaletteSettings colorPaletteSettings(m_pConfig);
        ColorPalette colorPalette = colorPaletteSettings.getTrackColorPalette();
        mixxx::RgbColor::optional_t color = pTrack->getColor();
        pTrack->setColor(colorPalette.previousColor(color));
    }
}

void WTrackTableView::assignNextTrackColor() {
    QModelIndexList indices = selectionModel()->selectedRows();
    if (indices.size() <= 0) {
        return;
    }

    TrackModel* trackModel = getTrackModel();
    if (!trackModel) {
        return;
    }

    QModelIndex index = indices.at(0);
    TrackPointer pTrack = trackModel->getTrack(index);
    if (pTrack) {
        ColorPaletteSettings colorPaletteSettings(m_pConfig);
        ColorPalette colorPalette = colorPaletteSettings.getTrackColorPalette();
        mixxx::RgbColor::optional_t color = pTrack->getColor();
        pTrack->setColor(colorPalette.nextColor(color));
    }
}

void WTrackTableView::slotPurge() {
    QModelIndexList indices = selectionModel()->selectedRows();
    if (indices.size() > 0) {
        TrackModel* trackModel = getTrackModel();
        if (trackModel) {
            trackModel->purgeTracks(indices);
        }
    }
}

void WTrackTableView::slotUnhide() {
    QModelIndexList indices = selectionModel()->selectedRows();

    if (indices.size() > 0) {
        TrackModel* trackModel = getTrackModel();
        if (trackModel) {
            trackModel->unhideTracks(indices);
        }
    }
}

void WTrackTableView::contextMenuEvent(QContextMenuEvent* event) {
    VERIFY_OR_DEBUG_ASSERT(m_pTrackMenu.get()) {
        initTrackMenu();
    }
    event->accept();
    // Update track indices in context menu
    QModelIndexList indices = selectionModel()->selectedRows();
    m_pTrackMenu->loadTrackModelIndices(indices);

    //Create the right-click menu
    m_pTrackMenu->popup(event->globalPos());
}

void WTrackTableView::onSearch(const QString& text) {
    TrackModel* trackModel = getTrackModel();
    if (trackModel) {
        bool searchWasEmpty = false;
        if (trackModel->currentSearch().isEmpty()) {
            saveNoSearchVScrollBarPos();
            searchWasEmpty = true;
        }
        trackModel->search(text);
        if (!searchWasEmpty && text.isEmpty()) {
            restoreNoSearchVScrollBarPos();
        }
    }
}

void WTrackTableView::onShow() {
}

void WTrackTableView::mouseMoveEvent(QMouseEvent* pEvent) {
    // Only use this for drag and drop if the LeftButton is pressed we need to
    // check for this because mousetracking is activated and this function is
    // called everytime the mouse is moved -- kain88 May 2012
    if (pEvent->buttons() != Qt::LeftButton) {
        // Needed for mouse-tracking to fire entered() events. If we call this
        // outside of this if statement then we get 'ghost' drags. See Bug
        // #1008737
        WLibraryTableView::mouseMoveEvent(pEvent);
        return;
    }

    TrackModel* trackModel = getTrackModel();
    if (!trackModel) {
        return;
    }
    //qDebug() << "MouseMoveEvent";
    // Iterate over selected rows and append each item's location url to a list.
    QList<QString> locations;
    const QModelIndexList indices = selectionModel()->selectedRows();

    for (const QModelIndex& index : indices) {
        if (!index.isValid()) {
            continue;
        }
        locations.append(trackModel->getTrackLocation(index));
    }
    DragAndDropHelper::dragTrackLocations(locations, this, "library");
}

// Drag enter event, happens when a dragged item hovers over the track table view
void WTrackTableView::dragEnterEvent(QDragEnterEvent * event) {
    auto trackModel = getTrackModel();
    //qDebug() << "dragEnterEvent" << event->mimeData()->formats();
    if (event->mimeData()->hasUrls()) {
        if (event->source() == this) {
            if (trackModel->hasCapabilities(TrackModel::Capability::Reorder)) {
                event->acceptProposedAction();
                return;
            }
        } else if (DragAndDropHelper::dragEnterAccept(*event->mimeData(),
                                                      "library", true, true)) {
            event->acceptProposedAction();
            return;
        }
    }
    event->ignore();
}

// Drag move event, happens when a dragged item hovers over the track table view...
// It changes the drop handle to a "+" when the drag content is acceptable.
// Without it, the following drop is ignored.
void WTrackTableView::dragMoveEvent(QDragMoveEvent * event) {
    auto trackModel = getTrackModel();
    // Needed to allow auto-scrolling
    WLibraryTableView::dragMoveEvent(event);

    //qDebug() << "dragMoveEvent" << event->mimeData()->formats();
    if (event->mimeData()->hasUrls())
    {
        if (event->source() == this) {
            if (trackModel->hasCapabilities(TrackModel::Capability::Reorder)) {
                event->acceptProposedAction();
            } else {
                event->ignore();
            }
        } else {
            event->acceptProposedAction();
        }
    } else {
        event->ignore();
    }
}

// Drag-and-drop "drop" event. Occurs when something is dropped onto the track table view
void WTrackTableView::dropEvent(QDropEvent * event) {
    TrackModel* trackModel = getTrackModel();

    // We only do things to the TrackModel in this method so if we don't have
    // one we should just bail.
    if (!trackModel) {
        return;
    }

    if (!event->mimeData()->hasUrls() || trackModel->isLocked()) {
        event->ignore();
        return;
    }

    // Save the vertical scrollbar position. Adding new tracks and moving tracks in
    // the SQL data models causes a select() (ie. generation of a new result set),
    // which causes view to reset itself. A view reset causes the widget to scroll back
    // up to the top, which is confusing when you're dragging and dropping. :)
    int vScrollBarPos = verticalScrollBar()->value();


    // Calculate the model index where the track or tracks are destined to go.
    // (the "drop" position in a drag-and-drop)
    // The user usually drops on the seam between two rows.
    // We take the row below the seam for reference.
    int dropRow = rowAt(event->pos().y());
    int height = rowHeight(dropRow);
    QPoint pointOfRowBelowSeam(event->pos().x(), event->pos().y() + height / 2);
    QModelIndex destIndex = indexAt(pointOfRowBelowSeam);

    //qDebug() << "destIndex.row() is" << destIndex.row();

    // Drag and drop within this widget (track reordering)
    if (event->source() == this &&
            trackModel->hasCapabilities(TrackModel::Capability::Reorder)) {
        // Note the above code hides an ambiguous case when a
        // playlist is empty. For that reason, we can't factor that
        // code out to be common for both internal reordering
        // and external drag-and-drop. With internal reordering,
        // you can't have an empty playlist. :)

        //qDebug() << "track reordering" << __FILE__ << __LINE__;

        // Save a list of row (just plain ints) so we don't get screwed over
        // when the QModelIndexes all become invalid (eg. after moveTrack()
        // or addTrack())
        const QModelIndexList indices = selectionModel()->selectedRows();

        QList<int> selectedRows;
        for (const QModelIndex& idx : indices) {
            selectedRows.append(idx.row());
        }

        // Note: The biggest subtlety in the way I've done this track reordering code
        // is that as soon as we've moved ANY track, all of our QModelIndexes probably
        // get screwed up. The starting point for the logic below is to say screw it to
        // the QModelIndexes, and just keep a list of row numbers to work from. That
        // ends up making the logic simpler and the behavior totally predictable,
        // which lets us do nice things like "restore" the selection model.

        // The model indices are sorted so that we remove the tracks from the table
        // in ascending order. This is necessary because if track A is above track B in
        // the table, and you remove track A, the model index for track B will change.
        // Sorting the indices first means we don't have to worry about this.
        //std::sort(m_selectedIndices.begin(), m_selectedIndices.end(), std::greater<QModelIndex>());
        std::sort(selectedRows.begin(), selectedRows.end());
        int maxRow = 0;
        int minRow = 0;
        if (!selectedRows.isEmpty()) {
            maxRow = selectedRows.last();
            minRow = selectedRows.first();
        }

        // Destination row, if destIndex is invalid we set it to last row + 1
        int destRow = destIndex.row() < 0 ? model()->rowCount() : destIndex.row();

        int selectedRowCount = selectedRows.count();
        int selectionRestoreStartRow = destRow;

        // Adjust first row of new selection
        if (destRow >= minRow && destRow <= maxRow) {
            // If you drag a contiguous selection of multiple tracks and drop
            // them somewhere inside that same selection, do nothing.
            return;
        } else {
            if (destRow < minRow) {
                // If we're moving the tracks _up_,
                // then reverse the order of the row selection
                // to make the algorithm below work as it is
                std::sort(selectedRows.begin(),
                      selectedRows.end(),
                      std::greater<int>());
            } else {
               if (destRow > maxRow) {
                   // If we're moving the tracks _down_,
                   // adjust the first row to reselect
                   selectionRestoreStartRow =
                        selectionRestoreStartRow - selectedRowCount;
                }
            }
        }

        // For each row that needs to be moved...
        while (!selectedRows.isEmpty()) {
            int movedRow = selectedRows.takeFirst(); // Remember it's row index
            // Move it
            trackModel->moveTrack(model()->index(movedRow, 0), destIndex);

            // Move the row indices for rows that got bumped up
            // into the void we left, or down because of the new spot
            // we're taking.
            for (int i = 0; i < selectedRows.count(); i++) {
                if ((selectedRows[i] > movedRow) && (
                    (destRow > selectedRows[i]) )) {
                    selectedRows[i] = selectedRows[i] - 1;
                } else if ((selectedRows[i] < movedRow) &&
                            (destRow < selectedRows[i])) {
                    selectedRows[i] = selectedRows[i] + 1;
                }
            }
        }


        // Highlight the moved rows again (restoring the selection)
        //QModelIndex newSelectedIndex = destIndex;
        for (int i = 0; i < selectedRowCount; i++) {
            this->selectionModel()->select(model()->index(selectionRestoreStartRow + i, 0),
                                            QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
    } else { // Drag and drop inside Mixxx is only for few rows, bulks happen here
        // Reset the selected tracks (if you had any tracks highlighted, it
        // clears them)
        this->selectionModel()->clear();

        // Add all the dropped URLs/tracks to the track model (playlist/crate)
        QList<TrackFile> trackFiles = DragAndDropHelper::supportedTracksFromUrls(
            event->mimeData()->urls(), false, true);

        QList<QString> fileLocationList;
        for (const TrackFile& trackFile : trackFiles) {
            fileLocationList.append(trackFile.location());
        }

        // Drag-and-drop from an external application
        // eg. dragging a track from Windows Explorer onto the track table.
        int numNewRows = fileLocationList.count();

        // Have to do this here because the index is invalid after
        // addTrack
        int selectionStartRow = destIndex.row();

        // Make a new selection starting from where the first track was
        // dropped, and select all the dropped tracks

        // If the track was dropped into an empty playlist, start at row
        // 0 not -1 :)
        if ((destIndex.row() == -1) && (model()->rowCount() == 0)) {
            selectionStartRow = 0;
        } else if ((destIndex.row() == -1) && (model()->rowCount() > 0)) {
            // If the track was dropped beyond the end of a playlist, then
            // we need to fudge the destination a bit...
            //qDebug() << "Beyond end of playlist";
            //qDebug() << "rowcount is:" << model()->rowCount();
            selectionStartRow = model()->rowCount();
        }

        // calling the addTracks returns number of failed additions
        int tracksAdded = trackModel->addTracks(destIndex, fileLocationList);

        // Decrement # of rows to select if some were skipped
        numNewRows -= (fileLocationList.size() - tracksAdded);

        // Create the selection, but only if the track model supports
        // reordering. (eg. crates don't support reordering/indexes)
        if (trackModel->hasCapabilities(TrackModel::Capability::Reorder)) {
            for (int i = selectionStartRow; i < selectionStartRow + numNewRows; i++) {
                this->selectionModel()->select(model()->index(i, 0),
                                               QItemSelectionModel::Select |
                                               QItemSelectionModel::Rows);
            }
        }
    }

    event->acceptProposedAction();
    updateGeometries();
    verticalScrollBar()->setValue(vScrollBarPos);
}

TrackModel* WTrackTableView::getTrackModel() const {
    TrackModel* trackModel = dynamic_cast<TrackModel*>(model());
    return trackModel;
}

void WTrackTableView::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Return) {
        // It is not a good idea if 'key_return'
        // causes a track to load since we allow in-line editing
        // of table items in general
        return;
    } else {
        QTableView::keyPressEvent(event);
    }
}

void WTrackTableView::loadSelectedTrack() {
    auto indices = selectionModel()->selectedRows();
    if (indices.size() > 0) {
        slotMouseDoubleClicked(indices.at(0));
    }
}

void WTrackTableView::loadSelectedTrackToGroup(QString group, bool play) {
    auto indices = selectionModel()->selectedRows();
    if (indices.size() > 0) {
        // If the track load override is disabled, check to see if a track is
        // playing before trying to load it
        if (!(m_pConfig->getValueString(
                               ConfigKey("[Controls]", "AllowTrackLoadToPlayingDeck"))
                            .toInt())) {
            // TODO(XXX): Check for other than just the first preview deck.
            if (group != "[PreviewDeck1]" &&
                    ControlObject::get(ConfigKey(group, "play")) > 0.0) {
                return;
            }
        }
        auto index = indices.at(0);
        auto trackModel = getTrackModel();
        TrackPointer pTrack;
        if (trackModel &&
                (pTrack = trackModel->getTrack(index))) {
            emit loadTrackToPlayer(pTrack, group, play);
        }
    }
}

QList<TrackId> WTrackTableView::getSelectedTrackIds() const {
    QList<TrackId> trackIds;

    QItemSelectionModel* pSelectionModel = selectionModel();
    VERIFY_OR_DEBUG_ASSERT(pSelectionModel != nullptr) {
        qWarning() << "No selected tracks available";
        return trackIds;
    }

    TrackModel* pTrackModel = getTrackModel();
    VERIFY_OR_DEBUG_ASSERT(pTrackModel != nullptr) {
        qWarning() << "No selected tracks available";
        return trackIds;
    }

    const QModelIndexList rows = selectionModel()->selectedRows();
    trackIds.reserve(rows.size());
    for (const QModelIndex& row: rows) {
        const TrackId trackId = pTrackModel->getTrackId(row);
        if (trackId.isValid()) {
            trackIds.append(trackId);
        } else {
            // This happens in the browse view where only some tracks
            // have an id.
            qDebug() << "Skipping row" << row << "with invalid track id";
        }
    }

    return trackIds;
}

void WTrackTableView::setSelectedTracks(const QList<TrackId>& trackIds) {
    QItemSelectionModel* pSelectionModel = selectionModel();
    VERIFY_OR_DEBUG_ASSERT(pSelectionModel != nullptr) {
        qWarning() << "No selected tracks available";
        return;
    }

    TrackModel* pTrackModel = getTrackModel();
    VERIFY_OR_DEBUG_ASSERT(pTrackModel != nullptr) {
        qWarning() << "No selected tracks available";
        return;
    }

    for (const auto& trackId : trackIds) {
        const auto gts = pTrackModel->getTrackRows(trackId);

        for (int trackRow : gts) {
            pSelectionModel->select(model()->index(trackRow, 0),
                    QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
    }
}

void WTrackTableView::addToAutoDJ(PlaylistDAO::AutoDJSendLoc loc) {
    auto trackModel = getTrackModel();
    if (!trackModel->hasCapabilities(TrackModel::Capability::AddToAutoDJ)) {
        return;
    }

    const QList<TrackId> trackIds = getSelectedTrackIds();
    if (trackIds.isEmpty()) {
        qWarning() << "No tracks selected for AutoDJ";
        return;
    }

    PlaylistDAO& playlistDao = m_pLibrary->trackCollections()
                                       ->internalCollection()
                                       ->getPlaylistDAO();

    // TODO(XXX): Care whether the append succeeded.
    m_pLibrary->trackCollections()->unhideTracks(trackIds);
    playlistDao.addTracksToAutoDJQueue(trackIds, loc);
}

void WTrackTableView::slotAddToAutoDJBottom() {
    addToAutoDJ(PlaylistDAO::AutoDJSendLoc::BOTTOM);
}

void WTrackTableView::slotAddToAutoDJTop() {
    addToAutoDJ(PlaylistDAO::AutoDJSendLoc::TOP);
}

void WTrackTableView::slotAddToAutoDJReplace() {
    addToAutoDJ(PlaylistDAO::AutoDJSendLoc::REPLACE);
}

void WTrackTableView::doSortByColumn(int headerSection, Qt::SortOrder sortOrder) {
    TrackModel* trackModel = getTrackModel();
    QAbstractItemModel* itemModel = model();

    if (trackModel == nullptr || itemModel == nullptr || !m_sorting) {
        return;
    }

    // Save the selection
    const QList<TrackId> selectedTrackIds = getSelectedTrackIds();
    int savedHScrollBarPos = horizontalScrollBar()->value();
    // Save the column of focused table cell.
    // The cell is not necessarily part of the selection, but even if it's
    // focused after deselecting a row we may assume the user clicked onto the
    // column that will be used for sorting.
    int prevColum = 0;
    if (currentIndex().isValid()) {
        prevColum = currentIndex().column();
    }

    sortByColumn(headerSection, sortOrder);

    QItemSelectionModel* currentSelection = selectionModel();
    currentSelection->reset(); // remove current selection

    // Find previously selected tracks and store respective rows for reselection.
    QMap<int, int> selectedRows;
    for (const auto& trackId : selectedTrackIds) {
        // TODO(rryan) slowly fixing the issues with BaseSqlTableModel. This
        // code is broken for playlists because it assumes each trackid is in
        // the table once. This will erroneously select all instances of the
        // track for playlists, but it works fine for every other view. The way
        // to fix this that we should do is to delegate the selection saving to
        // the TrackModel. This will allow the playlist table model to use the
        // table index as the unique id instead of this code stupidly using
        // trackid.
        const auto rows = trackModel->getTrackRows(trackId);
        for (int row : rows) {
            // Restore sort order by rows, so the following commands will act as expected
            selectedRows.insert(row, 0);
        }
    }

    // Select the first row of the previous selection.
    // This scrolls to that row and with the leftmost cell being focused we have
    // a starting point (currentIndex) for navigation with Up/Down keys.
    // Replaces broken scrollTo() (see comment below)
    if (!selectedRows.isEmpty()) {
        selectRow(selectedRows.firstKey());
    }

    // Refocus the cell in the column that was focused before sorting.
    // With this, any Up/Down key press moves the selection and keeps the
    // horizontal scrollbar position we will restore below.
    QModelIndex restoreIndex = itemModel->index(currentIndex().row(), prevColum);
    if (restoreIndex.isValid()) {
        setCurrentIndex(restoreIndex);
    }

    // Restore previous selection (doesn't affect focused cell).
    QMapIterator<int, int> i(selectedRows);
    while (i.hasNext()) {
        i.next();
        QModelIndex tl = itemModel->index(i.key(), 0);
        currentSelection->select(tl, QItemSelectionModel::Rows | QItemSelectionModel::Select);
    }

    // This seems to be broken since at least Qt 5.12: no scrolling is issued
    //scrollTo(first, QAbstractItemView::EnsureVisible);
    horizontalScrollBar()->setValue(savedHScrollBarPos);
}

void WTrackTableView::applySortingIfVisible() {
    // There are multiple instances of WTrackTableView, but we only want to
    // apply the sorting to the currently visible instance
    if (!isVisible()) {
        return;
    }

    applySorting();
}

void WTrackTableView::applySorting() {
    TrackModel* trackModel = getTrackModel();
    int sortColumnId = static_cast<int>(m_pSortColumn->get());
    if (sortColumnId == static_cast<int>(TrackModel::SortColumnId::Invalid)) {
        // During startup phase of Mixxx, this method is called with Invalid
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(
            sortColumnId >= static_cast<int>(TrackModel::SortColumnId::IdMin) &&
            sortColumnId < static_cast<int>(TrackModel::SortColumnId::IdMax)) {
        return;
    }

    int sortColumn = trackModel->columnIndexFromSortColumnId(static_cast<TrackModel::SortColumnId>(sortColumnId));
    if (sortColumn < 0) {
        return;
    }

    Qt::SortOrder sortOrder = (m_pSortOrder->get() == 0) ? Qt::AscendingOrder : Qt::DescendingOrder;

    // This line sorts the TrackModel
    horizontalHeader()->setSortIndicator(sortColumn, sortOrder);

    // in Qt5, we need to call it manually, which triggers finally the select()
    doSortByColumn(sortColumn, sortOrder);
}

void WTrackTableView::slotSortingChanged(int headerSection, Qt::SortOrder order) {

    double sortOrder = static_cast<double>(order);
    bool sortingChanged = false;

    TrackModel* trackModel = getTrackModel();
    TrackModel::SortColumnId sortColumnId = trackModel->sortColumnIdFromColumnIndex(headerSection);

    if (sortColumnId == TrackModel::SortColumnId::Invalid) {
        return;
    }

    if (static_cast<int>(sortColumnId) != static_cast<int>(m_pSortColumn->get())) {
        m_pSortColumn->set(static_cast<int>(sortColumnId));
        sortingChanged = true;
    }
    if (sortOrder != m_pSortOrder->get()) {
        m_pSortOrder->set(sortOrder);
        sortingChanged = true;
    }

    if (sortingChanged) {
        applySortingIfVisible();
    }
}

bool WTrackTableView::hasFocus() const {
    return QWidget::hasFocus();
}

void WTrackTableView::saveCurrentVScrollBarPos() {
    saveVScrollBarPos(getTrackModel());
}

void WTrackTableView::restoreCurrentVScrollBarPos() {
    restoreVScrollBarPos(getTrackModel());
}

void WTrackTableView::keyNotationChanged() {
    QWidget::update();
}
