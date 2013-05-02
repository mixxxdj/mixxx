#include <QItemDelegate>
#include <QtCore>
#include <QtGui>
#include <QtXml>
#include <QModelIndex>

#include "widget/wwidget.h"
#include "widget/wskincolor.h"
#include "widget/wtracktableviewheader.h"
#include "library/librarytablemodel.h"
#include "library/trackcollection.h"
#include "trackinfoobject.h"
#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "widget/wtracktableview.h"
#include "dlgtrackinfo.h"
#include "soundsourceproxy.h"
#include "playermanager.h"

WTrackTableView::WTrackTableView(QWidget * parent,
                                 ConfigObject<ConfigValue> * pConfig,
                                 TrackCollection* pTrackCollection, bool sorting)
        : WLibraryTableView(parent, pConfig,
                            ConfigKey(LIBRARY_CONFIGVALUE,
                                      WTRACKTABLEVIEW_VSCROLLBARPOS_KEY)),
          m_pConfig(pConfig),
          m_pTrackCollection(pTrackCollection),
          m_searchThread(this),
          m_sorting(sorting) {
    // Give a NULL parent because otherwise it inherits our style which can make
    // it unreadable. Bug #673411
    m_pTrackInfo = new DlgTrackInfo(NULL);
    connect(m_pTrackInfo, SIGNAL(next()),
            this, SLOT(slotNextTrackInfo()));
    connect(m_pTrackInfo, SIGNAL(previous()),
            this, SLOT(slotPrevTrackInfo()));

    connect(&m_loadTrackMapper, SIGNAL(mapped(QString)),
            this, SLOT(loadSelectionToGroup(QString)));

    connect(&m_deckMapper, SIGNAL(mapped(QString)),
            this, SLOT(loadSelectionToGroup(QString)));
    connect(&m_samplerMapper, SIGNAL(mapped(QString)),
            this, SLOT(loadSelectionToGroup(QString)));

    m_pNumSamplers = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey("[Master]", "num_samplers")));
    m_pNumDecks = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey("[Master]", "num_decks")));
    m_pNumPreviewDecks = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey("[Master]", "num_preview_decks")));

    m_pMenu = new QMenu(this);

    m_pSamplerMenu = new QMenu(this);
    m_pSamplerMenu->setTitle(tr("Load to Sampler"));
    m_pPlaylistMenu = new QMenu(this);
    m_pPlaylistMenu->setTitle(tr("Add to Playlist"));
    m_pCrateMenu = new QMenu(this);
    m_pCrateMenu->setTitle(tr("Add to Crate"));

    // Disable editing
    //setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Create all the context m_pMenu->actions (stuff that shows up when you
    //right-click)
    createActions();

    //Connect slots and signals to make the world go 'round.
    connect(this, SIGNAL(doubleClicked(const QModelIndex &)),
            this, SLOT(slotMouseDoubleClicked(const QModelIndex &)));

    connect(&m_playlistMapper, SIGNAL(mapped(int)),
            this, SLOT(addSelectionToPlaylist(int)));
    connect(&m_crateMapper, SIGNAL(mapped(int)),
            this, SLOT(addSelectionToCrate(int)));
}

WTrackTableView::~WTrackTableView()
{
    WTrackTableViewHeader* pHeader =
            dynamic_cast<WTrackTableViewHeader*>(horizontalHeader());
    if (pHeader) {
        pHeader->saveHeaderState();
    }

    delete m_pReloadMetadataAct;
    delete m_pAddToPreviewDeck;
    delete m_pAutoDJAct;
    delete m_pAutoDJTopAct;
    delete m_pRemoveAct;
    delete m_pHideAct;
    delete m_pUnhideAct;
    delete m_pPropertiesAct;
    delete m_pMenu;
    delete m_pPlaylistMenu;
    delete m_pCrateMenu;
    //delete m_pRenamePlaylistAct;
    delete m_pTrackInfo;
    delete m_pNumSamplers;
    delete m_pNumDecks;
    delete m_pNumPreviewDecks;
    delete m_pBpmLockAction;
    delete m_pBpmUnlockAction;
    delete m_pPurgeAct;
    delete m_pFileBrowserAct;
    delete m_pResetPlayedAct;
    delete m_pSamplerMenu;
}

// slot
void WTrackTableView::loadTrackModel(QAbstractItemModel *model) {
    //qDebug() << "WTrackTableView::loadTrackModel()" << model;

    TrackModel* track_model = dynamic_cast<TrackModel*>(model);

    Q_ASSERT(model);
    Q_ASSERT(track_model);

    /* If the model has not changed
     * there's no need to exchange the headers
     * this will cause a small GUI freeze
     */
    if (getTrackModel() == track_model) {
        // Re-sort the table even if the track model is the same. This triggers
        // a select() if the table is dirty.
        doSortByColumn(horizontalHeader()->sortIndicatorSection());
        return;
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
    WTrackTableViewHeader* header = new WTrackTableViewHeader(Qt::Horizontal, this);

    // WTF(rryan) The following saves on unnecessary work on the part of
    // WTrackTableHeaderView. setHorizontalHeader() calls setModel() on the
    // current horizontal header. If this happens on the old
    // WTrackTableViewHeader, then it will save its old state, AND do the work
    // of initializing its menus on the new model. We create a new
    // WTrackTableViewHeader, so this is wasteful. Setting a temporary
    // QHeaderView here saves on setModel() calls. Since we parent the
    // QHeaderView to the WTrackTableView, it is automatically deleted.
    QHeaderView* tempHeader = new QHeaderView(Qt::Horizontal, this);
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
    header->setMovable(true);
    header->setClickable(true);
    header->setHighlightSections(true);
    header->setSortIndicatorShown(m_sorting);

    // Initialize all column-specific things
    for (int i = 0; i < model->columnCount(); ++i) {
        // Setup delegates according to what the model tells us
        QAbstractItemDelegate* delegate = track_model->delegateForColumn(i, this);
        // We need to delete the old delegates, since the docs say the view will
        // not take ownership of them.
        QAbstractItemDelegate* old_delegate = itemDelegateForColumn(i);
        // If delegate is NULL, it will unset the delegate for the column
        setItemDelegateForColumn(i, delegate);
        delete old_delegate;

        // Show or hide the column based on whether it should be shown or not.
        if (track_model->isColumnInternal(i)) {
            //qDebug() << "Hiding column" << i;
            horizontalHeader()->hideSection(i);
        }
        /* If Mixxx starts the first time or the header states have been cleared
         * due to database schema evolution we gonna hide all columns that may
         * contain a potential large number of NULL values.  This will hide the
         * key colum by default unless the user brings it to front
         */
        if (track_model->isColumnHiddenByDefault(i) &&
            !header->hasPersistedHeaderState()) {
            //qDebug() << "Hiding column" << i;
            horizontalHeader()->hideSection(i);
        }
    }

    if (m_sorting) {
        // NOTE: Should be a UniqueConnection but that requires Qt 4.6
        connect(horizontalHeader(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)),
                this, SLOT(doSortByColumn(int)), Qt::AutoConnection);

        // Stupid hack that assumes column 0 is never visible, but this is a weak
        // proxy for "there was a saved column sort order"
        if (horizontalHeader()->sortIndicatorSection() > 0) {
            // Sort by the saved sort section and order. This line sorts the
            // TrackModel and in turn generates a select()
            horizontalHeader()->setSortIndicator(horizontalHeader()->sortIndicatorSection(),
                                                 horizontalHeader()->sortIndicatorOrder());
        } else {
            // No saved order is present. Use the TrackModel's default sort order.
            int sortColumn = track_model->defaultSortColumn();
            Qt::SortOrder sortOrder = track_model->defaultSortOrder();

            // If the TrackModel has an invalid or internal column as its default
            // sort, find the first non-internal column and sort by that.
            while (sortColumn < 0 || track_model->isColumnInternal(sortColumn)) {
                sortColumn++;
            }
            // This line sorts the TrackModel and in turn generates a select()
            horizontalHeader()->setSortIndicator(sortColumn, sortOrder);
        }
    }

    // Set up drag and drop behaviour according to whether or not the track
    // model says it supports it.

    // Defaults
    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::DragOnly);
    // Always enable drag for now (until we have a model that doesn't support
    // this.)
    setDragEnabled(true);

    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_RECEIVEDROPS)) {
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
}

void WTrackTableView::createActions() {
    Q_ASSERT(m_pMenu);
    Q_ASSERT(m_pSamplerMenu);

    m_pRemoveAct = new QAction(tr("Remove"), this);
    connect(m_pRemoveAct, SIGNAL(triggered()), this, SLOT(slotRemove()));

    m_pHideAct = new QAction(tr("Hide from Library"), this);
    connect(m_pHideAct, SIGNAL(triggered()), this, SLOT(slotHide()));

    m_pUnhideAct = new QAction(tr("Unhide from Library"), this);
    connect(m_pUnhideAct, SIGNAL(triggered()), this, SLOT(slotUnhide()));

    m_pPurgeAct = new QAction(tr("Purge from Library"), this);
    connect(m_pPurgeAct, SIGNAL(triggered()), this, SLOT(slotPurge()));

    m_pPropertiesAct = new QAction(tr("Properties"), this);
    connect(m_pPropertiesAct, SIGNAL(triggered()),
            this, SLOT(slotShowTrackInfo()));

    m_pFileBrowserAct = new QAction(tr("Open in File Browser"), this);
    connect(m_pFileBrowserAct, SIGNAL(triggered()),
            this, SLOT(slotOpenInFileBrowser()));

    m_pAutoDJAct = new QAction(tr("Add to Auto-DJ Queue (bottom)"), this);
    connect(m_pAutoDJAct, SIGNAL(triggered()), this, SLOT(slotSendToAutoDJ()));

    m_pAutoDJTopAct = new QAction(tr("Add to Auto-DJ Queue (top)"), this);
    connect(m_pAutoDJTopAct, SIGNAL(triggered()),
            this, SLOT(slotSendToAutoDJTop()));

    m_pReloadMetadataAct = new QAction(tr("Reload Track Metadata"), this);
    connect(m_pReloadMetadataAct, SIGNAL(triggered()),
            this, SLOT(slotReloadTrackMetadata()));

    m_pAddToPreviewDeck = new QAction(tr("Load to Preview Deck"), this);
    // currently there is only one preview deck so just map it here.
    QString previewDeckGroup = PlayerManager::groupForPreviewDeck(0);
    m_deckMapper.setMapping(m_pAddToPreviewDeck, previewDeckGroup);
    connect(m_pAddToPreviewDeck, SIGNAL(triggered()),
            &m_deckMapper, SLOT(map()));

    m_pResetPlayedAct = new QAction(tr("Reset Play Count"), this);
    connect(m_pResetPlayedAct, SIGNAL(triggered()),
            this, SLOT(slotResetPlayed()));

    m_pBpmLockAction = new QAction(tr("Lock BPM"), this);
    m_pBpmUnlockAction = new QAction(tr("Unlock BPM"), this);
    connect(m_pBpmLockAction, SIGNAL(triggered()),
            this, SLOT(slotLockBpm()));
    connect(m_pBpmUnlockAction, SIGNAL(triggered()),
            this, SLOT(slotUnlockBpm()));

    m_pClearBeatsAction = new QAction(tr("Clear BPM and Beatgrid"), this);
    connect(m_pClearBeatsAction, SIGNAL(triggered()),
            this, SLOT(slotClearBeats()));
}

// slot
void WTrackTableView::slotMouseDoubleClicked(const QModelIndex &index) {
    if (!modelHasCapabilities(TrackModel::TRACKMODELCAPS_LOADTODECK)) {
        return;
    }

    TrackModel* trackModel = getTrackModel();
    TrackPointer pTrack;
    if (trackModel && (pTrack = trackModel->getTrack(index))) {
        emit(loadTrack(pTrack));
    }
}

void WTrackTableView::loadSelectionToGroup(QString group, bool play) {
    QModelIndexList indices = selectionModel()->selectedRows();
    if (indices.size() > 0) {
        // If the track load override is disabled, check to see if a track is
        // playing before trying to load it
        if (!(m_pConfig->getValueString(
            ConfigKey("[Controls]","AllowTrackLoadToPlayingDeck")).toInt())) {
            ControlObject* pPlayCO = ControlObject::getControl(ConfigKey(group, "play"));
            // TODO(XXX): Check for other than just the first preview deck.
            if (group != "[PreviewDeck1]" && pPlayCO && pPlayCO->get() > 0.0f) {
                return;
            }
        }

        QModelIndex index = indices.at(0);
        TrackModel* trackModel = getTrackModel();
        TrackPointer pTrack;
        if (trackModel &&
                (pTrack = trackModel->getTrack(index))) {
            emit(loadTrackToPlayer(pTrack, group, play));
        }
    }
}

void WTrackTableView::slotRemove()
{
    QModelIndexList indices = selectionModel()->selectedRows();
    if (indices.size() > 0)
    {
        TrackModel* trackModel = getTrackModel();
        if (trackModel) {
            trackModel->removeTracks(indices);
        }
    }
}

void WTrackTableView::slotPurge(){
    QModelIndexList indices = selectionModel()->selectedRows();
    if (indices.size() > 0)
    {
        TrackModel* trackModel = getTrackModel();
        if (trackModel) {
            trackModel->purgeTracks(indices);
        }
    }
}

void WTrackTableView::slotOpenInFileBrowser() {
    TrackModel* trackModel = getTrackModel();
    if (!trackModel)
        return;

    QModelIndexList indices = selectionModel()->selectedRows();

    QSet<QString> dirs;
    foreach (QModelIndex index, indices) {
        if (!index.isValid())
            continue;

        QFileInfo file(trackModel->getTrackLocation(index));

        QDir directory = file.dir();
        if (!directory.exists()) {
            directory = QDir::home();
        }
        if (dirs.contains(directory.absolutePath())) {
            continue;
        }
        dirs.insert(directory.absolutePath());
        QDesktopServices::openUrl(QUrl::fromLocalFile(directory.absolutePath()));
    }
}

void WTrackTableView::slotHide()
{
    QModelIndexList indices = selectionModel()->selectedRows();
    if (indices.size() > 0)
    {
        TrackModel* trackModel = getTrackModel();
        if (trackModel) {
            trackModel->hideTracks(indices);
        }
    }
}

void WTrackTableView::slotUnhide()
{
    QModelIndexList indices = selectionModel()->selectedRows();
    if (indices.size() > 0)
    {
        TrackModel* trackModel = getTrackModel();
        if (trackModel) {
            trackModel->unhideTracks(indices);
        }
    }
}

void WTrackTableView::slotShowTrackInfo() {
    QModelIndexList indices = selectionModel()->selectedRows();

    if (indices.size() > 0) {
        showTrackInfo(indices[0]);
    }
}

void WTrackTableView::slotNextTrackInfo() {
    QModelIndex nextRow = currentTrackInfoIndex.sibling(
        currentTrackInfoIndex.row()+1, currentTrackInfoIndex.column());
    if (nextRow.isValid())
        showTrackInfo(nextRow);
}

void WTrackTableView::slotPrevTrackInfo() {
    QModelIndex prevRow = currentTrackInfoIndex.sibling(
        currentTrackInfoIndex.row()-1, currentTrackInfoIndex.column());
    if (prevRow.isValid())
        showTrackInfo(prevRow);
}

void WTrackTableView::showTrackInfo(QModelIndex index) {
    TrackModel* trackModel = getTrackModel();

    if (!trackModel)
        return;

    TrackPointer pTrack = trackModel->getTrack(index);
    // NULL is fine.
    m_pTrackInfo->loadTrack(pTrack);
    currentTrackInfoIndex = index;
    m_pTrackInfo->show();
}

void WTrackTableView::contextMenuEvent(QContextMenuEvent* event) {
    QModelIndexList indices = selectionModel()->selectedRows();

    // Gray out some stuff if multiple songs were selected.
    bool oneSongSelected = indices.size() == 1;

    m_pMenu->clear();

    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_ADDTOAUTODJ)) {
        m_pMenu->addAction(m_pAutoDJAct);
        m_pMenu->addAction(m_pAutoDJTopAct);
        m_pMenu->addSeparator();
    }

    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_LOADTODECK)) {
        int iNumDecks = m_pNumDecks->get();
        if (iNumDecks > 0) {
            for (int i = 1; i <= iNumDecks; ++i) {
                QString deckGroup = QString("[Channel%1]").arg(i);
                ControlObject* pPlayCO = ControlObject::getControl(
                    ConfigKey(deckGroup, "play"));
                bool deckPlaying = pPlayCO && pPlayCO->get() > 0.0;
                bool loadTrackIntoPlayingDeck = m_pConfig->getValueString(
                    ConfigKey("[Controls]","AllowTrackLoadToPlayingDeck")).toInt();
                bool deckEnabled = (!deckPlaying  || loadTrackIntoPlayingDeck)  && oneSongSelected;
                QAction* pAction = new QAction(tr("Load to Deck %1").arg(i), m_pMenu);
                pAction->setEnabled(deckEnabled);
                m_pMenu->addAction(pAction);
                m_deckMapper.setMapping(pAction, deckGroup);
                connect(pAction, SIGNAL(triggered()), &m_deckMapper, SLOT(map()));
            }
        }
    }

    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_LOADTOSAMPLER)) {
        int iNumSamplers = m_pNumSamplers->get();
        if (iNumSamplers > 0) {
            m_pSamplerMenu->clear();
            for (int i = 1; i <= iNumSamplers; ++i) {
                QString samplerGroup = QString("[Sampler%1]").arg(i);
                ControlObject* pPlayCO = ControlObject::getControl(
                    ConfigKey(samplerGroup, "play"));
                bool samplerPlaying = pPlayCO && pPlayCO->get() > 0.0;
                bool samplerEnabled = !samplerPlaying && oneSongSelected;
                QAction* pAction = new QAction(tr("Sampler %1").arg(i), m_pSamplerMenu);
                pAction->setEnabled(samplerEnabled);
                m_pSamplerMenu->addAction(pAction);
                m_samplerMapper.setMapping(pAction, samplerGroup);
                connect(pAction, SIGNAL(triggered()), &m_samplerMapper, SLOT(map()));
            }
            m_pMenu->addMenu(m_pSamplerMenu);
        }
    }

    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_LOADTOPREVIEWDECK) &&
        m_pNumPreviewDecks->get() > 0.0) {
        m_pMenu->addAction(m_pAddToPreviewDeck);
    }

    m_pMenu->addSeparator();

    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_ADDTOPLAYLIST)) {
        m_pPlaylistMenu->clear();
        PlaylistDAO& playlistDao = m_pTrackCollection->getPlaylistDAO();
        QMap<QString,int> playlists;
        int numPlaylists = playlistDao.playlistCount();
        for (int i = 0; i < numPlaylists; ++i) {
            int iPlaylistId = playlistDao.getPlaylistId(i);
            playlists.insert(playlistDao.getPlaylistName(iPlaylistId), iPlaylistId);
        }
        QMapIterator<QString, int> it(playlists);
        while (it.hasNext()) {
            it.next();
            if (!playlistDao.isHidden(it.value())) {
                // No leak because making the menu the parent means they will be
                // auto-deleted
                QAction* pAction = new QAction(it.key(), m_pPlaylistMenu);
                bool locked = playlistDao.isPlaylistLocked(it.value());
                pAction->setEnabled(!locked);
                m_pPlaylistMenu->addAction(pAction);
                m_playlistMapper.setMapping(pAction, it.value());
                connect(pAction, SIGNAL(triggered()), &m_playlistMapper, SLOT(map()));
            }
        }

        m_pMenu->addMenu(m_pPlaylistMenu);
    }

    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_ADDTOCRATE)) {
        m_pCrateMenu->clear();
        CrateDAO& crateDao = m_pTrackCollection->getCrateDAO();
        QMap<QString , int> crates;
        int numCrates = crateDao.crateCount();
        for (int i = 0; i < numCrates; ++i) {
            int iCrateId = crateDao.getCrateId(i);
            crates.insert(crateDao.crateName(iCrateId),iCrateId);
        }
        QMapIterator<QString, int> it(crates);
        while (it.hasNext()) {
            it.next();
            // No leak because making the menu the parent means they will be
            // auto-deleted
            QAction* pAction = new QAction(it.key(), m_pCrateMenu);
            bool locked = crateDao.isCrateLocked(it.value());
            pAction->setEnabled(!locked);
            m_pCrateMenu->addAction(pAction);
            m_crateMapper.setMapping(pAction, it.value());
            connect(pAction, SIGNAL(triggered()), &m_crateMapper, SLOT(map()));
        }

        m_pMenu->addMenu(m_pCrateMenu);
    }

    m_pMenu->addSeparator();

    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_BPMLOCK)) {
        m_pMenu->addAction(m_pBpmLockAction);
        m_pMenu->addAction(m_pBpmUnlockAction);
    }

    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_CLEAR_BEATS)) {
        TrackModel* trackModel = getTrackModel();
        if (trackModel == NULL) {
            return;
        }
        bool allowClear = true;
        int column = trackModel->fieldIndex("bpm_lock");
        for (int i = 0; i < indices.size(); ++i) {
            int row = indices.at(i).row();
            QModelIndex index = indices.at(i).sibling(row,column);
            if (index.data().toBool()) {
                allowClear = false;
            }
        }
        m_pClearBeatsAction->setEnabled(allowClear);
        m_pMenu->addAction(m_pClearBeatsAction);
    }

    bool locked = modelHasCapabilities(TrackModel::TRACKMODELCAPS_LOCKED);
    m_pMenu->addSeparator();
    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_RELOADMETADATA)) {
        m_pMenu->addAction(m_pReloadMetadataAct);
    }
    // REMOVE and HIDE should not be at the first menu position to avoid excitedly clicks
    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_REMOVE)) {
        m_pRemoveAct->setEnabled(!locked);
        m_pMenu->addAction(m_pRemoveAct);
    }
    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_HIDE)) {
        m_pHideAct->setEnabled(!locked);
        m_pMenu->addAction(m_pHideAct);
    }
    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_UNHIDE)) {
        m_pUnhideAct->setEnabled(!locked);
        m_pMenu->addAction(m_pUnhideAct);
    }
    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_PURGE)) {
        m_pPurgeAct->setEnabled(!locked);
        m_pMenu->addAction(m_pPurgeAct);
    }
    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_RESETPLAYED)) {
        m_pMenu->addAction(m_pResetPlayedAct);
    }
    m_pMenu->addAction(m_pFileBrowserAct);
    m_pMenu->addSeparator();
    m_pPropertiesAct->setEnabled(oneSongSelected);
    m_pMenu->addAction(m_pPropertiesAct);

    //Create the right-click menu
    m_pMenu->popup(event->globalPos());
}

void WTrackTableView::onSearch(const QString& text) {
    TrackModel* trackModel = getTrackModel();
    if (trackModel) {
        m_searchThread.enqueueSearch(trackModel, text);
    }
}

void WTrackTableView::onSearchStarting() {
    saveVScrollBarPos();
}

void WTrackTableView::onSearchCleared() {
    restoreVScrollBarPos();
    TrackModel* trackModel = getTrackModel();
    if (trackModel) {
        m_searchThread.enqueueSearch(trackModel, "");
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
    if (!trackModel)
        return;
    // qDebug() << "MouseMoveEvent";
    // Iterate over selected rows and append each item's location url to a list.
    QList<QUrl> locationUrls;
    QModelIndexList indices = selectionModel()->selectedRows();
    foreach (QModelIndex index, indices) {
        if (!index.isValid()) {
            continue;
        }
        QUrl url = QUrl::fromLocalFile(trackModel->getTrackLocation(index));
        if (!url.isValid()) {
            qDebug() << this << "ERROR: invalid url" << url;
            continue;
        }
        locationUrls.append(url);
    }

    if (locationUrls.empty()) {
        return;
    }

    QMimeData* mimeData = new QMimeData();
    mimeData->setUrls(locationUrls);

    QDrag* drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(QPixmap(":images/library/ic_library_drag_and_drop.png"));
    drag->exec(Qt::CopyAction);
}

/** Drag enter event, happens when a dragged item hovers over the track table view*/
void WTrackTableView::dragEnterEvent(QDragEnterEvent * event)
{
    //qDebug() << "dragEnterEvent" << event->mimeData()->formats();
    if (event->mimeData()->hasUrls())
    {
        if (event->source() == this) {
            if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_REORDER)) {
                event->acceptProposedAction();
            } else {
                event->ignore();
            }
        } else {
            QList<QUrl> urls(event->mimeData()->urls());
            bool anyAccepted = false;
            foreach (QUrl url, urls) {
                QFileInfo file(url.toLocalFile());
                if (SoundSourceProxy::isFilenameSupported(file.fileName()))
                    anyAccepted = true;
            }
            if (anyAccepted) {
                event->acceptProposedAction();
            } else {
                event->ignore();
            }
        }
    } else {
        event->ignore();
    }
}

// Drag move event, happens when a dragged item hovers over the track table view...
// Why we need this is a little vague, but without it, drag-and-drop just doesn't work.
// -- Albert June 8/08
void WTrackTableView::dragMoveEvent(QDragMoveEvent * event) {
    // Needed to allow auto-scrolling
    WLibraryTableView::dragMoveEvent(event);

    //qDebug() << "dragMoveEvent" << event->mimeData()->formats();
    if (event->mimeData()->hasUrls())
    {
        if (event->source() == this) {
            if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_REORDER)) {
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
void WTrackTableView::dropEvent(QDropEvent * event){
    if (!event->mimeData()->hasUrls()) {
        event->ignore();
        return;
    }
    QList<QUrl> urls(event->mimeData()->urls());
    QUrl url;
    QModelIndex selectedIndex; //Index of a selected track (iterator)

    // Filter out invalid URLs (eg. files that aren't supported audio filetypes, etc.)
    QRegExp fileRx(SoundSourceProxy::supportedFileExtensionsRegex(),
                    Qt::CaseInsensitive);
    for (int i=0; i<urls.size(); i++) {
        if (fileRx.indexIn(urls.at(i).path()) == -1) {
            // remove invalid urls and decrease i because the size of
            // urls has changed.
            urls.removeAt(i--);
        }
    }

    // Save the vertical scrollbar position. Adding new tracks and moving tracks in
    // the SQL data models causes a select() (ie. generation of a new result set),
    // which causes view to reset itself. A view reset causes the widget to scroll back
    // up to the top, which is confusing when you're dragging and dropping. :)
    saveVScrollBarPos();

    // The model index where the track or tracks are destined to go. :)
    // (the "drop" position in a drag-and-drop)
    QModelIndex destIndex = indexAt(event->pos());

    //qDebug() << "destIndex.row() is" << destIndex.row();

    // Drag and drop within this widget (track reordering)
    if (event->source() == this)
    {
        // For an invalid destination (eg. dropping a track beyond
        // the end of the playlist), place the track(s) at the end
        // of the playlist.
        if (destIndex.row() == -1) {
            int destRow = model()->rowCount() - 1;
            destIndex = model()->index(destRow, 0);
        }
        // Note the above code hides an ambiguous case when a
        // playlist is empty. For that reason, we can't factor that
        // code out to be common for both internal reordering
        // and external drag-and-drop. With internal reordering,
        // you can't have an empty playlist. :)

        //qDebug() << "track reordering" << __FILE__ << __LINE__;

        // Save a list of row (just plain ints) so we don't get screwed over
        // when the QModelIndexes all become invalid (eg. after moveTrack()
        // or addTrack())
        QModelIndexList indices = selectionModel()->selectedRows();

        QList<int> selectedRows;
        QModelIndex idx;
        foreach (idx, indices)
        {
            selectedRows.append(idx.row());
        }

        // Note: The biggest subtlety in the way I've done this track reordering code
        // is that as soon as we've moved ANY track, all of our QModelIndexes probably
        // get screwed up. The starting point for the logic below is to say screw it to
        // the QModelIndexes, and just keep a list of row numbers to work from. That
        // ends up making the logic simpler and the behaviour totally predictable,
        // which lets us do nice things like "restore" the selection model.

        if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_REORDER)) {
            TrackModel* trackModel = getTrackModel();

            // The model indices are sorted so that we remove the tracks from the table
            // in ascending order. This is necessary because if track A is above track B in
            // the table, and you remove track A, the model index for track B will change.
            // Sorting the indices first means we don't have to worry about this.
            //qSort(m_selectedIndices);
            //qSort(m_selectedIndices.begin(), m_selectedIndices.end(), qGreater<QModelIndex>());
            qSort(selectedRows);
            int maxRow = 0;
            int minRow = 0;
            if (!selectedRows.isEmpty()) {
                maxRow = selectedRows.last();
                minRow = selectedRows.first();
            }
            int selectedRowCount = selectedRows.count();
            int firstRowToSelect = destIndex.row();

            //If you drag a contiguous selection of multiple tracks and drop
            //them somewhere inside that same selection, do nothing.
            if (destIndex.row() >= minRow && destIndex.row() <= maxRow)
                return;

            //If we're moving the tracks _up_, then reverse the order of the row selection
            //to make the algorithm below work without added complexity.
            if (destIndex.row() < minRow) {
                qSort(selectedRows.begin(), selectedRows.end(), qGreater<int>());
            }

            if (destIndex.row() > maxRow) {
                //Shuffle the row we're going to start making a new selection at:
                firstRowToSelect = firstRowToSelect - selectedRowCount + 1;
            }

            //For each row that needs to be moved...
            while (!selectedRows.isEmpty()) {
                int movedRow = selectedRows.takeFirst(); //Remember it's row index
                //Move it
                trackModel->moveTrack(model()->index(movedRow, 0), destIndex);

                //Shuffle the row indices for rows that got bumped up
                //into the void we left, or down because of the new spot
                //we're taking.
                for (int i = 0; i < selectedRows.count(); i++) {
                    if ((selectedRows[i] > movedRow) &&
                        (destIndex.row() > selectedRows[i])) {
                        selectedRows[i] = selectedRows[i] - 1;
                    } else if ((selectedRows[i] < movedRow) &&
                                (destIndex.row() < selectedRows[i])) {
                        selectedRows[i] = selectedRows[i] + 1;
                    }
                }
            }

            //Highlight the moved rows again (restoring the selection)
            //QModelIndex newSelectedIndex = destIndex;
            for (int i = 0; i < selectedRowCount; i++) {
                this->selectionModel()->select(model()->index(firstRowToSelect + i, 0),
                                                QItemSelectionModel::Select | QItemSelectionModel::Rows);
            }
        }
    } else {//Drag and drop inside Mixxx is only for few rows, bulks happen here
        //Reset the selected tracks (if you had any tracks highlighted, it
        //clears them)
        this->selectionModel()->clear();

        //Drag-and-drop from an external application
        //eg. dragging a track from Windows Explorer onto the track table.
        TrackModel* trackModel = getTrackModel();
        if (trackModel) {
            int numNewRows = urls.count();

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
            QList<QString> fileLocationList;
            foreach(url, urls) {
                QString file(url.toLocalFile());
                fileLocationList.append(file);
            }
            // calling the addTracks returns number of failed additions
            int tracksAdded = trackModel->addTracks(destIndex, fileLocationList);

            // Decrement # of rows to select if some were skipped
            numNewRows -= (fileLocationList.size() - tracksAdded);

            // Create the selection, but only if the track model supports
            // reordering. (eg. crates don't support reordering/indexes)
            if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_REORDER)) {
                for (int i = selectionStartRow; i < selectionStartRow + numNewRows; i++) {
                    this->selectionModel()->select(model()->index(i, 0),
                                                    QItemSelectionModel::Select |
                                                    QItemSelectionModel::Rows);
                }
            }
        }
    }

    event->acceptProposedAction();
    restoreVScrollBarPos();

}

TrackModel* WTrackTableView::getTrackModel() {
    TrackModel* trackModel = dynamic_cast<TrackModel*>(model());
    return trackModel;
}

bool WTrackTableView::modelHasCapabilities(TrackModel::CapabilitiesFlags capabilities) {
    TrackModel* trackModel = getTrackModel();
    return trackModel &&
            (trackModel->getCapabilities() & capabilities) == capabilities;
}

void WTrackTableView::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Return) {
        // It is not a good idea if 'key_return'
        // causes a track to load since we allow in-line editing
        // of table items in general
        return;
    } else if (event->key() == Qt::Key_BracketLeft) {
        loadSelectionToGroup("[Channel1]");
    } else if (event->key() == Qt::Key_BracketRight) {
        loadSelectionToGroup("[Channel2]");
    } else {
        QTableView::keyPressEvent(event);
    }
}

void WTrackTableView::loadSelectedTrack() {
    QModelIndexList indexes = selectionModel()->selectedRows();
    if (indexes.size() > 0) {
        slotMouseDoubleClicked(indexes.at(0));
    }
}

void WTrackTableView::loadSelectedTrackToGroup(QString group, bool play) {
    loadSelectionToGroup(group, play);
}

void WTrackTableView::slotSendToAutoDJ() {
    // append to auto DJ
    sendToAutoDJ(false); // bTop = false
}

void WTrackTableView::slotSendToAutoDJTop() {
    sendToAutoDJ(true); // bTop = true
}

void WTrackTableView::sendToAutoDJ(bool bTop) {
    if (!modelHasCapabilities(TrackModel::TRACKMODELCAPS_ADDTOAUTODJ)) {
        return;
    }

    PlaylistDAO& playlistDao = m_pTrackCollection->getPlaylistDAO();
    int iAutoDJPlaylistId = playlistDao.getPlaylistIdFromName(AUTODJ_TABLE);
    if (iAutoDJPlaylistId == -1) {
        return;
    }

    QModelIndexList indices = selectionModel()->selectedRows();
    QList<int> trackIds;

    TrackModel* trackModel = getTrackModel();
    if (!trackModel) {
        return;
    }

    foreach (QModelIndex index, indices) {
        TrackPointer pTrack = trackModel->getTrack(index);
        if (pTrack) {
            int iTrackId = pTrack->getId();
            if (iTrackId == -1) {
                continue;
            }
            trackIds.append(iTrackId);
        }
    }

    if (bTop) {
        // Load track to position two because position one is
        // already loaded to the player
        playlistDao.insertTracksIntoPlaylist(trackIds,
                                            iAutoDJPlaylistId, 2);
    } else {
        playlistDao.appendTracksToPlaylist(
            trackIds, iAutoDJPlaylistId);
    }
}

void WTrackTableView::slotReloadTrackMetadata() {
    if (!modelHasCapabilities(TrackModel::TRACKMODELCAPS_RELOADMETADATA)) {
        return;
    }

    QModelIndexList indices = selectionModel()->selectedRows();

    TrackModel* trackModel = getTrackModel();

    if (trackModel == NULL) {
        return;
    }

    foreach (QModelIndex index, indices) {
        TrackPointer pTrack = trackModel->getTrack(index);
        if (pTrack) {
            pTrack->parse();
        }
    }
}

//slot for reset played count, sets count to 0 of one or more tracks
void WTrackTableView::slotResetPlayed() {
    QModelIndexList indices = selectionModel()->selectedRows();
    TrackModel* trackModel = getTrackModel();

    if (trackModel == NULL) {
        return;
    }

    foreach (QModelIndex index, indices) {
        TrackPointer pTrack = trackModel->getTrack(index);
        if (pTrack) {
            pTrack->setTimesPlayed(0);
        }
    }
}

void WTrackTableView::addSelectionToPlaylist(int iPlaylistId) {
    PlaylistDAO& playlistDao = m_pTrackCollection->getPlaylistDAO();
    TrackModel* trackModel = getTrackModel();

    if (!trackModel) {
        return;
    }

    QModelIndexList indices = selectionModel()->selectedRows();
    QList<int> trackIds;
    foreach (QModelIndex index, indices) {
        TrackPointer pTrack = trackModel->getTrack(index);
        if (!pTrack) {
            continue;
        }
        int iTrackId = pTrack->getId();
        if (iTrackId != -1) {
            trackIds.append(iTrackId);
        }
    }
    if (trackIds.size() > 0) {
        playlistDao.appendTracksToPlaylist(trackIds, iPlaylistId);
    }
}

void WTrackTableView::addSelectionToCrate(int iCrateId) {
    CrateDAO& crateDao = m_pTrackCollection->getCrateDAO();
    TrackModel* trackModel = getTrackModel();

    if (!trackModel) {
        return;
    }

    QModelIndexList indices = selectionModel()->selectedRows();
    QList<int> trackIds;
    foreach (QModelIndex index, indices) {
        TrackPointer pTrack = trackModel->getTrack(index);
        if (!pTrack) {
            continue;
        }
        int iTrackId = pTrack->getId();
        if (iTrackId != -1) {
            trackIds.append(iTrackId);
        }
    }

    if (trackIds.size() > 0) {
        crateDao.addTracksToCrate(trackIds, iCrateId);
    }
}

void WTrackTableView::doSortByColumn(int headerSection) {
    TrackModel* trackModel = getTrackModel();
    QAbstractItemModel* itemModel = model();

    if (trackModel == NULL || itemModel == NULL || m_sorting == false)
        return;

    // Save the selection
    QModelIndexList selection = selectionModel()->selectedRows();
    QSet<int> trackIds;
    foreach (QModelIndex index, selection) {
        int trackId = trackModel->getTrackId(index);
        trackIds.insert(trackId);
    }

    sortByColumn(headerSection);

    QItemSelectionModel* currentSelection = selectionModel();

    // Find a visible column
    int visibleColumn = 0;
    while (isColumnHidden(visibleColumn) && visibleColumn < itemModel->columnCount()) {
        visibleColumn++;
    }

    currentSelection->reset(); // remove current selection

    QMap<int,int> selectedRows;
    foreach (int trackId, trackIds) {

        // TODO(rryan) slowly fixing the issues with BaseSqlTableModel. This
        // code is broken for playlists because it assumes each trackid is in
        // the table once. This will erroneously select all instances of the
        // track for playlists, but it works fine for every other view. The way
        // to fix this that we should do is to delegate the selection saving to
        // the TrackModel. This will allow the playlist table model to use the
        // table index as the unique id instead of this code stupidly using
        // trackid.
        QLinkedList<int> rows = trackModel->getTrackRows(trackId);
        foreach (int row, rows) {
            // Restore sort order by rows, so the following commands will act as expected
            selectedRows.insert(row,0);
        }
    }

    QModelIndex first;
    QMapIterator<int,int> i(selectedRows);
    while (i.hasNext()) {
        i.next();
        QModelIndex tl = itemModel->index(i.key(), visibleColumn);
        currentSelection->select(tl, QItemSelectionModel::Rows | QItemSelectionModel::Select);

        if (!first.isValid()) {
            first = tl;
        }
    }

    if (first.isValid()) {
        scrollTo(first, QAbstractItemView::EnsureVisible);
        //scrollTo(first, QAbstractItemView::PositionAtCenter);
    }
}

void WTrackTableView::slotLockBpm() {
    lockBpm(true);
}

void WTrackTableView::slotUnlockBpm() {
    lockBpm(false);
}

void WTrackTableView::lockBpm(bool lock) {
    TrackModel* trackModel = getTrackModel();
    if (trackModel == NULL) {
        return;
    }

    QModelIndexList selectedTrackIndices = selectionModel()->selectedRows();
    // TODO: This should be done in a thread for large selections
    for (int i = 0; i < selectedTrackIndices.size(); ++i) {
        QModelIndex index = selectedTrackIndices.at(i);
        TrackPointer track = trackModel->getTrack(index);
        track->setBpmLock(lock);
    }
}

void WTrackTableView::slotClearBeats() {
    TrackModel* trackModel = getTrackModel();
    if (trackModel == NULL) {
        return;
    }

    QModelIndexList selectedTrackIndices = selectionModel()->selectedRows();
    // TODO: This should be done in a thread for large selections
    for (int i = 0; i < selectedTrackIndices.size(); ++i) {
        QModelIndex index = selectedTrackIndices.at(i);
        TrackPointer track = trackModel->getTrack(index);
        if (!track->hasBpmLock()) {
            track->setBeats(BeatsPointer());
        }
    }
}
