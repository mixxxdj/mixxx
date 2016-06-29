// libraryfeature.cpp
// Created 8/17/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>
#include <QIcon>
#include <QModelIndex>
#include <QVariant>
#include <QAbstractItemModel>
#include <QUrl>
#include <QDesktopServices>
#include <QTreeView>

#include "library/libraryfeature.h"
#include "widget/wbaselibrary.h"
#include "widget/wlibrarysidebar.h"

// KEEP THIS cpp file to tell scons that moc should be called on the class!!!
// The reason for this is that LibraryFeature uses slots/signals and for this
// to work the code has to be precompiled by moc
LibraryFeature::LibraryFeature(UserSettingsPointer pConfig, 
                               Library* pLibrary,
                               QObject* parent)
        : QObject(parent),
          m_pConfig(pConfig),
          m_pLibrary(pLibrary),
          m_featureFocus(-1) {
}

LibraryFeature::~LibraryFeature() {
    
}

void LibraryFeature::bindSidebarWidget(WBaseLibrary *pSidebarWidget, 
                                       KeyboardEventFilter* pKeyboard) {    
    QWidget* pSidebar = createSidebarWidget(pKeyboard);
    if (pSidebar == nullptr) {
        return;
    }
    
    pSidebar->setParent(pSidebarWidget);
    pSidebarWidget->registerView(getViewName(), pSidebar);
}

QWidget *LibraryFeature::createSidebarWidget(KeyboardEventFilter* pKeyboard) {
    //qDebug() << "LibraryFeature::bindSidebarWidget";
    TreeItemModel* pTreeModel = getChildModel();
    WLibrarySidebar* pSidebar = new WLibrarySidebar(nullptr);
    pSidebar->installEventFilter(pKeyboard);
    pSidebar->setModel(pTreeModel);
    
    connect(pSidebar, SIGNAL(clicked(const QModelIndex&)),
            this, SLOT(activateChild(const QModelIndex&)));
    connect(pSidebar, SIGNAL(doubleClicked(const QModelIndex&)),
            this, SLOT(onLazyChildExpandation(const QModelIndex&)));
    connect(pSidebar, SIGNAL(rightClicked(const QPoint&, const QModelIndex&)),
            this, SLOT(onRightClickChild(const QPoint&, const QModelIndex&)));
    connect(pSidebar, SIGNAL(expanded(const QModelIndex&)),
            this, SLOT(onLazyChildExpandation(const QModelIndex&)));
    
    return pSidebar;
}

void LibraryFeature::setFeatureFocus(int focus) {
    m_featureFocus = focus;
}

QStringList LibraryFeature::getPlaylistFiles(QFileDialog::FileMode mode) {
    QString lastPlaylistDirectory = m_pConfig->getValueString(
            ConfigKey("[Library]", "LastImportExportPlaylistDirectory"),
            QDesktopServices::storageLocation(QDesktopServices::MusicLocation));

    QFileDialog dialogg(NULL,
                        tr("Import Playlist"),
                        lastPlaylistDirectory,
                        tr("Playlist Files (*.m3u *.m3u8 *.pls *.csv)"));
    dialogg.setAcceptMode(QFileDialog::AcceptOpen);
    dialogg.setFileMode(mode);
    dialogg.setModal(true);

    // If the user refuses return
    if (!dialogg.exec()) {
        return QStringList();
    }
    return dialogg.selectedFiles();
}
