
#include "widget/winfobar.h"

#include <QAction>
#include <QDebug>
#include <QFrame>
#include <QHBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMetaType>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QStringList>
#include <QUrl>
#include <QVBoxLayout>
#include <Qt>

#include "control/controlobject.h"
#include "library/trackset/crate/cratefeature.h"
#include "library/trackset/playlistfeature.h"
#include "library/trackset/setlogfeature.h"
#include "util/dnd.h"

// crates which are only occupied by some tracks
WInfoBarItem::WInfoBarItem(const QString& text, QWidget* parent)
        : QPushButton(text, parent) {
    setFlat(true);
    setMinimumSize(0, 0);
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy(QSizePolicy::Policy::Preferred, sizePolicy().verticalPolicy()));
}

// crates in which all tracks are part of
WInfoBarCrateItem::WInfoBarCrateItem(const CrateSummary crt, QWidget* parent)
        : WInfoBarItem(crt.getName(), parent),
          m_crate(crt) {
    connect(this,
            &QAbstractButton::clicked,
            [this] {
                emit(activateCrate(m_crate));
            });
}

// crates which are only occupied by some tracks
WInfoBarPlaylistItem::WInfoBarPlaylistItem(
        const PlaylistSummary playlist, bool isHistory, QWidget* parent)
        : WInfoBarItem(playlist.name(), parent),
          m_playlist(playlist),
          m_isHistory(isHistory) {
    connect(this,
            &QAbstractButton::clicked,
            [this] {
                emit(activatePlaylist(m_playlist, m_isHistory));
            });
}

WInfoBarWorker::WInfoBarWorker(QObject* parent, TrackCollectionManager* manager)
        : QObject(parent),
          m_pTrackCollectionManager(manager) {
    DEBUG_ASSERT(manager != nullptr);
}

void WInfoBarWorker::query(int query, QList<TrackId> trackIds) {
    queryCrates(query, trackIds);
    queryPlaylists(query, trackIds);
    queryHistory(query, trackIds);
}

void WInfoBarWorker::queryCrates(int query, QList<TrackId> trackIds) {
    QString where = QString(" WHERE %1 > 0 ").arg("track_count");

    QList<CrateSummary> rv = QList<CrateSummary>();
    rv.reserve(trackIds.length());

    CrateSummarySelectResult results =
            m_pTrackCollectionManager->internalCollection()
                    ->crates()
                    .selectCratesWithTrackCount(trackIds);

    CrateSummary crate;
    while (results.populateNext(&crate)) {
        if (crate.getTrackCount() == 0)
            continue;
        rv.append(crate);
    }
    emit(crateResult(query, trackIds.length(), rv));
}

void WInfoBarWorker::queryPlaylists(int query, QList<TrackId> trackIds) {
    //QString where = QString(" WHERE %1 > 0 ").arg("track_count");

    QList<PlaylistSummary> rv = QList<PlaylistSummary>();
    rv.reserve(trackIds.length());

    QList<PlaylistSummary> results =
            m_pTrackCollectionManager->internalCollection()
                    ->getPlaylistDAO()
                    .createPlaylistSummaryForTracks(trackIds);

    emit(playlistResult(query, trackIds.length(), results));
}

void WInfoBarWorker::queryHistory(int query, QList<TrackId> trackIds) {
    //QString where = QString(" WHERE %1 > 0 ").arg("track_count");

    QList<PlaylistSummary> rv = QList<PlaylistSummary>();
    rv.reserve(trackIds.length());

    QList<PlaylistSummary> results =
            m_pTrackCollectionManager->internalCollection()
                    ->getPlaylistDAO()
                    .createPlaylistSummaryForTracks(
                            trackIds, PlaylistDAO::HiddenType::PLHT_SET_LOG);
    qDebug() << "history result length" << results.length();

    emit(historyResult(query, trackIds.length(), results));
}

WInfoBarButton::WInfoBarButton(QWidget* parent)
        : QToolButton(parent) {
    /*    connect(this,
                &QToolButton::clicked,
                this,
                &WInfoBarButton::slotTriggered);
*/
    setState(State::VISIBLE);
}

void WInfoBarButton::setState(WInfoBarButton::State newState) {
    m_state = newState;
    qDebug() << "updated";
    style()->unpolish(this);
    style()->polish(this);
    emit(stateChanged(m_state));
};

void WInfoBarButton::nextCheckState() {
    //Q_UNUSED(action);
    qDebug() << "triggered";
    switch (m_state) {
    case WInfoBarButton::State::VISIBLE:
        m_state = WInfoBarButton::State::EXPANDED;
        break;
    case WInfoBarButton::State::EXPANDED:
        m_state = WInfoBarButton::State::HIDDEN;
        break;
    default:
        m_state = WInfoBarButton::State::VISIBLE;
    }
    style()->unpolish(this);
    style()->polish(this);
    emit(stateChanged(m_state));
}

WInfoBarContainer::WInfoBarContainer(QString name,
        QWidget* pParent)
        : QFrame(pParent),
          m_pButton(new WInfoBarButton(this)),
          m_pContainer(nullptr),
          m_pMainLayout(nullptr),
          m_pMainContainer(nullptr) {
    setObjectName(QString("%1Frame").arg(name));

    m_pContainer = new QHBoxLayout(this);
    m_pContainer->setSpacing(2);
    m_pContainer->setContentsMargins(QMargins(2, 0, 2, 0));

    m_pMainContainer = new QWidget(this);
    m_pMainContainer->setObjectName(QString("%1Container").arg(name));
    m_pMainContainer->setLayout(m_pContainer);

    m_pMainLayout = new QHBoxLayout(this);
    m_pMainLayout->setContentsMargins(QMargins(0, 0, 0, 0));
    m_pMainLayout->addWidget(m_pButton);
    m_pMainLayout->addWidget(m_pMainContainer);

    m_pButton->setObjectName(QString("%1Button").arg(name));
    connect(m_pButton,
            &WInfoBarButton::stateChanged,
            this,
            &WInfoBarContainer::slotStateChange);

    setLayout(m_pMainLayout);
}

void WInfoBarContainer::slotStateChange(WInfoBarButton::State state) {
    qDebug() << "stateChange" << static_cast<int>(state);
    if (state == WInfoBarButton::State::HIDDEN) {
        m_pMainContainer->setVisible(false);
    } else {
        m_pMainContainer->setVisible(true);
    }
    if (state == WInfoBarButton::State::EXPANDED) {
        m_pMainContainer->setSizePolicy(QSizePolicy(QSizePolicy::Policy::Expanding,
                QSizePolicy::Policy::Minimum));
        m_pMainContainer->setMaximumSize(QSize(1000000, 20));
    } else {
        m_pMainContainer->setSizePolicy(QSizePolicy(QSizePolicy::Policy::Minimum,
                QSizePolicy::Policy::Minimum));
    }
    emit(stateChanged(state));
}

void WInfoBarContainer::clear() {
    QLayoutItem* child;
    while ((child = m_pContainer->takeAt(0)) != nullptr) {
        delete child->widget(); // delete the widget
        delete child;           // delete the layout item
    }
}

void WInfoBarContainer::addItem(WInfoBarItem* item) {
    m_pContainer->addWidget(item);
    m_pContainer->setAlignment(item, Qt::AlignLeft | Qt::AlignTop);
}

void WInfoBarContainer::addStretch(int factor) {
    m_pContainer->addStretch(factor);
}

// WInfoBar implementation
QThread* WInfoBar::s_worker_thread = nullptr; //QThread();

WInfoBar::WInfoBar(QString group,
        UserSettingsPointer pConfig,
        Library* pLibrary,
        QWidget* pParent)
        : QStatusBar(pParent),
          WBaseWidget(pParent),
          m_pGroup(group),
          m_pConfig(pConfig),
          m_pLibrary(pLibrary) {
    // FIXME(poelzi): does it make sense to allow drops of certain stuff ???
    setAcceptDrops(false);
    setMinimumSize(0, 0);
    //setWidgetResizable(true);
    setFocusPolicy(Qt::NoFocus);
    //setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // initialize the shared worker thread
    if (s_worker_thread == nullptr) {
        // initialize the worker thread shared between all widgets on first use
        s_worker_thread = new QThread(nullptr);
        s_worker_thread->setObjectName("WInfoBarworker");
        s_worker_thread->start(QThread::LowPriority);
        qDebug() << "worker thread initialized";
    }
    m_pWorker = new WInfoBarWorker(this, pLibrary->trackCollections());
    m_pWorker->moveToThread(s_worker_thread);

    // we use queued connections because they connect to a different thread
    connect(m_pWorker,
            &WInfoBarWorker::crateResult,
            this,
            &WInfoBar::receiveCrateResults,
            Qt::QueuedConnection);
    connect(m_pWorker,
            &WInfoBarWorker::playlistResult,
            this,
            &WInfoBar::receivePlaylistResults,
            Qt::QueuedConnection);
    connect(m_pWorker,
            &WInfoBarWorker::historyResult,
            this,
            &WInfoBar::receiveHistoryResults,
            Qt::QueuedConnection);
    connect(this,
            &WInfoBar::query,
            m_pWorker,
            &WInfoBarWorker::query,
            Qt::QueuedConnection);

    // crate updates
    connect(&m_pLibrary->trackCollection(),
            &TrackCollection::crateTracksChanged,
            this,
            &WInfoBar::slotCrateTracksChanged);
    // very seldom, so we build it eagerly
    connect(&m_pLibrary->trackCollection(),
            &TrackCollection::crateSummaryChanged,
            [this](const QSet<CrateId>& crates) {
                Q_UNUSED(crates);
                queryCurrentTracks();
            });
}

// generates unique id's for all worker requests
int WInfoBar::s_last_id = 0;

int WInfoBar::generateId() {
    return ++s_last_id;
}

void WInfoBar::slotCrateTracksChanged(CrateId crate,
        const QList<TrackId>& tracksAdded,
        const QList<TrackId>& tracksRemoved) {
    Q_UNUSED(crate);
    bool rebuild = false;
    for (TrackPointer track : qAsConst(m_currentTracks)) {
        const TrackId id = track->getId();
        if (tracksAdded.contains(id) || tracksRemoved.contains(id)) {
            rebuild = true;
            break;
        }
    }
    if (rebuild) {
        // A currently loaded track is affected, rebuild the bar
        queryCurrentTracks();
    }
}

void WInfoBar::receiveCrateResults(int queryId, int total, QList<CrateSummary> crates) {
    qDebug() << "WInfoBar::receiveCrateResults" << queryId << total << crates.length();
    // we are only interested in results we requested last
    if (queryId != m_id) {
        return;
    }
    m_pCratesContainer->clear();
    foreach (CrateSummary crate, crates) {
        auto item = new WInfoBarCrateItem(crate, m_pCratesContainer);
        item->setTotals(total);

        connect(item,
                &WInfoBarCrateItem::activateCrate,
                this,
                &WInfoBar::slotActivateCrate);

        m_pCratesContainer->addItem(item);
    }
    m_pCratesContainer->addStretch(0);
    //m_pCratesContainer->
    //updateVisibilty();
    //m_pCratesContainer->addStretch();
}

void WInfoBar::slotActivateCrate(CrateSummary crate) {
    VERIFY_OR_DEBUG_ASSERT(m_pLibrary && m_pLibrary->getCreateFeature()) {
        return;
    }
    m_pLibrary->getCreateFeature()->activate();
    m_pLibrary->getCreateFeature()->activateCrate(crate.getId());
}

void WInfoBar::slotActivatePlaylist(PlaylistSummary playlist, bool isHistory) {
    VERIFY_OR_DEBUG_ASSERT(m_pLibrary) {
        return;
    }
    if (isHistory) {
        VERIFY_OR_DEBUG_ASSERT(m_pLibrary->getSetlogFeature()) {
            return;
        }
        m_pLibrary->getSetlogFeature()->activate();
        m_pLibrary->getSetlogFeature()->activatePlaylist(playlist.id());
    } else {
        VERIFY_OR_DEBUG_ASSERT(m_pLibrary->getPlaylistFeature()) {
            return;
        }
        m_pLibrary->getPlaylistFeature()->activate();
        m_pLibrary->getPlaylistFeature()->activatePlaylist(playlist.id());
    }
}

void WInfoBar::receivePlaylistResults(int queryId, int total, QList<PlaylistSummary> playlists) {
    addPlaylistResults(false, queryId, total, playlists);
}

void WInfoBar::receiveHistoryResults(int queryId, int total, QList<PlaylistSummary> playlists) {
    addPlaylistResults(true, queryId, total, playlists);
}

void WInfoBar::addPlaylistResults(bool isHistory,
        int queryId,
        int total,
        QList<PlaylistSummary> playlists) {
    qDebug() << "WInfoBar::receivePlaylistResults" << isHistory << queryId
             << total << playlists.length();
    WInfoBarContainer* container;
    if (isHistory) {
        container = m_pHistoryContainer;
    } else {
        container = m_pPlaylistsContainer;
    }
    // we are only interested in results we requested last
    if (queryId != m_id) {
        return;
    }
    // clear the container in case this was a refresh request
    container->clear();
    foreach (PlaylistSummary playlist, playlists) {
        auto item = new WInfoBarPlaylistItem(playlist, isHistory, container);
        item->setTotals(total);

        connect(item,
                &WInfoBarPlaylistItem::activatePlaylist,
                this,
                &WInfoBar::slotActivatePlaylist);

        container->addItem(item);
    }
    container->addStretch(0);

    //updateVisibilty();
    //m_pCratesContainer->addStretch();
}

void WInfoBar::clearCurrentTracks() {
    foreach (TrackPointer track, qAsConst(m_currentTracks)) {
        VERIFY_OR_DEBUG_ASSERT(track) {
            continue;
        }
        disconnect(track.get(),
                nullptr,
                this,
                nullptr);
    }

    m_currentTracks.clear();
}

void WInfoBar::slotTrackLoaded(TrackPointer track) {
    clear();
    if (track == nullptr) {
        return;
    }
    m_id = generateId();

    clearCurrentTracks();
    m_currentTracks.append(track);

    queryCurrentTracks();
}

void WInfoBar::slotTrackSelection(QList<TrackPointer> tracks) {
    clear();
    qDebug() << "WInfoBar::slotTrackSelection";

    clearCurrentTracks();
    m_currentTracks = tracks;

    queryCurrentTracks();
}

void WInfoBar::queryCurrentTracks() {
    m_id = generateId();

    QList<TrackId> lst = QList<TrackId>();
    lst.reserve(m_currentTracks.size());

    foreach (auto track, qAsConst(m_currentTracks)) {
        if (track != nullptr) {
            lst.append(track->getId());
        }
    }

    emit query(m_id, lst);
}

void WInfoBar::setup(const QDomNode& node, const SkinContext& context) {
    Q_UNUSED(node);
    Q_UNUSED(context);

    setupUI();
}

void WInfoBar::prepareFrame(QFrame* frame) {
    frame->setMinimumSize(0, 0);
    frame->setSizePolicy(QSizePolicy(
            QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Minimum));
}

void WInfoBar::setupUI() {
    /*
    m_pContainerFrame = new QFrame(this);
    m_pContainerFrame->setObjectName(QLatin1String("infobarFrame"));
    m_pContainerFrame->setMinimumSize(0, 0);

    m_pContainer = new QHBoxLayout();
    m_pContainer->setSpacing(0);
    m_pContainer->setObjectName(QLatin1String("infobarContainer"));
    m_pContainer->setContentsMargins(QMargins(0, 0, 0, 0));
    m_pContainer->setSpacing(0);
    m_pContainerFrame->setLayout(m_pContainer);

    m_pCratesFrame = new QFrame(this);
    m_pCratesFrame->setObjectName(QLatin1String("infobarCrates"));
    prepareFrame(m_pCratesFrame);

    m_pCratesContainer = new QHBoxLayout();
    m_pCratesContainer->setObjectName(QLatin1String("infobarCratesContainer"));
    m_pCratesContainer->setContentsMargins(QMargins(0, 0, 0, 0));
    m_pCratesContainer->setSpacing(0);
    //m_pCratesContainer->setMinimumSize(0, 0);
    m_pCratesFrame->setLayout(m_pCratesContainer);

    m_pPlaylistsFrame = new QFrame(this);
    m_pPlaylistsFrame->setObjectName(QLatin1String("infobarPlaylists"));
    prepareFrame(m_pPlaylistsFrame);


    m_pPlaylistsContainer = new QHBoxLayout();
    m_pPlaylistsContainer->setObjectName(QLatin1String("infobarPlaylistsContainer"));
    //m_pPlaylistsContainer->setMinimumSize(0, 0);
    m_pPlaylistsContainer->setContentsMargins(QMargins(0, 0, 0, 0));
    m_pPlaylistsContainer->setSpacing(0);

    m_pPlaylistsFrame->setLayout(m_pPlaylistsContainer);

    m_pHistoryFrame = new QFrame(this);
    m_pHistoryFrame->setObjectName(QLatin1String("infobarHistory"));
    prepareFrame(m_pHistoryFrame);

    m_pHistoryContainer = new QHBoxLayout();
    m_pHistoryContainer->setObjectName(QLatin1String("infobarHistoryContainer"));
    m_pHistoryContainer->setContentsMargins(QMargins(0, 0, 0, 0));
    m_pHistoryContainer->setSpacing(0);

    //m_pHistoryContainer->setMinimumSize(0, 0);
    m_pHistoryFrame->setLayout(m_pHistoryContainer);
    */
    m_pCratesContainer = new WInfoBarContainer("InfobarCrates", this);
    connect(m_pCratesContainer,
            &WInfoBarContainer::stateChanged,
            this,
            &WInfoBar::slotRebuildWidgets);

    m_pPlaylistsContainer = new WInfoBarContainer("InfobarPlaylists", this);
    connect(m_pPlaylistsContainer,
            &WInfoBarContainer::stateChanged,
            this,
            &WInfoBar::slotRebuildWidgets);

    m_pHistoryContainer = new WInfoBarContainer("InfobarHistory", this);
    connect(m_pHistoryContainer,
            &WInfoBarContainer::stateChanged,
            this,
            &WInfoBar::slotRebuildWidgets);

    //m_pContainer->addWidget(m_pCratesFrame, 0);
    //m_pContainer->addWidget(m_pPlaylistsFrame, 0);
    //m_pContainer->addWidget(m_pHistoryFrame, 0);
    //m_pContainer->addStretch();
    //m_pContainer->addSpacing(0);
    //m_pContainerFrame->setLayout(m_pContainer);
    //setWidget(m_pContainerFrame);
    //setLayout(m_pContainer);
    setSizeGripEnabled(false);
    setMinimumWidth(0);
    rebuildWidgets();
}

void WInfoBar::rebuildWidgets() {
    removeWidget(m_pCratesContainer);
    removeWidget(m_pPlaylistsContainer);
    removeWidget(m_pHistoryContainer);

    insertWidget(0,
            m_pCratesContainer,
            m_pCratesContainer->state() == WInfoBarButton::State::EXPANDED ? 1
                                                                           : 0);
    insertWidget(1,
            m_pPlaylistsContainer,
            m_pPlaylistsContainer->state() == WInfoBarButton::State::EXPANDED
                    ? 1
                    : 0);
    insertWidget(2,
            m_pHistoryContainer,
            m_pHistoryContainer->state() == WInfoBarButton::State::EXPANDED
                    ? 1
                    : 0);

    m_pCratesContainer->show();
    m_pPlaylistsContainer->show();
    m_pHistoryContainer->show();
}

void WInfoBar::clear() {
    m_pCratesContainer->clear();
    m_pPlaylistsContainer->clear();
    m_pHistoryContainer->clear();
}

void WInfoBar::contextMenuEvent(QContextMenuEvent* event) {
    Q_UNUSED(event);
    qDebug() << "contextmen";
}

// void WInfoBar::mouseMoveEvent(QMouseEvent* event) {
//     if ((event->buttons() & Qt::LeftButton) && m_pCurrentTrack) {
//         DragAndDropHelper::dragTrack(m_pCurrentTrack, this, m_pGroup);
//     }
// }

void WInfoBar::dragEnterEvent(QDragEnterEvent* event) {
    event->ignore();
}

void WInfoBar::dropEvent(QDropEvent* event) {
    event->ignore();
}
