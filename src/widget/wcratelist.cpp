
#include <QDebug>
#include <QUrl>
#include <QMetaType>
#include <QListWidget>
#include <QStringList>
#include <QListWidgetItem>

#include "control/controlobject.h"
#include "widget/wcratelist.h"
#include "util/dnd.h"


WCrateListWorker::WCrateListWorker(QObject* parent, const TrackCollection* pCollection):
    QObject(parent) {
    DEBUG_ASSERT(pCollection != nullptr);

    m_pTrackCollection = pCollection;

}

WCrateItem::WCrateItem(const QString &text, QListWidget *parent):
    QListWidgetItem(text, parent, QListWidgetItem::UserType) {}

WCrateItemSome::WCrateItemSome(const QString &text, QListWidget *parent):
    QListWidgetItem(text, parent, QListWidgetItem::UserType) {}


void WCrateListWorker::query(int query, QList<TrackId> trackIds) {
    QString where = QString(" WHERE %1 > 0 ").arg("track_count");

    QList<CrateSummary> rv = QList<CrateSummary>();
    rv.reserve(trackIds.length());

    CrateSummarySelectResult results = m_pTrackCollection->crates().selectCratesWithTrackCount(trackIds);

    CrateSummary crate;
    while (results.populateNext(&crate)) {
        if (crate.getTrackCount() == 0)
            continue;
        rv.append(crate);
    }
    emit(result(query, trackIds.length(), rv));
}

QThread* WCrateList::s_worker_thread = nullptr; //QThread();

WCrateList::WCrateList(const char* group, UserSettingsPointer pConfig,
                       const TrackCollection* pCollection,  QWidget* pParent)
        : QListWidget(pParent), WBaseWidget(pParent),
          m_pGroup(group),
          m_pConfig(pConfig),
          m_pCollection(pCollection) {

    // FIXME(poelzi): does it make sense to allow drops of certain stuff ???
    setAcceptDrops(false);

    // initilize the shared worker thread
    if (s_worker_thread == nullptr) {
        // initialize the worker thread shared between all widgets on first use
        s_worker_thread = new QThread(nullptr);
        s_worker_thread->setObjectName("wcratelistworker");
        s_worker_thread->start(QThread::LowPriority);
        qDebug() << "worker thread initialized";
    }
    m_pWorker = new WCrateListWorker(this, pCollection);
    m_pWorker->moveToThread(s_worker_thread);

    connect(m_pWorker, SIGNAL(result(int, uint, QList<CrateSummary>)),
            this, SLOT(receiveResults(int, uint, QList<CrateSummary>)));
    connect(this, SIGNAL(query(int, QList<TrackId>)),
            m_pWorker, SLOT(query(int, QList<TrackId>)));
}

// generates unique id's for all worker requests
int WCrateList::s_last_id = 0;

int WCrateList::generateId() {
    return ++s_last_id;
}

void WCrateList::receiveResults(int query, uint total, QList<CrateSummary> crates) {
    // we are only interested in results we requested last
    if (query != m_id) {
        return;
    }

    foreach (CrateSummary crate, crates) {
        if (crate.getTrackCount() == total) {
            WCrateItem *item = new WCrateItem(crate.getName(), this);
            addItem(item);
        } else {
            WCrateItemSome *item = new WCrateItemSome(crate.getName(), this);
            addItem(item);
        }

    }
}

void WCrateList::slotTrackLoaded(TrackPointer track) {
    clear();
    if (track == nullptr) {
        return;
    }
    m_id = generateId();

    QList<TrackId> lst = QList<TrackId>();
    lst.append(track->getId());
    emit query(m_id, lst);
}

void WCrateList::slotTrackSelection(QList<TrackPointer> tracks) {
    clear();

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

void WCrateList::setup(const QDomNode& node, const SkinContext& context) {
    Q_UNUSED(node);
    Q_UNUSED(context);
}

void WCrateList::mouseMoveEvent(QMouseEvent *event) {
    if ((event->buttons() & Qt::LeftButton) && m_pCurrentTrack) {
        DragAndDropHelper::dragTrack(m_pCurrentTrack, this, m_pGroup);
    }
}

void WCrateList::dragEnterEvent(QDragEnterEvent *event) {
    event->ignore();
}

void WCrateList::dropEvent(QDropEvent *event) {
    event->ignore();
}
