#include "aoide/activecollectionagent.h"

#include <QMessageBox>

#include "aoide/subsystem.h"
#include "aoide/web/createcollectiontask.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("aoide ActiveCollectionAgent");

} // anonymous namespace

namespace aoide {

ActiveCollectionAgent::ActiveCollectionAgent(
        Subsystem* subsystem,
        TrackCollectionManager* trackCollectionManager,
        QObject* parent)
        : QObject(parent),
          m_subsystem(subsystem),
          m_trackCollectionManager(trackCollectionManager) {
    connect(m_subsystem.data(),
            &Subsystem::collectionsChanged,
            this,
            &ActiveCollectionAgent::onCollectionsChanged);
}

void ActiveCollectionAgent::onCollectionsChanged(
        int flags) {
    auto* const subsystem = m_subsystem.data();
    VERIFY_OR_DEBUG_ASSERT(subsystem) {
        return;
    }
    if (subsystem->activeCollection()) {
        if (flags & Subsystem::CollectionsChangedFlags::ACTIVE_COLLECTION) {
            kLogger.info()
                    << "Active collection"
                    << subsystem->activeCollection();
        }
        return;
    }
    // Select the first collection that contains local files,
    // preferably a collection that uses virtual file paths.
    for (const auto& collection : subsystem->allCollections()) {
        if (collection.body().mediaSourceConfig().sourcePath().pathKind() ==
                json::kSourcePathKindVirtualFilePath) {
            subsystem->selectActiveCollection(
                    subsystem->allCollections().front().header().uid());
            DEBUG_ASSERT(subsystem->activeCollection());
            return;
        }
    }
    for (const auto& collection : subsystem->allCollections()) {
        if (collection.body().mediaSourceConfig().sourcePath().pathKind() ==
                json::kSourcePathKindFileUrl) {
            subsystem->selectActiveCollection(
                    subsystem->allCollections().front().header().uid());
            DEBUG_ASSERT(subsystem->activeCollection());
            return;
        }
    }
    DEBUG_ASSERT(!subsystem->activeCollection());
    auto* const trackCollectionManager = m_trackCollectionManager.data();
    VERIFY_OR_DEBUG_ASSERT(trackCollectionManager) {
        return;
    }
    const auto rootDirs = trackCollectionManager->internalCollection()->loadRootDirs(true);
    // Ensure that one collection is active. If no collections
    // exist then create a default one.
    json::Collection collection;
    if (rootDirs.size() == 1) {
        auto rootDir = rootDirs.first().location();
        const auto rootUrl = QUrl::fromLocalFile(rootDir);
        DEBUG_ASSERT(rootUrl.isValid());
        DEBUG_ASSERT(rootUrl.toLocalFile() == rootDir);
        const auto baseUrl = mixxx::EncodedUrl::fromQUrlWithTrailingSlash(rootUrl);
        collection.setMediaSourceConfig(json::MediaSourceConfig::forLocalFiles(baseUrl));
    } else {
        VERIFY_OR_DEBUG_ASSERT(rootDirs.size() <= 1) {
            // TODO: How to handle this case? Create multiple collections,
            // one for each music folder? But then we would need multiple
            // active collections instead of only a single collection.
            auto msgBox = QMessageBox(
                    QMessageBox::Warning,
                    tr("Create aoide Collection"),
                    tr("Multiple music folders per collection are not supported."),
                    QMessageBox::Ok);
            msgBox.exec();
            return;
        }
        DEBUG_ASSERT(rootDirs.isEmpty());
        auto msgBox = QMessageBox(
                QMessageBox::Warning,
                tr("Create aoide Collection"),
                tr("Cannot create new collection without a music folder."),
                QMessageBox::Ok);
        msgBox.exec();
        return;
    }
    collection.setTitle("Mixxx Collection");
    collection.setKind("mixxx.org");
    collection.setNotes("Created by Mixxx");
    auto msgBox = QMessageBox(
            QMessageBox::Information,
            tr("Creating aoide Collection"),
            collection.toPrettyString(),
            QMessageBox::Ok | QMessageBox::Cancel);
    if (msgBox.exec() != QMessageBox::Ok) {
        return;
    }
    auto* task = subsystem->createCollection(collection);
    connect(
            task,
            &CreateCollectionTask::succeeded,
            this,
            [this, subsystem, task](json::CollectionEntity entity) {
                Q_UNUSED(entity);
                DEBUG_ASSERT(task == qobject_cast<CreateCollectionTask*>(sender()));
                VERIFY_OR_DEBUG_ASSERT(task) {
                    return;
                }
                task->deleteLater();
                subsystem->invokeRefreshCollections();
            },
            Qt::UniqueConnection);
    task->invokeStart();
}

} // namespace aoide
