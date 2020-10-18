
#include "widget/winfobar.h"

#include <QDebug>
#include <QFrame>
#include <QHBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMetaType>
#include <QPushButton>
#include <QScrollArea>
#include <QStringList>
#include <QUrl>
#include <QVBoxLayout>

#include "control/controlobject.h"
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
WInfoBarCrateItem::WInfoBarCrateItem(const CrateSummary crate, QWidget* parent)
        : WInfoBarItem(crate.getName(), parent) {
    //setText();
}

// crates which are only occupied by some tracks
WInfoBarCrateMixedItem::WInfoBarCrateMixedItem(const CrateSummary crate, QWidget* parent)
        : WInfoBarCrateItem(crate, parent) {
}

WInfoBarWorker::WInfoBarWorker(QObject* parent, TrackCollectionManager* manager)
        : QObject(parent),
          m_pTrackCollectionManager(manager) {
    DEBUG_ASSERT(manager != nullptr);
}

void WInfoBarWorker::query(int query, QList<TrackId> trackIds) {
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

QThread* WInfoBar::s_worker_thread = nullptr; //QThread();

WInfoBar::WInfoBar(QString group,
        UserSettingsPointer pConfig,
        Library* pLibrary,
        QWidget* pParent)
        : QScrollArea(pParent),
          WBaseWidget(pParent),
          m_pGroup(group),
          m_pConfig(pConfig),
          m_pLibrary(pLibrary) {
    // FIXME(poelzi): does it make sense to allow drops of certain stuff ???
    setAcceptDrops(false);
    setMinimumSize(0, 0);
    setWidgetResizable(true);
    setFocusPolicy(Qt::NoFocus);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

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

    connect(m_pWorker,
            &WInfoBarWorker::crateResult,
            this,
            &WInfoBar::receiveCrateResults);
    connect(this,
            &WInfoBar::query,
            m_pWorker,
            &WInfoBarWorker::query);
}

// generates unique id's for all worker requests
int WInfoBar::s_last_id = 0;

int WInfoBar::generateId() {
    return ++s_last_id;
}

void WInfoBar::receiveCrateResults(int query, uint total, QList<CrateSummary> crates) {
    qDebug() << "WInfoBar::receiveCrateResults" << query << total;
    // we are only interested in results we requested last
    if (query != m_id) {
        return;
    }

    foreach (CrateSummary crate, crates) {
        if (crate.getTrackCount() == total) {
            auto item = new WInfoBarCrateItem(crate, this);
            m_pCratesContainer->addWidget(item);
            m_pCratesContainer->setAlignment(item, Qt::AlignLeft | Qt::AlignTop);
        } else {
            auto item = new WInfoBarCrateMixedItem(crate, this);
            m_pCratesContainer->addWidget(item);
            m_pCratesContainer->setAlignment(item, Qt::AlignLeft | Qt::AlignTop);
        }
    }
    m_pCratesContainer->addStretch();
}

void WInfoBar::slotTrackLoaded(TrackPointer track) {
    clear();
    if (track == nullptr) {
        return;
    }
    m_id = generateId();

    QList<TrackId> lst = QList<TrackId>();
    lst.append(track->getId());
    emit query(m_id, lst);
}

void WInfoBar::slotTrackSelection(QList<TrackPointer> tracks) {
    clear();
    qDebug() << "WInfoBar::slotTrackSelection";

    m_id = generateId();

    QList<TrackId> lst = QList<TrackId>();
    lst.reserve(tracks.size());

    foreach (auto track, tracks) {
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

void WInfoBar::setupUI() {
    m_pContainerFrame = new QFrame();
    m_pContainerFrame->setObjectName(QString::fromUtf8("infobarFrame"));
    m_pContainerFrame->setMinimumSize(0, 0);

    m_pContainer = new QHBoxLayout();
    m_pContainer->setSpacing(0);
    m_pContainer->setObjectName(QString::fromUtf8("infobarContainer"));
    m_pContainerFrame->setLayout(m_pContainer);

    m_pCratesFrame = new QFrame();
    m_pCratesFrame->setObjectName(QString::fromUtf8("infobarCrates"));
    m_pCratesFrame->setMinimumSize(0, 0);
    m_pCratesFrame->setSizePolicy(QSizePolicy(
            QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Minimum));

    m_pCratesContainer = new QHBoxLayout();
    m_pCratesContainer->setObjectName(QString::fromUtf8("infobarCratesContainer"));
    m_pCratesContainer->setSpacing(0);
    //m_pCratesContainer->setMinimumSize(0, 0);
    m_pCratesFrame->setLayout(m_pCratesContainer);

    m_pPlaylistsFrame = new QFrame();
    m_pPlaylistsFrame->setObjectName(QString::fromUtf8("infobarPlaylists"));
    m_pPlaylistsFrame->setMinimumSize(0, 0);
    m_pPlaylistsFrame->setSizePolicy(QSizePolicy(
            QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Minimum));

    m_pPlaylistsContainer = new QHBoxLayout();
    m_pPlaylistsContainer->setObjectName(QString::fromUtf8("infobarPlaylistsContainer"));
    //m_pPlaylistsContainer->setMinimumSize(0, 0);
    m_pPlaylistsFrame->setLayout(m_pPlaylistsContainer);

    m_pHistoryFrame = new QFrame();
    m_pHistoryFrame->setObjectName(QString::fromUtf8("infobarHistory"));
    m_pHistoryFrame->setMinimumSize(0, 0);
    m_pHistoryFrame->setSizePolicy(QSizePolicy(
            QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Minimum));

    m_pHistoryContainer = new QHBoxLayout();
    m_pHistoryContainer->setObjectName(QString::fromUtf8("infobarHistoryContainer"));
    //m_pHistoryContainer->setMinimumSize(0, 0);
    m_pHistoryFrame->setLayout(m_pHistoryContainer);

    m_pContainer->addWidget(m_pCratesFrame, 0);
    m_pContainer->addWidget(m_pPlaylistsFrame, 0);
    m_pContainer->addWidget(m_pHistoryFrame, 0);
    m_pContainer->addStretch();
    //m_pContainer->addSpacing(0);
    m_pContainerFrame->setLayout(m_pContainer);
    setWidget(m_pContainerFrame);
    //setLayout(m_pContainer);
}

void WInfoBar::clear() {
    QLayoutItem* child;
    while ((child = m_pCratesContainer->takeAt(0)) != nullptr) {
        delete child->widget(); // delete the widget
        delete child;           // delete the layout item
    }
    while ((child = m_pPlaylistsContainer->takeAt(0)) != nullptr) {
        delete child->widget(); // delete the widget
        delete child;           // delete the layout item
    }
    while ((child = m_pHistoryContainer->takeAt(0)) != nullptr) {
        delete child->widget(); // delete the widget
        delete child;           // delete the layout item
    }
}

void WInfoBar::mouseMoveEvent(QMouseEvent* event) {
    if ((event->buttons() & Qt::LeftButton) && m_pCurrentTrack) {
        DragAndDropHelper::dragTrack(m_pCurrentTrack, this, m_pGroup);
    }
}

void WInfoBar::dragEnterEvent(QDragEnterEvent* event) {
    event->ignore();
}

void WInfoBar::dropEvent(QDropEvent* event) {
    event->ignore();
}
