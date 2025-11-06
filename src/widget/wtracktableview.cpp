#include "widget/wtracktableview.h"

#include <QDrag>
#include <QModelIndex>
#include <QScrollBar>
#include <QShortcut>
#include <QStylePainter>
#include <QUrl>

#include "control/controlobject.h"
#include "library/dao/trackschema.h"
#include "library/library.h"
#include "library/library_prefs.h"
#include "library/librarytablemodel.h"
#include "library/searchqueryparser.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "mixer/playermanager.h"
#include "moc_wtracktableview.cpp"
#include "preferences/colorpalettesettings.h"
#include "preferences/dialog/dlgprefdeck.h"
#include "preferences/dialog/dlgpreflibrary.h"
#include "sources/soundsourceproxy.h"
#include "track/track.h"
#include "track/trackref.h"
#include "util/assert.h"
#include "util/defs.h"
#include "util/dnd.h"
#include "util/time.h"
#include "widget/wtrackmenu.h"
#include "widget/wtracktableviewheader.h"

namespace {
const bool sDebug = false;
const QString windowName = QStringLiteral("[WTRACKTABLEVIEW]");
const QString windowNamePreparation = QStringLiteral("PreparationWindow");
const QString windowNameLibrary = QStringLiteral("Library");

// ConfigValue key for QTable vertical scrollbar position
const ConfigKey kVScrollBarPosConfigKey{
        // mixxx::library::prefs::kConfigGroup is defined in another
        // unit of compilation and cannot be reused here!
        QStringLiteral("[Library]"),
        QStringLiteral("VScrollBarPos")};

} // anonymous namespace

WTrackTableView::WTrackTableView(QWidget* pParent,
        UserSettingsPointer pConfig,
        Library* pLibrary,
        double backgroundColorOpacity)
        : WLibraryTableView(pParent, pConfig),
          m_pConfig(pConfig),
          m_pLibrary(pLibrary),
          m_backgroundColorOpacity(backgroundColorOpacity),
          // Default color for the focus border of TableItemDelegates
          m_focusBorderColor(kDefaultFocusBorderColor),
          m_trackPlayedColor(kDefaultTrackPlayedColor),
          m_trackMissingColor(kDefaultTrackMissingColor),
          m_dropIndicatorColor(kDefaultDropIndicatorColor),
          m_sorting(false),
          m_selectionChangedSinceLastGuiTick(true),
          m_loadCachedOnly(false),
          m_dropRow(-1) {
    // Connect slots and signals to make the world go 'round.
    connect(this, &WTrackTableView::doubleClicked, this, &WTrackTableView::slotMouseDoubleClicked);

    m_pCOTGuiTick = new ControlProxy(
            QStringLiteral("[App]"), QStringLiteral("gui_tick_50ms_period_s"), this);
    m_pCOTGuiTick->connectValueChanged(this, &WTrackTableView::slotGuiTick50ms);

    m_pKeyNotation = new ControlProxy(mixxx::library::prefs::kKeyNotationConfigKey, this);
    m_pKeyNotation->connectValueChanged(this, &WTrackTableView::keyNotationChanged);

    m_pSortColumn = new ControlProxy("[Library]", "sort_column", this);
    m_pSortColumn->connectValueChanged(this, &WTrackTableView::applySortingIfVisible);
    m_pSortOrder = new ControlProxy("[Library]", "sort_order", this);
    m_pSortOrder->connectValueChanged(this, &WTrackTableView::applySortingIfVisible);

    connect(this,
            &WTrackTableView::scrollValueChanged,
            this,
            &WTrackTableView::slotScrollValueChanged);
}

WTrackTableView::~WTrackTableView() {
    WTrackTableViewHeader* pHeader =
            qobject_cast<WTrackTableViewHeader*>(horizontalHeader());
    if (pHeader) {
        pHeader->saveHeaderState();
    }
}
#ifdef __LINUX__
void WTrackTableView::currentChanged(
        const QModelIndex& current,
        const QModelIndex& previous) {
    // This override fixes https://github.com/mixxxdj/mixxx/pull/14734
    // On Ubuntu-based distros it may happen that the InputMethod management
    // isn't working correctly for some reason. When the focused cell in the
    // tracks view is changed, QAbstractItemView::currentChanged()
    // https://github.com/qt/qtbase/blob/82015992c853b50dac167da26b8b858ac4794c66/src/widgets/itemviews/qabstractitemview.cpp#L3823
    // enables Qt::WA_InputMethodEnabled for editable columns/cells.
    // Then, special keys are interpreted as QInputMethodEvent instead of
    // QKeyEvent, which are therefore not filtered by KeyboardEventFilter and
    // some keys of built-in keyboard mappings don't work.
    // (de_DE, fr_FR, fr_CH and probably others).
    // Reset Qt::WA_InputMethodEnabled right away if no editor is open
    QTableView::currentChanged(current, previous);
    if (state() != QTableView::EditingState) {
        // Does not interfere with hovered Star delegate, the only editor that
        // is opened on hover.
        setAttribute(Qt::WA_InputMethodEnabled, false);
    }
}
#endif

void WTrackTableView::enableCachedOnly() {
    if (!m_loadCachedOnly) {
        // don't try to load and search covers, drawing only
        // covers which are already in the QPixmapCache.
        emit onlyCachedCoversAndOverviews(true);
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // Workaround for Qt6 bug https://bugreports.qt.io/browse/QTBUG-108595:
    // If 'selectedClick' is enabled Ctrl+click opens the editor instead of
    // toggling the clicked item.
    // TODO Remove or adjust version guard as soon as the bug is fixed.
    if (m_pLibrary->selectedClickEnabled()) {
        if (selectionModel()->selectedRows().size() > 1) {
            setSelectedClick(false);
        } else {
            setSelectedClick(true);
        }
    }
#endif
    QTableView::selectionChanged(selected, deselected);
}

void WTrackTableView::slotGuiTick50ms(double /*unused*/) {
    if (!isVisible()) {
        // Don't proceed if this isn't visible.
        return;
    }

    // if the user is stopped in the same row for more than 0.1 s,
    // we load un-cached cover arts as well.
    mixxx::Duration timeDelta = mixxx::Time::elapsed() - m_lastUserAction;
    if (m_loadCachedOnly && timeDelta > mixxx::Duration::fromMillis(100)) {
        // Show the currently selected track in the large cover art view and
        // highlights crate and playlists. Doing this in selectionChanged
        // slows down scrolling performance so we wait until the user has
        // stopped interacting first.
        if (m_selectionChangedSinceLastGuiTick) {
            const QModelIndexList indices = getSelectedRows();
            if (indices.size() == 1 && indices.first().isValid()) {
                // A single track has been selected
                TrackModel* pTrackModel = getTrackModel();
                if (pTrackModel) {
                    TrackPointer pTrack = pTrackModel->getTrack(indices.first());
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
        emit onlyCachedCoversAndOverviews(false);
        m_loadCachedOnly = false;
    }
}

// slot
void WTrackTableView::pasteFromSidebar() {
    pasteTracks(QModelIndex());
}

void WTrackTableView::pasteFromSidebarInPreparationWindow() {
    pasteTracks(QModelIndex());
}

// slot
void WTrackTableView::loadTrackModel(QAbstractItemModel* model, bool restoreState) {
    qDebug() << "WTrackTableView::loadTrackModel()" << model;
    if (sDebug) {
        qDebug() << windowName
                 << " -> [loadTrackModel] toggled for model: " << model
                 << " & restoreState: " << restoreState;
        qDebug() << windowName
                 << "[loadTrackModel] -> "
                    "qobject_cast<WLibraryPreparationWindow*>(parent())"
                 << qobject_cast<WLibraryPreparationWindow*>(parent());
    }

    if (qobject_cast<WLibraryPreparationWindow*>(parent())) {
        // Do nothing for WLibraryPreparationWindow
        if (sDebug) {
            qDebug() << windowName << "[loadTrackModel] -> "
                     << windowNamePreparation << " -> NO ACTION";
        }
    } else {
        // Execute for WLibrary
        if (sDebug) {
            qDebug() << windowName << "[loadTrackModel] -> toggled for "
                     << windowNameLibrary << " -> ACTION";
            // Proceed with loading the track model
            qDebug() << windowName << "[loadTrackModel]";
        }

        VERIFY_OR_DEBUG_ASSERT(model) {
            return;
        }
        TrackModel* pTrackModel = dynamic_cast<TrackModel*>(model);
        VERIFY_OR_DEBUG_ASSERT(pTrackModel) {
            return;
        }

        m_sorting = pTrackModel->hasCapabilities(TrackModel::Capability::Sorting);

        // If the model has not changed there's no need to exchange the headers
        // which would cause a small GUI freeze
        if (getTrackModel() == pTrackModel) {
            // Re-sort the table even if the track model is the same. This triggers
            // a select() if the table is dirty.
            doSortByColumn(horizontalHeader()->sortIndicatorSection(),
                    horizontalHeader()->sortIndicatorOrder());

            if (restoreState) {
                restoreCurrentViewState();
            }
            return;
        }

        setVisible(false);

        // Save the previous track model's header state
        WTrackTableViewHeader* pOldheader =
                qobject_cast<WTrackTableViewHeader*>(horizontalHeader());
        if (pOldheader) {
            pOldheader->saveHeaderState();
        }

        // rryan 12/2009 : Due to a bug in Qt, in order to switch to a model with
        // different columns than the old model, we have to create a new horizontal
        // header. Also, for some reason the WTrackTableView has to be hidden or
        // else problems occur. Since we parent the WtrackTableViewHeader's to the
        // WTrackTableView, they are automatically deleted.
        auto* pHeader = new WTrackTableViewHeader(Qt::Horizontal, this);

        // WTF(rryan) The following saves on unnecessary work on the part of
        // WTrackTableHeaderView. setHorizontalHeader() calls setModel() on the
        // current horizontal header. If this happens on the old
        // WTrackTableViewHeader, then it will save its old state, AND do the work
        // of initializing its menus on the new model. We create a new
        // WTrackTableViewHeader, so this is wasteful. Setting a temporary
        // QHeaderView here saves on setModel() calls. Since we parent the
        // QHeaderView to the WTrackTableView, it is automatically deleted.
        auto* tempHeader = new QHeaderView(Qt::Horizontal, this);
        // Tobias Rafreider: DO NOT SET SORTING TO TRUE during header replacement
        // Otherwise, setSortingEnabled(1) will immediately trigger sortByColumn()
        // For some reason this will cause 4 select statements in series
        // from which 3 are redundant --> expensive at all
        //
        // Sorting columns, however, is possible because we
        // enable clickable sorting indicators some lines below.
        // Furthermore, we connect signal 'sortIndicatorChanged'.
        //
        // Fixes Bug https://github.com/mixxxdj/mixxx/issues/5643

        setSortingEnabled(false);
        setHorizontalHeader(tempHeader);

        setModel(model);
        setHorizontalHeader(pHeader);
        pHeader->setSectionsMovable(true);
        pHeader->setSectionsClickable(true);
        // Setting this to true would render all column labels BOLD as soon as the
        // tableview is focused -- and would not restore the previous style when
        // it's unfocused. This can not be overwritten with qss, so it can screw up
        // the skin design. Also, due to selectionModel()->selectedRows() it is not
        // even useful to indicate the focused column because all columns are highlighted.
        pHeader->setHighlightSections(false);
        pHeader->setSortIndicatorShown(m_sorting);
        pHeader->setDefaultAlignment(Qt::AlignLeft);

        // Initialize all column-specific things
        for (int i = 0; i < model->columnCount(); ++i) {
            // Setup delegates according to what the model tells us
            QAbstractItemDelegate* delegate =
                    pTrackModel->delegateForColumn(i, this);
            // We need to delete the old delegates, since the docs say the view will
            // not take ownership of them.
            QAbstractItemDelegate* old_delegate = itemDelegateForColumn(i);
            // If delegate is NULL, it will unset the delegate for the column
            setItemDelegateForColumn(i, delegate);
            delete old_delegate;

            // Show or hide the column based on whether it should be shown or not.
            if (pTrackModel->isColumnInternal(i)) {
                // qDebug() << "Hiding column" << i;
                horizontalHeader()->hideSection(i);
            }
            // If Mixxx starts the first time or the header states have been cleared
            // due to database schema evolution we gonna hide all columns that may
            // contain a potential large number of NULL values.  This will hide the
            // key column by default unless the user brings it to front
            if (pTrackModel->isColumnHiddenByDefault(i) &&
                    !pHeader->hasPersistedHeaderState()) {
                // qDebug() << "Hiding column" << i;
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
            connect(pHeader,
                    &WTrackTableViewHeader::shuffle,
                    this,
                    &WTrackTableView::slotRandomSorting);

            Qt::SortOrder sortOrder;
            TrackModel::SortColumnId sortColumn =
                    pTrackModel->sortColumnIdFromColumnIndex(
                            horizontalHeader()->sortIndicatorSection());
            if (sortColumn != TrackModel::SortColumnId::Invalid) {
                // Sort by the saved sort section and order.
                sortOrder = horizontalHeader()->sortIndicatorOrder();
            } else {
                // No saved order is present. Use the TrackModel's default sort order.
                sortColumn = pTrackModel->sortColumnIdFromColumnIndex(
                        pTrackModel->defaultSortColumn());
                sortOrder = pTrackModel->defaultSortOrder();

                if (sortColumn == TrackModel::SortColumnId::Invalid) {
                    // If the TrackModel has an invalid or internal column as its default
                    // sort, find the first valid sort column and sort by that.
                    const int columnCount =
                            model->columnCount(); // just to avoid an endless
                                                  // while loop
                    for (int sortColumnIndex = 0; sortColumnIndex < columnCount;
                            sortColumnIndex++) {
                        sortColumn = pTrackModel->sortColumnIdFromColumnIndex(sortColumnIndex);
                        if (sortColumn != TrackModel::SortColumnId::Invalid) {
                            break;
                        }
                    }
                }
            }

            m_pSortColumn->set(static_cast<double>(sortColumn));
            m_pSortOrder->set(sortOrder);
            applySorting();
        }

        // Set up drag and drop behavior according to whether or not the track
        // model says it supports it.

        // Defaults
        setAcceptDrops(true);
        // Always enable drag for now (until we have a model that doesn't support
        // this.)
        setDragEnabled(true);

        // if (pTrackModel->hasCapabilities(TrackModel::Capability::ReceiveDrops)) {
        //     setDragDropMode(QAbstractItemView::DragDrop);
        //     setDropIndicatorShown(true);
        //     // viewport()->setAcceptDrops(true);
        // } else {
        //     setDragDropMode(QAbstractItemView::DragOnly);
        // }

        // Possible giant fuckup alert - It looks like Qt has something like these
        // caps built-in, see http://doc.trolltech.com/4.5/qt.html#ItemFlag-enum and
        // the flags(...) function that we're already using in LibraryTableModel. I
        // haven't been able to get it to stop us from using a model as a drag
        // target though, so my hacks above may not be completely unjustified.

        // Now also apply the current font to the new header
        pHeader->setFont(font());

        setVisible(true);

        // trigger restoring scrollBar position, selection etc.
        if (restoreState) {
            restoreCurrentViewState();
        }
        initTrackMenu();
    }
}

void WTrackTableView::loadTrackModelInPreparationWindow(
        QAbstractItemModel* model, bool restoreState) {
    qDebug() << "WTrackTableView::loadTrackModel()" << model;
    if (sDebug) {
        qDebug() << windowName
                 << " -> [loadTrackModelInPreparationWindow] toggled for model: " << model
                 << " & restoreState: " << restoreState;
        qDebug() << windowName
                 << "[loadTrackModelInPreparationWindow] -> "
                    "qobject_cast<WLibraryPreparationWindow*>(parent())"
                 << qobject_cast<WLibraryPreparationWindow*>(parent());
    }
    if (qobject_cast<WLibraryPreparationWindow*>(parent())) {
        // Execute for WLibraryPreparationWindow
        if (sDebug) {
            qDebug() << windowName
                     << "[loadTrackModelInPreparationWindow] -> toggled for "
                     << windowNamePreparation << " -> ACTION";
            // Proceed with loading the track model
            qDebug() << windowName << "[loadTrackModelInPreparationWindow]";
        }

        VERIFY_OR_DEBUG_ASSERT(model) {
            return;
        }
        TrackModel* pTrackModel = dynamic_cast<TrackModel*>(model);
        VERIFY_OR_DEBUG_ASSERT(pTrackModel) {
            return;
        }

        m_sorting = pTrackModel->hasCapabilities(TrackModel::Capability::Sorting);

        // If the model has not changed there's no need to exchange the headers
        // which would cause a small GUI freeze
        if (getTrackModel() == pTrackModel) {
            // Re-sort the table even if the track model is the same. This triggers
            // a select() if the table is dirty.
            doSortByColumn(horizontalHeader()->sortIndicatorSection(),
                    horizontalHeader()->sortIndicatorOrder());

            if (restoreState) {
                restoreCurrentViewState();
            }
            return;
        }

        setVisible(false);

        // Save the previous track model's header state
        WTrackTableViewHeader* pOldheader =
                qobject_cast<WTrackTableViewHeader*>(horizontalHeader());
        if (pOldheader) {
            pOldheader->saveHeaderState();
        }

        // rryan 12/2009 : Due to a bug in Qt, in order to switch to a model with
        // different columns than the old model, we have to create a new horizontal
        // header. Also, for some reason the WTrackTableView has to be hidden or
        // else problems occur. Since we parent the WtrackTableViewHeader's to the
        // WTrackTableView, they are automatically deleted.
        auto* pHeader = new WTrackTableViewHeader(Qt::Horizontal, this);

        // WTF(rryan) The following saves on unnecessary work on the part of
        // WTrackTableHeaderView. setHorizontalHeader() calls setModel() on the
        // current horizontal header. If this happens on the old
        // WTrackTableViewHeader, then it will save its old state, AND do the work
        // of initializing its menus on the new model. We create a new
        // WTrackTableViewHeader, so this is wasteful. Setting a temporary
        // QHeaderView here saves on setModel() calls. Since we parent the
        // QHeaderView to the WTrackTableView, it is automatically deleted.
        auto* tempHeader = new QHeaderView(Qt::Horizontal, this);
        // Tobias Rafreider: DO NOT SET SORTING TO TRUE during header replacement
        // Otherwise, setSortingEnabled(1) will immediately trigger sortByColumn()
        // For some reason this will cause 4 select statements in series
        // from which 3 are redundant --> expensive at all
        //
        // Sorting columns, however, is possible because we
        // enable clickable sorting indicators some lines below.
        // Furthermore, we connect signal 'sortIndicatorChanged'.
        //
        // Fixes Bug https://github.com/mixxxdj/mixxx/issues/5643

        setSortingEnabled(false);
        setHorizontalHeader(tempHeader);

        setModel(model);
        setHorizontalHeader(pHeader);
        pHeader->setSectionsMovable(true);
        pHeader->setSectionsClickable(true);
        // Setting this to true would render all column labels BOLD as soon as the
        // tableview is focused -- and would not restore the previous style when
        // it's unfocused. This can not be overwritten with qss, so it can screw up
        // the skin design. Also, due to selectionModel()->selectedRows() it is not
        // even useful to indicate the focused column because all columns are highlighted.
        pHeader->setHighlightSections(false);
        pHeader->setSortIndicatorShown(m_sorting);
        pHeader->setDefaultAlignment(Qt::AlignLeft);

        // Initialize all column-specific things
        for (int i = 0; i < model->columnCount(); ++i) {
            // Setup delegates according to what the model tells us
            QAbstractItemDelegate* delegate =
                    pTrackModel->delegateForColumn(i, this);
            // We need to delete the old delegates, since the docs say the view will
            // not take ownership of them.
            QAbstractItemDelegate* old_delegate = itemDelegateForColumn(i);
            // If delegate is NULL, it will unset the delegate for the column
            setItemDelegateForColumn(i, delegate);
            delete old_delegate;

            // Show or hide the column based on whether it should be shown or not.
            if (pTrackModel->isColumnInternal(i)) {
                // qDebug() << "Hiding column" << i;
                horizontalHeader()->hideSection(i);
            }
            // If Mixxx starts the first time or the header states have been cleared
            // due to database schema evolution we gonna hide all columns that may
            // contain a potential large number of NULL values.  This will hide the
            // key column by default unless the user brings it to front
            if (pTrackModel->isColumnHiddenByDefault(i) &&
                    !pHeader->hasPersistedHeaderState()) {
                // qDebug() << "Hiding column" << i;
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
            connect(pHeader,
                    &WTrackTableViewHeader::shuffle,
                    this,
                    &WTrackTableView::slotRandomSorting);

            Qt::SortOrder sortOrder;
            TrackModel::SortColumnId sortColumn =
                    pTrackModel->sortColumnIdFromColumnIndex(
                            horizontalHeader()->sortIndicatorSection());
            if (sortColumn != TrackModel::SortColumnId::Invalid) {
                // Sort by the saved sort section and order.
                sortOrder = horizontalHeader()->sortIndicatorOrder();
            } else {
                // No saved order is present. Use the TrackModel's default sort order.
                sortColumn = pTrackModel->sortColumnIdFromColumnIndex(
                        pTrackModel->defaultSortColumn());
                sortOrder = pTrackModel->defaultSortOrder();

                if (sortColumn == TrackModel::SortColumnId::Invalid) {
                    // If the TrackModel has an invalid or internal column as its default
                    // sort, find the first valid sort column and sort by that.
                    const int columnCount =
                            model->columnCount(); // just to avoid an endless
                                                  // while loop
                    for (int sortColumnIndex = 0; sortColumnIndex < columnCount;
                            sortColumnIndex++) {
                        sortColumn = pTrackModel->sortColumnIdFromColumnIndex(sortColumnIndex);
                        if (sortColumn != TrackModel::SortColumnId::Invalid) {
                            break;
                        }
                    }
                }
            }

            m_pSortColumn->set(static_cast<double>(sortColumn));
            m_pSortOrder->set(sortOrder);
            applySorting();
        }

        // Set up drag and drop behavior according to whether or not the track
        // model says it supports it.

        // Defaults
        setAcceptDrops(true);
        // Always enable drag for now (until we have a model that doesn't support
        // this.)
        setDragEnabled(true);

        if (pTrackModel->hasCapabilities(TrackModel::Capability::ReceiveDrops)) {
            setDragDropMode(QAbstractItemView::DragDrop);
            setDropIndicatorShown(true);
            // viewport()->setAcceptDrops(true);
        } else {
            setDragDropMode(QAbstractItemView::DragOnly);
        }

        // Possible giant fuckup alert - It looks like Qt has something like these
        // caps built-in, see http://doc.trolltech.com/4.5/qt.html#ItemFlag-enum and
        // the flags(...) function that we're already using in LibraryTableModel. I
        // haven't been able to get it to stop us from using a model as a drag
        // target though, so my hacks above may not be completely unjustified.

        // Now also apply the current font to the new header
        pHeader->setFont(font());

        setVisible(true);

        // trigger restoring scrollBar position, selection etc.
        if (restoreState) {
            restoreCurrentViewState();
        }
        initTrackMenu();
    } else {
        // Do nothing for WLibrary
        if (sDebug) {
            qDebug() << windowName << "[loadTrackModelInPreparationWindow] -> "
                     << "windowNameLibrary" << " -> NO ACTION";
        }
    }
}

void WTrackTableView::initTrackMenu() {
    auto* pTrackModel = getTrackModel();
    DEBUG_ASSERT(pTrackModel);

    if (m_pTrackMenu) {
        m_pTrackMenu->deleteLater();
    }

    m_pTrackMenu = make_parented<WTrackMenu>(this,
            m_pConfig,
            m_pLibrary,
            WTrackMenu::Feature::All,
            pTrackModel);
    connect(m_pTrackMenu.get(),
            &WTrackMenu::loadTrackToPlayer,
            this,
            &WLibraryTableView::loadTrackToPlayer);

    connect(m_pTrackMenu,
            &WTrackMenu::trackMenuVisible,
            this,
            [this](bool visible) {
                emit trackMenuVisible(visible);
            });
    // after removing tracks from the view via track menu, restore a usable
    // selection/currentIndex for navigation via keyboard & controller
    connect(m_pTrackMenu,
            &WTrackMenu::restoreCurrentViewStateOrIndex,
            this,
            &WTrackTableView::slotrestoreCurrentIndex);
}

// slot
void WTrackTableView::slotMouseDoubleClicked(const QModelIndex& index) {
    // Read the current TrackDoubleClickAction setting
    // TODO simplify this casting madness
    int doubleClickActionConfigValue =
            m_pConfig->getValue(mixxx::library::prefs::kTrackDoubleClickActionConfigKey,
                    static_cast<int>(DlgPrefLibrary::TrackDoubleClickAction::LoadToDeck));
    DlgPrefLibrary::TrackDoubleClickAction doubleClickAction =
            static_cast<DlgPrefLibrary::TrackDoubleClickAction>(
                    doubleClickActionConfigValue);

    if (doubleClickAction == DlgPrefLibrary::TrackDoubleClickAction::Ignore) {
        return;
    }

    auto* pTrackModel = getTrackModel();
    VERIFY_OR_DEBUG_ASSERT(pTrackModel) {
        return;
    }

    if (doubleClickAction == DlgPrefLibrary::TrackDoubleClickAction::LoadToDeck &&
            pTrackModel->hasCapabilities(
                    TrackModel::Capability::LoadToDeck)) {
        TrackPointer pTrack = pTrackModel->getTrack(index);
        if (pTrack) {
            emit loadTrack(pTrack);
        }
    } else if (doubleClickAction == DlgPrefLibrary::TrackDoubleClickAction::AddToAutoDJBottom &&
            pTrackModel->hasCapabilities(
                    TrackModel::Capability::AddToAutoDJ)) {
        addToAutoDJ(PlaylistDAO::AutoDJSendLoc::BOTTOM);
    } else if (doubleClickAction == DlgPrefLibrary::TrackDoubleClickAction::AddToAutoDJTop &&
            pTrackModel->hasCapabilities(
                    TrackModel::Capability::AddToAutoDJ)) {
        addToAutoDJ(PlaylistDAO::AutoDJSendLoc::TOP);
    }
}

TrackModel::SortColumnId WTrackTableView::getColumnIdFromCurrentIndex() {
    TrackModel* pTrackModel = getTrackModel();
    VERIFY_OR_DEBUG_ASSERT(pTrackModel) {
        return TrackModel::SortColumnId::Invalid;
    }
    return pTrackModel->sortColumnIdFromColumnIndex(currentIndex().column());
}

void WTrackTableView::assignPreviousTrackColor() {
    TrackModel* pTrackModel = getTrackModel();
    if (!pTrackModel) {
        return;
    }
    const QModelIndexList indices = getSelectedRows();
    if (indices.isEmpty()) {
        return;
    }

    QModelIndex index = indices.at(0);
    TrackPointer pTrack = pTrackModel->getTrack(index);
    if (pTrack) {
        ColorPaletteSettings colorPaletteSettings(m_pConfig);
        ColorPalette colorPalette = colorPaletteSettings.getTrackColorPalette();
        mixxx::RgbColor::optional_t color = pTrack->getColor();
        pTrack->setColor(colorPalette.previousColor(color));
    }
}

void WTrackTableView::assignNextTrackColor() {
    TrackModel* pTrackModel = getTrackModel();
    if (!pTrackModel) {
        return;
    }
    const QModelIndexList indices = getSelectedRows();
    if (indices.isEmpty()) {
        return;
    }

    QModelIndex index = indices.at(0);
    TrackPointer pTrack = pTrackModel->getTrack(index);
    if (pTrack) {
        ColorPaletteSettings colorPaletteSettings(m_pConfig);
        ColorPalette colorPalette = colorPaletteSettings.getTrackColorPalette();
        mixxx::RgbColor::optional_t color = pTrack->getColor();
        pTrack->setColor(colorPalette.nextColor(color));
    }
}

void WTrackTableView::slotPurge() {
    TrackModel* pTrackModel = getTrackModel();
    if (!pTrackModel) {
        return;
    }
    const QModelIndexList indices = getSelectedRows();
    if (indices.isEmpty()) {
        return;
    }
    saveCurrentIndex();
    pTrackModel->purgeTracks(indices);
    restoreCurrentIndex();
}

void WTrackTableView::slotDeleteTracksFromDisk() {
    const QModelIndexList indices = getSelectedRows();
    if (indices.isEmpty()) {
        return;
    }
    saveCurrentIndex();
    m_pTrackMenu->loadTrackModelIndices(indices);
    m_pTrackMenu->slotRemoveFromDisk();
    // WTrackmenu emits restoreCurrentViewStateOrIndex()
}

void WTrackTableView::slotUnhide() {
    TrackModel* pTrackModel = getTrackModel();
    if (!pTrackModel) {
        return;
    }
    const QModelIndexList indices = getSelectedRows();
    if (indices.isEmpty()) {
        return;
    }
    saveCurrentIndex();
    pTrackModel->unhideTracks(indices);
    restoreCurrentIndex();
}

void WTrackTableView::slotShowHideTrackMenu(bool show) {
    VERIFY_OR_DEBUG_ASSERT(m_pTrackMenu.get()) {
        initTrackMenu();
    }
    if (show == m_pTrackMenu->isVisible()) {
        emit trackMenuVisible(show);
        return;
    }
    if (show) {
        const auto selectedIndices = selectionModel()->selectedIndexes();
        if (selectedIndices.isEmpty()) {
            // If selection is empty, contextMenuEvent() won't work anyway.
            return;
        }

        // Show at current index if it's valid. When using only a controller for
        // for track selection, there's only on row selected.
        // If it's not part of the selection, like when Ctrl+click was used to
        // deselect a row, we use the first selected row and the column of the
        // current index.
        // Else show at cursor position.
        QPoint evPos;
        const auto currIdx = currentIndex();
        if (currIdx.isValid()) {
            if (selectedIndices.contains(currIdx)) {
                evPos = visualRect(currIdx).center();
            } else {
                // use first selected row and column of current index
                const QList<int> rows = getSelectedRowNumbers();
                const QModelIndex tempIdx = currIdx.siblingAtRow(rows.first());
                evPos = visualRect(tempIdx).center();
            }
            // If the selected row is outside the table's viewport (above or below),
            // let the menu unfold from the bottom center to hopefully clarify
            // that the menu belongs to the library and not to some deck widget.
            if (!viewport()->rect().contains(evPos)) {
                evPos = QPoint(viewport()->rect().center().x(),
                        viewport()->rect().bottom());
            } else {
                // The viewports start below the header, but mapToGlobal() uses
                // the rect() as reference. Add header height to y and we're good.
                // Assumes the view shows at least one row which is at least as tall
                // as the header, else we'll end up below the viewport, then it's
                // up to QMenu to find an adequate popup position.
                evPos += QPoint(0, horizontalHeader()->height());
            }
            evPos = mapToGlobal(evPos);
        } else {
            evPos = QCursor::pos();
        }
        showTrackMenu(evPos, indexAt(evPos));
    } else {
        m_pTrackMenu->close();
    }
}

void WTrackTableView::contextMenuEvent(QContextMenuEvent* pEvent) {
    VERIFY_OR_DEBUG_ASSERT(m_pTrackMenu.get()) {
        initTrackMenu();
    }
    pEvent->accept();

    showTrackMenu(pEvent->globalPos(), indexAt(pEvent->pos()));
}

void WTrackTableView::showTrackMenu(const QPoint pos, const QModelIndex& index) {
    VERIFY_OR_DEBUG_ASSERT(m_pTrackMenu.get()) {
        return;
    }
    // Update track indices in context menu
    const QModelIndexList indices = getSelectedRows();
    if (indices.isEmpty()) {
        return;
    }
    m_pTrackMenu->loadTrackModelIndices(indices);
    m_pTrackMenu->setTrackPropertyName(columnNameOfIndex(index));

    saveCurrentIndex();

    m_pTrackMenu->popup(pos);
    // WTrackmenu emits restoreCurrentViewStateOrIndex() on hide if required
}

QString WTrackTableView::columnNameOfIndex(const QModelIndex& index) const {
    if (!index.isValid()) {
        return {};
    }
    VERIFY_OR_DEBUG_ASSERT(model()) {
        return {};
    }
    return model()->headerData(
                          index.column(),
                          Qt::Horizontal,
                          TrackModel::kHeaderNameRole)
            .toString();
}

void WTrackTableView::onSearch(const QString& text) {
    TrackModel* pTrackModel = getTrackModel();
    if (!pTrackModel) {
        return;
    }

    saveCurrentViewState();
    bool queryIsLessSpecific = SearchQueryParser::queryIsLessSpecific(
            pTrackModel->currentSearch(), text);
    QList<TrackId> selectedTracks = getSelectedTrackIds();
    TrackId prevTrack = getCurrentTrackId();
    saveCurrentIndex();
    pTrackModel->search(text);
    if (queryIsLessSpecific) {
        // If the user removed query terms, we try to select the same
        // tracks as before
        setCurrentTrackId(prevTrack, m_prevColumn);
        setSelectedTracks(selectedTracks);
    } else {
        // The user created a more specific search query, try to restore a
        // previous state
        if (!restoreCurrentViewState()) {
            // We found no saved state for this query, try to select the
            // tracks last active, if they are part of the result set
            if (!setCurrentTrackId(prevTrack, m_prevColumn)) {
                // if the last focused track is not present try to focus the
                // respective index and scroll there
                restoreCurrentIndex();
            }
            setSelectedTracks(selectedTracks);
        }
    }
}

void WTrackTableView::onShow() {
}

void WTrackTableView::mousePressEvent(QMouseEvent* pEvent) {
    DragAndDropHelper::mousePressed(pEvent);
    WLibraryTableView::mousePressEvent(pEvent);
}

void WTrackTableView::mouseMoveEvent(QMouseEvent* pEvent) {
    // Only use this for drag and drop if the LeftButton is pressed we need to
    // check for this because mousetracking is activated and this function is
    // called every time the mouse is moved -- kain88 May 2012
    if (pEvent->buttons() != Qt::LeftButton) {
        // Needed for mouse-tracking to fire entered() events. If we call this
        // outside of this if statement then we get 'ghost' drags. See issue
        // #6507
        WLibraryTableView::mouseMoveEvent(pEvent);
        return;
    }

    TrackModel* pTrackModel = getTrackModel();
    if (!pTrackModel) {
        return;
    }
    //qDebug() << "MouseMoveEvent";

    if (DragAndDropHelper::mouseMoveInitiatesDrag(pEvent)) {
        // Iterate over selected rows and append each item's location url to a list.
        QList<QString> locations;
        const QModelIndexList indices = getSelectedRows();
        if (sDebug) {
            qDebug() << "[WTRACKTABLEVIEW] -> mouseMoveInitiatesDrag -> "
                        "DragDrop indices "
                     << indices;
            qDebug() << "[WTRACKTABLEVIEW] -> mouseMoveInitiatesDrag -> "
                        "DragDrop getSelectedRows() "
                     << getSelectedRows();
        }

        for (const QModelIndex& index : indices) {
            if (!index.isValid()) {
                if (sDebug) {
                    qDebug() << "[WTRACKTABLEVIEW] -> mouseMoveInitiatesDrag "
                                "-> DragDrop index is NOT valid ";
                }
                continue;
            }
            locations.append(pTrackModel->getTrackLocation(index));
            if (sDebug) {
                qDebug() << "[WTRACKTABLEVIEW] -> mouseMoveInitiatesDrag -> DragDrop "
                            "pTrackModel->getTrackLocation(index) "
                         << pTrackModel->getTrackLocation(index);
            }
        }
        if (sDebug) {
            qDebug() << "[WTRACKTABLEVIEW] -> mouseMoveInitiatesDrag -> "
                        "DragDrop locations "
                     << locations;
        }

        // EVE
        // Note Setting for d&d in prepwindow

        if (qobject_cast<WLibraryPreparationWindow*>(parent())) {
            // WLibraryPreparationWindow
            if (sDebug) {
                qDebug() << "[WTRACKTABLEVIEW] -> mouseMoveInitiatesDrag -> "
                            "DragDrop librarypreparationwindow ";
                qDebug() << "[WTRACKTABLEVIEW] -> mouseMoveInitiatesDrag -> "
                            "DragDrop locations "
                         << locations;
            }
            DragAndDropHelper::dragTrackLocations(locations, this, "librarypreparationwindow");
        } else if (qobject_cast<WLibrary*>(parent())) {
            // WLibrary
            if (sDebug) {
                qDebug() << "[WTRACKTABLEVIEW] -> mouseMoveInitiatesDrag -> DragDrop wlibrary ";
                qDebug() << "[WTRACKTABLEVIEW] -> mouseMoveInitiatesDrag -> "
                            "DragDrop locations "
                         << locations;
            }
            DragAndDropHelper::dragTrackLocations(locations, this, "library");
        } else {
            if (sDebug) {
                qDebug() << "[WTRACKTABLEVIEW] -> mouseMoveInitiatesDrag -> DragDrop else autodj ";
                qDebug() << "[WTRACKTABLEVIEW] -> mouseMoveInitiatesDrag -> "
                            "DragDrop locations "
                         << locations;
            }
            DragAndDropHelper::dragTrackLocations(locations, this, "Auto DJ");
        }
    }
}

// Drag enter event, happens when a dragged item hovers over the track table view
void WTrackTableView::dragEnterEvent(QDragEnterEvent * event) {
    auto* pTrackModel = getTrackModel();
    if (!pTrackModel || pTrackModel->isLocked() || !event->mimeData()->hasUrls()) {
        event->ignore();
        return;
    }
    //qDebug() << "dragEnterEvent" << event->mimeData()->formats();
    if (event->source() == this) {
        if (pTrackModel->hasCapabilities(TrackModel::Capability::Reorder)) {
            event->acceptProposedAction();
        }
    } else if (DragAndDropHelper::dragEnterAccept(*event->mimeData(),
                       "library",
                       true,
                       true)) {
        event->acceptProposedAction();
    } else if (DragAndDropHelper::dragEnterAccept(*event->mimeData(),
                       "preparationwindow",
                       true,
                       true)) {
        event->acceptProposedAction();
    }
}

void WTrackTableView::dragLeaveEvent(QDragLeaveEvent* /*event*/) {
    // Reset drop indicator
    m_dropRow = -1;
    update();
}

// Drag move event, happens when a dragged item hovers over the track table view...
// It changes the drop handle to a "+" when the drag content is acceptable.
// Without it, the following drop is ignored.
void WTrackTableView::dragMoveEvent(QDragMoveEvent * event) {
    auto* pTrackModel = getTrackModel();
    if (!pTrackModel || pTrackModel->isLocked() || !event->mimeData()->hasUrls()) {
        event->ignore();
        return;
    }
    // Needed to allow auto-scrolling
    WLibraryTableView::dragMoveEvent(event);

    int newDropRow = -1;

    //qDebug() << "dragMoveEvent" << event->mimeData()->formats();
    if (event->source() == this) {
        if (pTrackModel->hasCapabilities(TrackModel::Capability::ReceiveDrops)) {
            event->acceptProposedAction();
        } else {
            event->ignore();
        }
    } else {
        event->acceptProposedAction();
    }

    if (pTrackModel->hasCapabilities(TrackModel::Capability::Reorder) &&
            event->isAccepted()) {
        // If this is a playlist, calculate the position where the track(s)
        // will be inserted.
        // Handle above/below row v-center like in dropEvent()
        int posY = event->position().toPoint().y();
        int hoverRow = rowAt(posY);       // is -1 below last row
        int height = rowHeight(hoverRow); // is 0 below last row
        int yBelow = posY + static_cast<int>(height / 2);
        newDropRow = rowAt(yBelow);
        if (newDropRow == -1) {
            newDropRow = model()->rowCount();
        }
    }

    if (newDropRow != m_dropRow) {
        m_dropRow = newDropRow;
        update();
    }
}

// Drag-and-drop "drop" event. Occurs when something is dropped onto the track table view
void WTrackTableView::dropEvent(QDropEvent * event) {
    TrackModel* pTrackModel = getTrackModel();
    // We only do things to the TrackModel in this method so if we don't have
    // one we should just bail.
    if (!pTrackModel || pTrackModel->isLocked() || !event->mimeData()->hasUrls()) {
        event->ignore();
        return;
    }

    QItemSelectionModel* pSelectionModel = selectionModel();
    VERIFY_OR_DEBUG_ASSERT(pSelectionModel != nullptr) {
        qWarning() << "No selection model available";
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QPoint position = event->position().toPoint();
#else
    QPoint position = event->pos();
#endif
    int dropRow = rowAt(position.y()); // is -1 below last row
    int height = rowHeight(dropRow);   // is 0 below last row
    QPoint pointOfRowBelowSeam(position.x(), position.y() + height / 2);
    QModelIndex destIndex = indexAt(pointOfRowBelowSeam);

    //qDebug() << "destIndex.row() is" << destIndex.row();

    // Drag and drop within this widget (track reordering)
    if (event->source() == this &&
            pTrackModel->hasCapabilities(TrackModel::Capability::Reorder)) {
        // Note the above code hides an ambiguous case when a
        // playlist is empty. For that reason, we can't factor that
        // code out to be common for both internal reordering
        // and external drag-and-drop. With internal reordering,
        // you can't have an empty playlist. :)

        // Save a list of rows (just plain ints) so we don't get screwed over
        // when the QModelIndexes all become invalid (eg. after moveTrack()
        // or addTrack())
        QList<int> selectedRows = getSelectedRowNumbers();
        if (selectedRows.isEmpty()) {
            return;
        }

        moveRows(selectedRows, destIndex.row());
    } else { // Drag and drop inside Mixxx is only for few rows, bulks happen here
        // Reset the selected tracks (if you had any tracks highlighted, it
        // clears them)
        pSelectionModel->clear();

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

        // Add all the dropped URLs/tracks to the track model (playlist/crate)
        int numNewRows;
        {
            const QList<mixxx::FileInfo> trackFileInfos =
                    DragAndDropHelper::supportedTracksFromUrls(
                            event->mimeData()->urls(), false, true);
            QList<QString> trackLocations;
            trackLocations.reserve(trackFileInfos.size());
            for (const auto& fileInfo : trackFileInfos) {
                trackLocations.append(fileInfo.location());
            }
            numNewRows = pTrackModel->addTracks(destIndex, trackLocations);
            DEBUG_ASSERT(numNewRows >= 0);
            DEBUG_ASSERT(numNewRows <= trackFileInfos.size());
        }

        // Create the selection, but only if the track model supports
        // reordering. (eg. crates don't support reordering/indexes)
        if (pTrackModel->hasCapabilities(TrackModel::Capability::Reorder)) {
            // TODO Also set current index to have good starting point for navigation?
            for (int i = selectionStartRow; i < selectionStartRow + numNewRows; i++) {
                pSelectionModel->select(model()->index(i, 0),
                        QItemSelectionModel::Select |
                                QItemSelectionModel::Rows);
            }
        }
    }

    event->acceptProposedAction();
    updateGeometries();
    verticalScrollBar()->setValue(vScrollBarPos);

    m_dropRow = -1;
    update();
}

void WTrackTableView::paintEvent(QPaintEvent* e) {
    QTableView::paintEvent(e);

    if (m_dropRow < 0) {
        return;
    }

    // Draw drop indicator line
    int y;
    int rowsTotal = model()->rowCount();
    if (m_dropRow < rowsTotal) {
        y = rowViewportPosition(m_dropRow);
    } else {
        y = rowViewportPosition(rowsTotal - 1) + rowHeight(0);
    }
    // Line is 2px tall. Make sure the entire line is visible.
    if (y == 0) {
        y += 1;
    } else if (y + 1 > viewport()->height()) {
        y = viewport()->height() - 1;
    }

    QPainter painter(viewport());
    QPen pen(m_dropIndicatorColor, 2, Qt::SolidLine);
    painter.setPen(pen);
    painter.drawLine(0, y, viewport()->width(), y);
}

QModelIndexList WTrackTableView::getSelectedRows() const {
    if (getTrackModel() == nullptr) {
        return {};
    }
    QItemSelectionModel* pSelectionModel = selectionModel();
    VERIFY_OR_DEBUG_ASSERT(pSelectionModel != nullptr) {
        qWarning() << "[WTRACKTABLEVIEW] -> getSelectedRows() -> No selection model available";
        return {};
    }
    return pSelectionModel->selectedRows();
}

QList<int> WTrackTableView::getSelectedRowNumbers() const {
    const QModelIndexList indices = getSelectedRows();
    QList<int> selectedRows;
    for (const QModelIndex& idx : indices) {
        selectedRows.append(idx.row());
    }
    std::sort(selectedRows.begin(), selectedRows.end());
    return selectedRows;
}

TrackModel* WTrackTableView::getTrackModel() const {
    TrackModel* pTrackModel = dynamic_cast<TrackModel*>(model());
    return pTrackModel;
}

namespace {
QModelIndex calculateCutIndex(const QModelIndex& currentIndex,
        const QModelIndexList& removedIndices) {
    if (removedIndices.empty()) {
        return QModelIndex();
    }
    const int row = currentIndex.row();
    int rowAfterRemove = row;
    for (const auto& removeIndex : removedIndices) {
        if (removeIndex.row() < row) {
            rowAfterRemove--;
        }
    }
    return currentIndex.siblingAtRow(rowAfterRemove);
}
} // namespace

void WTrackTableView::removeSelectedTracks() {
    const QModelIndexList indices = getSelectedRows();
    const QModelIndex newIndex = calculateCutIndex(currentIndex(), indices);
    getTrackModel()->removeTracks(indices);
    setCurrentIndex(newIndex);
}

void WTrackTableView::cutSelectedTracks() {
    const QModelIndexList indices = getSelectedRows();
    const QModelIndex newIndex = calculateCutIndex(currentIndex(), indices);
    getTrackModel()->cutTracks(indices);
    setCurrentIndex(newIndex);
}

void WTrackTableView::copySelectedTracks() {
    const QModelIndexList indices = getSelectedRows();
    getTrackModel()->copyTracks(indices);
}

void WTrackTableView::pasteTracks(const QModelIndex& index) {
    TrackModel* trackModel = getTrackModel();
    if (!trackModel) {
        return;
    }

    const auto prevIdx = currentIndex();

    const QList<int> rows = trackModel->pasteTracks(index);
    if (rows.empty()) {
        return;
    }

    updateGeometries();
    const auto lastVisibleRow = rowAt(height());

    // Use selectRow to scroll to the first or last pasted row. We would use
    // scrollTo but this is broken. This solution was already used elsewhere
    // in this way.
    if (rows.back() > lastVisibleRow) {
        selectRow(rows.back());
    } else {
        selectRow(rows.front());
    }

    const auto idx = prevIdx.siblingAtRow(rows.back());
    QItemSelectionModel* pSelectionModel = selectionModel();
    if (pSelectionModel && idx.isValid()) {
        pSelectionModel->setCurrentIndex(idx,
                QItemSelectionModel::SelectCurrent | QItemSelectionModel::Select);
    }

    // Select all the rows that we pasted
    for (const auto row : rows) {
        selectionModel()->select(model()->index(row, 0),
                QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
}

void WTrackTableView::moveRows(QList<int> selectedRowsIn, int destRow) {
    TrackModel* pTrackModel = getTrackModel();
    if (!pTrackModel) {
        return;
    }
    if (selectedRowsIn.isEmpty()) {
        return;
    }

    // Note(RRyan/Max Linke):
    // The biggest subtlety in the way I've done this track reordering code
    // is that as soon as we've moved ANY track, all of our QModelIndexes probably
    // get screwed up. The starting point for the logic below is to say screw
    // the QModelIndexes, and just keep a list of row numbers to work from.
    // That ends up making the logic simpler and the behavior totally predictable,
    // which lets us do nice things like "restore" the selection model.

    // The model indices are sorted so that we remove the tracks from the table
    // in ascending order. This is necessary because if track A is above track B in
    // the table, and you remove track A, the model index for track B will change.
    // Sorting the indices first means we don't have to worry about this.
    QList<int> selectedRows = std::move(selectedRowsIn);

    // An invalid destination row means we're supposed to move the selection to the end.
    // Happens when we drop tracks into the void below the last track.
    destRow = destRow < 0 ? model()->rowCount() : destRow;
    // Required for refocusing the correct column and restoring the selection
    // after we moved. Use 0 if the index is invalid for some reason.
    int idxCol = std::max(0, currentIndex().column());
    int selectedRowCount = selectedRows.count();
    int selectionRestoreStartRow = destRow;
    int firstSelRow = selectedRows.first();
    int lastSelRow = selectedRows.last();

    if (destRow == firstSelRow && selectedRowCount == 1) {
        return; // no-op
    }

    // Adjust first row of new selection
    if (destRow >= firstSelRow && destRow <= lastSelRow) {
        // Destination is inside the selection.
        if (selectedRowCount == lastSelRow - firstSelRow + 1) {
            // If we drag a contiguous selection of multiple tracks and drop them
            // somewhere inside that same selection, we obviously have nothing to do.
            // This is also a good way to abort accidental drags.
            return;
        }
        // Non-continuous selection:
        if (destRow == firstSelRow) {
            // Consolidate selection at first selected row.
            // Remove consecutive rows (they are already in place) until we find
            // the first gap in the selection.
            // Use the row after that continuous part as destination.
            while (destRow == firstSelRow) {
                selectedRows.removeFirst();
                firstSelRow = selectedRows.first();
                destRow++;
            }
        } else {
            return;
        }
    }

    if (destRow < firstSelRow) {
        // If we're moving the tracks UP, reverse the order of the row selection
        // to make the algorithm below work as it is
        std::sort(selectedRows.begin(),
                selectedRows.end(),
                std::greater<int>());
    } else { // Down
        if (destRow > lastSelRow) {
            // If we're moving the tracks DOWN, adjust the first row to reselect
            selectionRestoreStartRow =
                    selectionRestoreStartRow - selectedRowCount;
        }
    }

    // For each row that needs to be moved...
    while (!selectedRows.isEmpty()) {
        int movedRow = selectedRows.takeFirst(); // Remember it's row index
        // Move it
        pTrackModel->moveTrack(model()->index(movedRow, 0), model()->index(destRow, 0));

        // Move the row indices for rows that got bumped up
        // into the void we left, or down because of the new spot
        // we're taking.
        for (int i = 0; i < selectedRows.count(); i++) {
            if ((selectedRows[i] > movedRow) && ((destRow > selectedRows[i]))) {
                selectedRows[i] = selectedRows[i] - 1;
            } else if ((selectedRows[i] < movedRow) &&
                    (destRow < selectedRows[i])) {
                selectedRows[i] = selectedRows[i] + 1;
            }
        }
    }

    // Set current index.
    // TODO If we moved down, pick the last selected row?
    // int idxRow = destRow < firstSelRow
    //         ? selectionRestoreStartRow
    //         : selectionRestoreStartRow + selectedRowCount - 1;
    const auto idx = model()->index(selectionRestoreStartRow, idxCol);
    QItemSelectionModel* pSelectionModel = selectionModel();
    if (pSelectionModel && idx.isValid()) {
        pSelectionModel->setCurrentIndex(idx,
                QItemSelectionModel::SelectCurrent | QItemSelectionModel::Select);
    }

    // Select the moved rows (restore previous selection)
    for (int i = 0; i < selectedRowCount; i++) {
        selectionModel()->select(model()->index(selectionRestoreStartRow + i, idxCol),
                QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
}

void WTrackTableView::moveSelectedTracks(QKeyEvent* event) {
    TrackModel* pTrackModel = getTrackModel();
    if (!pTrackModel || pTrackModel->isLocked()) {
        return;
    }

    QList<int> selectedRows = getSelectedRowNumbers();
    if (selectedRows.isEmpty()) {
        return;
    }

    bool up = event->key() == Qt::Key_Up;
    bool pageUp = event->key() == Qt::Key_PageUp;
    bool down = event->key() == Qt::Key_Down;
    bool pageDown = event->key() == Qt::Key_PageDown;
    bool top = event->key() == Qt::Key_Home;
    bool bottom = event->key() == Qt::Key_End;

    // Check if we have a continuous selection.
    int firstSelRow = selectedRows.first();
    int lastSelRow = selectedRows.last();
    int rowCount = model()->rowCount();
    bool continuous = selectedRows.length() == lastSelRow - firstSelRow + 1;
    if (continuous &&
            (((up || pageUp || top) && firstSelRow == 0) ||
                    ((down || pageDown || bottom) && lastSelRow == rowCount - 1))) {
        // Continuous selection with no more rows to skip in the desired
        // direction, further Up/Down would wrap around the current index.
        // Ignore.
        return;
    }

    int destRow = 0; // for Home (top)
    if (bottom || ((down || pageDown) && lastSelRow == rowCount - 1)) {
        // In case of End, or non-continuous selection and lastSelRow already
        // at the end, we simply paste at the end by invalidating the index.
        destRow = -1;
    } else if (up || pageUp) {
        // currentIndex can be anywhere inside or outside the selection.
        // Set it to first selected row, then pass through the key event
        // to get us the desired destination index.
        setCurrentIndex(model()->index(firstSelRow, currentIndex().column()));
        QTableView::keyPressEvent(event);
        destRow = currentIndex().row();
        if (destRow >= lastSelRow) {
            // Multiple Up presses caused moveCursor() to wrap around.
            // Paste at the top.
            destRow = 0;
        }
    } else if (down || pageDown) {
        // Same when moving down, set index to last selected row.
        setCurrentIndex(model()->index(lastSelRow, currentIndex().column()));
        QTableView::keyPressEvent(event);
        destRow = currentIndex().row() + 1;
        if ((pageDown && destRow >= rowCount) || destRow <= firstSelRow) {
            // PageDown hit the end of the list, or moveCursor() wrapped around.
            // Explicitly paste at the end.
            destRow = -1;
        }
    }

    moveRows(selectedRows, destRow);
}

void WTrackTableView::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
    case kPropertiesShortcutKey: {
        // Return invokes the double-click action.
        // Ctrl+Return opens the track properties dialog.
        // Ignore it if any cell editor is open.
        // Note: we use kPropertiesShortcutKey/~Mofifier here and in
        // in WTrackMenu to display the shortcut.
        if (state() == QTableView::EditingState) {
            break;
        }
        if (event->modifiers().testFlag(Qt::NoModifier)) {
            slotMouseDoubleClicked(currentIndex());
        } else if ((event->modifiers() & kPropertiesShortcutModifier)) {
            TrackModel* pTrackModel = getTrackModel();
            if (!pTrackModel ||
                    !pTrackModel->hasCapabilities(
                            TrackModel::Capability::EditMetadata)) {
                return;
            }
            const QModelIndexList indices = getSelectedRows();
            if (indices.isEmpty()) {
                return;
            }
            m_pTrackMenu->loadTrackModelIndices(indices);
            // Pass the name of the focused column to DlgTrackInfo/~Multi
            // so they can focus the respective edit field.
            // We use the column of the current index (last focus cell), even
            // it may not be part of the selection, we just assume it's in the
            // desired column.
            const QString columnName = columnNameOfIndex(currentIndex());
            m_pTrackMenu->setTrackPropertyName(columnName);
            m_pTrackMenu->slotShowDlgTrackInfo();
        }
        return;
    }
    case kHideRemoveShortcutKey: {
        if (event->modifiers() == kHideRemoveShortcutModifier) {
            hideOrRemoveSelectedTracks();
        }
        return;
    }
    default:
        break;
    }
    TrackModel* pTrackModel = getTrackModel();
    if (pTrackModel) {
        if (!pTrackModel->isLocked()) {
            if (event->matches(QKeySequence::Delete) || event->key() == Qt::Key_Backspace) {
                removeSelectedTracks();
                return;
            }
            if (event->matches(QKeySequence::Cut)) {
                cutSelectedTracks();
                return;
            }
            if (event->matches(QKeySequence::Paste)) {
                pasteTracks(currentIndex());
                return;
            }
            if (event->key() == Qt::Key_Escape) {
                clearSelection();
                setCurrentIndex(QModelIndex());
            }

            if (event->modifiers().testFlag(Qt::AltModifier) &&
                    (event->key() == Qt::Key_Up ||
                            event->key() == Qt::Key_Down ||
                            event->key() == Qt::Key_PageUp ||
                            event->key() == Qt::Key_PageDown ||
                            event->key() == Qt::Key_Home ||
                            event->key() == Qt::Key_End) &&
                    pTrackModel->hasCapabilities(TrackModel::Capability::Reorder)) {
                moveSelectedTracks(event);
                return;
            }
        }

        if (event->matches(QKeySequence::Copy)) {
            copySelectedTracks();
            return;
        }

        if (event->modifiers().testFlag(Qt::ControlModifier) &&
                event->modifiers().testFlag(Qt::ShiftModifier) &&
                event->key() == Qt::Key_C) {
            // copy the cell content as native QKeySequence::Copy would
            QKeyEvent ke =
                    QKeyEvent{QEvent::KeyPress, Qt::Key_C, Qt::ControlModifier};
            QTableView::keyPressEvent(&ke);
            return;
        }
    }
    QTableView::keyPressEvent(event);
}

void WTrackTableView::resizeEvent(QResizeEvent* event) {
    // When the tracks view shrinks in height, e.g. when other skin regions expand,
    // and if the row was visible before resizing, scroll to it afterwards.

    // these heights are the actual inner region without header, scrollbars and padding
    int oldHeight = event->oldSize().height();
    int newHeight = event->size().height();

    if (newHeight >= oldHeight) {
        QTableView::resizeEvent(event);
        return;
    }

    QModelIndex currIndex = currentIndex();
    int currRow = currIndex.row();
    int rHeight = rowHeight(currRow);

    if (currRow < 0 || rHeight == 0) { // true if currIndex is invalid
        QTableView::resizeEvent(event);
        return;
    }

    // y-pos of the top edge, negative value means above viewport boundary
    int posInView = rowViewportPosition(currRow);
    // Check if the row is visible.
    // Note: don't use viewport()->height() because that may already have changed
    bool rowWasVisible = posInView > 0 && posInView - rHeight < oldHeight;

    QTableView::resizeEvent(event);

    if (!rowWasVisible) {
        return;
    }

    // Check if the item is fully visible. If not, scroll to show it
    posInView = rowViewportPosition(currRow);
    if (posInView - rHeight < 0 || posInView + rHeight > newHeight) {
        scrollTo(currIndex);
    }
}

void WTrackTableView::hideOrRemoveSelectedTracks() {
    TrackModel* pTrackModel = getTrackModel();
    if (!pTrackModel) {
        return;
    }
    const QModelIndexList indices = getSelectedRows();
    if (indices.isEmpty()) {
        return;
    }

    TrackModel::Capability cap;
    // Remove is the primary action if allowed (playlists and crates).
    // Else we test for remove capability.
    // In the track menu the hotkey is shown for 'Remove ..' actions, or if there
    // is no remove action, for 'Hide ..'. Hence, to match the hotkey, do Hide
    // only if the track model doesn't support any Remove actions.
    if (pTrackModel->hasCapabilities(TrackModel::Capability::Remove)) {
        cap = TrackModel::Capability::Remove;
    } else if (pTrackModel->hasCapabilities(TrackModel::Capability::RemoveCrate)) {
        cap = TrackModel::Capability::RemoveCrate;
    } else if (pTrackModel->hasCapabilities(TrackModel::Capability::RemovePlaylist)) {
        cap = TrackModel::Capability::RemovePlaylist;
    } else if (pTrackModel->hasCapabilities(TrackModel::Capability::Hide)) {
        cap = TrackModel::Capability::Hide;
    } else { // Locked playlists and crates
        return;
    }

    switch (cap) {
    case TrackModel::Capability::Remove:
    case TrackModel::Capability::RemoveCrate:
    case TrackModel::Capability::RemovePlaylist: {
        if (pTrackModel->isLocked()) {
            return;
        }
    default:
        break;
    }
    }

    if (pTrackModel->getRequireConfirmationToHideRemoveTracks()) {
        QString title;
        QString message;
        if (cap == TrackModel::Capability::Hide) {
            // Hide tracks if this is the main library table
            title = tr("Confirm track hide");
            message = tr("Are you sure you want to hide the selected tracks?");
        } else {
            title = tr("Confirm track removal");
            // Else remove the tracks from AutoDJ/crate/playlist
            if (cap == TrackModel::Capability::Remove) {
                message =
                        tr("Are you sure you want to remove the selected "
                           "tracks from AutoDJ queue?");
            } else if (cap == TrackModel::Capability::RemoveCrate) {
                message =
                        tr("Are you sure you want to remove the selected "
                           "tracks from this crate?");
            } else { // TrackModel::Capability::RemovePlaylist
                message =
                        tr("Are you sure you want to remove the selected "
                           "tracks from this playlist?");
            }
        }

        QMessageBox msg;
        msg.setIcon(QMessageBox::Question);
        msg.setWindowTitle(title);
        msg.setText(message);
        QCheckBox notAgainCB(tr("Don't ask again during this session"));
        notAgainCB.setCheckState(Qt::Unchecked);
        msg.setCheckBox(&notAgainCB);
        msg.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msg.setDefaultButton(QMessageBox::Cancel);
        if (msg.exec() != QMessageBox::Ok) {
            return;
        }

        if (notAgainCB.isChecked()) {
            pTrackModel->setRequireConfirmationToHideRemoveTracks(false);
        }
    }

    saveCurrentIndex();

    if (cap == TrackModel::Capability::Hide) {
        pTrackModel->hideTracks(indices);
    } else {
        pTrackModel->removeTracks(indices);
    }

    restoreCurrentIndex();
}

/// If applicable, requests that the selected field/item be edited
/// Does nothing otherwise.
void WTrackTableView::editSelectedItem() {
    if (state() != EditingState) {
        edit(currentIndex(), EditKeyPressed, nullptr);
    }
}

void WTrackTableView::activateSelectedTrack() {
    const QModelIndexList indices = getSelectedRows();
    if (indices.isEmpty()) {
        return;
    }
    slotMouseDoubleClicked(indices.at(0));
}

#ifdef __STEM__
void WTrackTableView::loadSelectedTrackToGroup(const QString& group,
        mixxx::StemChannelSelection stemMask,
        bool play) {
#else
void WTrackTableView::loadSelectedTrackToGroup(const QString& group,
        bool play) {
#endif
    const QModelIndexList indices = getSelectedRows();
    if (indices.isEmpty()) {
        return;
    }
    bool allowLoadTrackIntoPlayingDeck = false;
    if (m_pConfig->exists(kConfigKeyLoadWhenDeckPlaying)) {
        int loadWhenDeckPlaying =
                m_pConfig->getValueString(kConfigKeyLoadWhenDeckPlaying).toInt();
        switch (static_cast<LoadWhenDeckPlaying>(loadWhenDeckPlaying)) {
        case LoadWhenDeckPlaying::Allow:
        case LoadWhenDeckPlaying::AllowButStopDeck:
            allowLoadTrackIntoPlayingDeck = true;
            break;
        case LoadWhenDeckPlaying::Reject:
            break;
        }
    } else {
        // support older version of this flag
        allowLoadTrackIntoPlayingDeck =
                m_pConfig->getValue<bool>(kConfigKeyAllowTrackLoadToPlayingDeck);
    }
    // If the track load override is disabled, check to see if a track is
    // playing before trying to load it.
    // Always load to preview deck.
    if (!allowLoadTrackIntoPlayingDeck &&
            !PlayerManager::isPreviewDeckGroup(group) &&
            ControlObject::get(ConfigKey(group, "play")) > 0.0) {
        return;
    }
    auto index = indices.at(0);
    auto* pTrackModel = getTrackModel();
    TrackPointer pTrack;
    if (pTrackModel && (pTrack = pTrackModel->getTrack(index))) {
#ifdef __STEM__
        DEBUG_ASSERT(!stemMask || pTrack->hasStem());
        emit loadTrackToPlayer(pTrack, group, stemMask, play);
#else
        emit loadTrackToPlayer(pTrack, group, play);
#endif
    }
}

QList<TrackId> WTrackTableView::getSelectedTrackIds() const {
    TrackModel* pTrackModel = getTrackModel();
    VERIFY_OR_DEBUG_ASSERT(pTrackModel != nullptr) {
        qWarning() << "No track model available";
        return {};
    }

    const QModelIndexList rows = getSelectedRows();
    QList<TrackId> trackIds;
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

TrackId WTrackTableView::getCurrentTrackId() const {
    TrackModel* pTrackModel = getTrackModel();
    VERIFY_OR_DEBUG_ASSERT(pTrackModel != nullptr) {
        qWarning() << "No track model available";
        return {};
    }

    QItemSelectionModel* pSelectionModel = selectionModel();
    VERIFY_OR_DEBUG_ASSERT(pSelectionModel != nullptr) {
        qWarning() << "No selection model available";
        return {};
    }

    const QModelIndex current = pSelectionModel->currentIndex();
    if (current.isValid()) {
        return pTrackModel->getTrackId(current);
    }
    return {};
}

bool WTrackTableView::isTrackInCurrentView(const TrackId& trackId) {
    VERIFY_OR_DEBUG_ASSERT(trackId.isValid()) {
        return false;
    }
    //qDebug() << "WTrackTableView::isTrackInCurrentView" << trackId;
    TrackModel* pTrackModel = getTrackModel();
    VERIFY_OR_DEBUG_ASSERT(pTrackModel != nullptr) {
        qWarning() << "No track model";
        return false;
    }
    const QVector<int> trackRows = pTrackModel->getTrackRows(trackId);
    //qDebug() << "   track found?" << !trackRows.empty();
    return !trackRows.empty();
}

void WTrackTableView::setSelectedTracks(const QList<TrackId>& trackIds) {
    TrackModel* pTrackModel = getTrackModel();
    VERIFY_OR_DEBUG_ASSERT(pTrackModel != nullptr) {
        qWarning() << "No track model";
        return;
    }

    QItemSelectionModel* pSelectionModel = selectionModel();
    VERIFY_OR_DEBUG_ASSERT(pSelectionModel != nullptr) {
        qWarning() << "No selection model";
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

bool WTrackTableView::setCurrentTrackId(const TrackId& trackId, int column, bool scrollToTrack) {
    if (!trackId.isValid()) {
        return false;
    }

    TrackModel* pTrackModel = getTrackModel();
    VERIFY_OR_DEBUG_ASSERT(pTrackModel != nullptr) {
        qWarning() << "No track model";
        return false;
    }
    QItemSelectionModel* pSelectionModel = selectionModel();
    VERIFY_OR_DEBUG_ASSERT(pSelectionModel != nullptr) {
        qWarning() << "No selection model";
        return false;
    }

    const QVector<int> trackRows = pTrackModel->getTrackRows(trackId);
    if (trackRows.empty()) {
        qDebug() << "WTrackTableView: track" << trackId << "is not in current view";
        return false;
    }

    QModelIndex idx = model()->index(trackRows[0], column);
    // In case the column is not visible pick the left-most one
    if (isIndexHidden(idx)) {
        idx = model()->index(idx.row(), columnAt(0));
    }
    selectRow(idx.row());
    pSelectionModel->setCurrentIndex(idx,
            QItemSelectionModel::SelectCurrent | QItemSelectionModel::Select);

    if (scrollToTrack) {
        scrollTo(idx);
    }

    return true;
}

void WTrackTableView::addToAutoDJ(PlaylistDAO::AutoDJSendLoc loc) {
    auto* pTrackModel = getTrackModel();
    if (!pTrackModel || !pTrackModel->hasCapabilities(TrackModel::Capability::AddToAutoDJ)) {
        return;
    }

    const QList<TrackId> trackIds = getSelectedTrackIds();
    if (trackIds.isEmpty()) {
        qWarning() << "No tracks selected for AutoDJ";
        return;
    }

    PlaylistDAO& playlistDao = m_pLibrary->trackCollectionManager()
                                       ->internalCollection()
                                       ->getPlaylistDAO();

    // TODO(XXX): Care whether the append succeeded.
    m_pLibrary->trackCollectionManager()->unhideTracks(trackIds);
    playlistDao.addTracksToAutoDJQueue(trackIds, loc);
}

void WTrackTableView::addToAutoDJBottom() {
    addToAutoDJ(PlaylistDAO::AutoDJSendLoc::BOTTOM);
}

void WTrackTableView::addToAutoDJTop() {
    addToAutoDJ(PlaylistDAO::AutoDJSendLoc::TOP);
}

void WTrackTableView::addToAutoDJReplace() {
    addToAutoDJ(PlaylistDAO::AutoDJSendLoc::REPLACE);
}

void WTrackTableView::selectTrack(const TrackId& trackId) {
    if (trackId.isValid() && setCurrentTrackId(trackId, 0, true)) {
        setSelectedTracks({trackId});
    } else {
        setSelectedTracks({});
    }
}

void WTrackTableView::moveSelection(int delta) {
    QAbstractItemModel* pModel = model();

    if (pModel == nullptr) {
        return;
    }

    while (delta != 0) {
        QItemSelectionModel* currentSelection = selectionModel();
        if (currentSelection->selectedRows().length() > 0) {
            if (delta > 0) {
                // i is positive, so we want to move the highlight down
                int row = currentSelection->selectedRows().last().row();
                if (row + 1 < pModel->rowCount()) {
                    selectRow(row + 1);
                } else {
                    // we wrap around at the end of the list so it is faster to get
                    // to the top of the list again
                    selectRow(0);
                }

                delta--;
            } else {
                // i is negative, so move down
                int row = currentSelection->selectedRows().first().row();
                if (row - 1 >= 0) {
                    selectRow(row - 1);
                } else {
                    selectRow(pModel->rowCount() - 1);
                }

                delta++;
            }
        } else {
            // no selection, so select the first or last element depending on delta
            if (delta > 0) {
                selectRow(0);
                delta--;
            } else {
                selectRow(pModel->rowCount() - 1);
                delta++;
            }
        }
    }
}

void WTrackTableView::doSortByColumn(int headerSection, Qt::SortOrder sortOrder) {
    TrackModel* pTrackModel = getTrackModel();
    QAbstractItemModel* pItemModel = model();
    if (!pTrackModel || !pItemModel || !m_sorting) {
        return;
    }

    // Save the selection
    // If this is a track model that may contain a track multiple times (a playlist),
    // we store the positions in order to reselect only the current selection,
    // not all occurrences of selected tracks.
    QList<TrackId> selectedTrackIds;
    QList<int> selectedTrackPositions;
    bool usePositions = pTrackModel->hasCapabilities(TrackModel::Capability::Reorder);
    if (usePositions) {
        const QModelIndexList indices = selectionModel()->selectedRows();
        selectedTrackPositions = pTrackModel->getSelectedPositions(indices);
    } else {
        selectedTrackIds = getSelectedTrackIds();
    }

    int savedHScrollBarPos = horizontalScrollBar()->value();
    // Save the column of focused table cell.
    // The cell is not necessarily part of the selection, but even if it's
    // focused after deselecting a row we may assume the user clicked onto the
    // column that will be used for sorting.
    int prevColumn = 0;
    if (currentIndex().isValid()) {
        prevColumn = currentIndex().column();
    }

    sortByColumn(headerSection, sortOrder);

    if (usePositions) {
        selectTracksByPosition(selectedTrackPositions, prevColumn);
    } else {
        selectTracksById(selectedTrackIds, prevColumn);
    }

    // This seems to be broken since at least Qt 5.12: no scrolling is issued
    // scrollTo(first, QAbstractItemView::EnsureVisible);
    horizontalScrollBar()->setValue(savedHScrollBarPos);
}

void WTrackTableView::selectTracksByPosition(const QList<int>& positions, int prevColumn) {
    if (positions.isEmpty()) {
        return;
    }
    TrackModel* pTrackModel = getTrackModel();
    QItemSelectionModel* pSelectionModel = selectionModel();
    pSelectionModel->reset(); // remove current selection

    // Find previously selected tracks and store respective rows for reselection.
    QList<int> rows;
    for (int pos : positions) {
        rows.append(pTrackModel->getTrackRowByPosition(pos));
    }

    // Select the first row of the previous selection.
    // This scrolls to that row and with the leftmost cell being focused we have
    // a starting point (currentIndex) for navigation with Up/Down keys.
    // Replaces broken scrollTo() (see comment below)
    if (!rows.isEmpty()) {
        selectRow(rows.first());
    }

    // Refocus the cell in the column that was focused before sorting.
    // With this, any Up/Down key press moves the selection and keeps the
    // horizontal scrollbar position we will restore below.
    QModelIndex restoreIndex = model()->index(currentIndex().row(), prevColumn);
    if (restoreIndex.isValid()) {
        setCurrentIndex(restoreIndex);
    }

    // Restore previous selection (doesn't affect focused cell).
    for (int row : rows) {
        pSelectionModel->select(model()->index(row, prevColumn),
                QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
}

// Don't use this on playlists since they may contain a TrackId multiple times.
// See doSortByColumn.
void WTrackTableView::selectTracksById(const QList<TrackId>& trackIds, int prevColum) {
    TrackModel* pTrackModel = getTrackModel();
    QAbstractItemModel* pItemModel = model();

    QItemSelectionModel* pSelectionModel = selectionModel();
    VERIFY_OR_DEBUG_ASSERT(pSelectionModel != nullptr) {
        qWarning() << "No selection model available";
        return;
    }
    pSelectionModel->reset(); // remove current selection

    // Find previously selected tracks and store respective rows for reselection.
    QMap<int, int> selectedRows;
    for (const auto& trackId : trackIds) {
        const auto rows = pTrackModel->getTrackRows(trackId);
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
    QModelIndex restoreIndex = pItemModel->index(currentIndex().row(), prevColum);
    if (restoreIndex.isValid()) {
        setCurrentIndex(restoreIndex);
    }

    // Restore previous selection (doesn't affect focused cell).
    QMapIterator<int, int> i(selectedRows);
    while (i.hasNext()) {
        i.next();
        QModelIndex tl = pItemModel->index(i.key(), 0);
        pSelectionModel->select(tl, QItemSelectionModel::Rows | QItemSelectionModel::Select);
    }
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
    TrackModel* pTrackModel = getTrackModel();
    if (!pTrackModel) {
        return;
    }
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

    int sortColumn = pTrackModel->columnIndexFromSortColumnId(
            static_cast<TrackModel::SortColumnId>(sortColumnId));
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
    TrackModel* pTrackModel = getTrackModel();
    if (!pTrackModel) {
        return;
    }

    TrackModel::SortColumnId sortColumnId = pTrackModel->sortColumnIdFromColumnIndex(headerSection);
    if (sortColumnId == TrackModel::SortColumnId::Invalid) {
        return;
    }

    double sortOrder = static_cast<double>(order);
    bool sortingChanged = false;

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

void WTrackTableView::slotRandomSorting() {
    // There's no need to remove the shuffle feature of the Preview column
    // (and replace it with a dedicated randomize slot to BaseSqltableModel),
    // so we simply abuse that column.
    auto previewCol = TrackModel::SortColumnId::Preview;
    m_pSortColumn->set(static_cast<int>(previewCol));
    applySortingIfVisible();
}

bool WTrackTableView::hasFocus() const {
    return QWidget::hasFocus();
}

void WTrackTableView::setFocus() {
    QWidget::setFocus(Qt::OtherFocusReason);
}

QString WTrackTableView::getModelStateKey() const {
    TrackModel* pTrackModel = getTrackModel();
    if (!pTrackModel) {
        return {};
    }
    bool noSearch = pTrackModel->currentSearch().trimmed().isEmpty();
    return pTrackModel->modelKey(noSearch);
}

void WTrackTableView::keyNotationChanged() {
    QWidget::update();
}
