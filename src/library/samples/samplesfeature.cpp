#include "library/samples/samplesfeature.h"

#include <QFileInfo>
#include <QStandardPaths>
#include <memory>

#include "library/browse/foldertreemodel.h"
#include "library/library.h"
#include "library/samples/dlgsamples.h"
#include "library/treeitem.h"
#include "moc_samplesfeature.cpp"
#include "widget/wlibrary.h"

namespace {

const QString kViewName = QStringLiteral("Samples");

QString samplesPath() {
    // Check for samples directory relative to the app binary
    QString appDir = QCoreApplication::applicationDirPath();
    // In development/build tree, samples are in <builddir>/res/samples/
    // In installed package, they're at <prefix>/share/mixxx/samples/
    QStringList candidates = {
        appDir + QStringLiteral("/res/samples/"),
        appDir + QStringLiteral("/../res/samples/"),
        appDir + QStringLiteral("/../share/mixxx/samples/"),
        QStringLiteral("res/samples/"),
    };
    for (const auto& path : candidates) {
        QFileInfo fi(path);
        if (fi.exists() && fi.isDir()) {
            return fi.absoluteFilePath();
        }
    }
    // Fallback
    return QStringLiteral("res/samples/");
}

} // anonymous namespace

SamplesFeature::SamplesFeature(Library* pLibrary,
        UserSettingsPointer pConfig)
        : LibraryFeature(pLibrary, pConfig, QStringLiteral("samples")),
          m_pSidebarModel(new FolderTreeModel(this)) {
    // The sidebar model will be populated when the view is bound
    // For now, create a simple root item
    std::unique_ptr<TreeItem> pRootItem = TreeItem::newRoot(this);
    m_pSidebarModel->setRootItem(std::move(pRootItem));
}

QVariant SamplesFeature::title() {
    return QVariant(tr("Samples"));
}

TreeItemModel* SamplesFeature::sidebarModel() const {
    return m_pSidebarModel;
}

void SamplesFeature::bindLibraryWidget(WLibrary* pLibraryWidget,
                                 KeyboardEventFilter* keyboard) {
    DlgSamples* pSamplesView = new DlgSamples(pLibraryWidget,
                                               m_pConfig,
                                               m_pLibrary,
                                               keyboard);
    pSamplesView->installEventFilter(keyboard);
    pLibraryWidget->registerView(kViewName, pSamplesView);
    connect(pSamplesView,
            &DlgSamples::loadTrack,
            this,
            &SamplesFeature::loadTrack);
    connect(pSamplesView,
            &DlgSamples::loadTrackToPlayer,
            this,
            &SamplesFeature::loadTrackToPlayer);
    connect(this,
            &SamplesFeature::refreshBrowseModel,
            pSamplesView,
            &DlgSamples::refreshBrowseModel);
    connect(this,
            &SamplesFeature::requestRestoreSearch,
            pSamplesView,
            &DlgSamples::slotRestoreSearch);
    connect(pSamplesView,
            &DlgSamples::restoreSearch,
            this,
            &SamplesFeature::restoreSearch);
    connect(pSamplesView,
            &DlgSamples::restoreModelState,
            this,
            &SamplesFeature::restoreModelState);
}

void SamplesFeature::activate() {
    emit refreshBrowseModel();
    emit switchToView(kViewName);
    emit requestRestoreSearch();
    emit enableCoverArtDisplay(false);
}