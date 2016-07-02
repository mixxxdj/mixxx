// libraryfeature.cpp
// Created 8/17/2009 by RJ Ryan (rryan@mit.edu)

#include <QDebug>
#include <QAbstractItemModel>
#include <QDesktopServices>
#include <QIcon>
#include <QLabel>
#include <QModelIndex>
#include <QTreeView>
#include <QUrl>
#include <QVariant>
#include <QVBoxLayout>


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

QWidget *LibraryFeature::createSidebarWidget(KeyboardEventFilter* pKeyboard) {
    //qDebug() << "LibraryFeature::bindSidebarWidget";
    QFrame* pContainer = new QFrame(nullptr);
    pContainer->setContentsMargins(0,0,0,0);
    
    QVBoxLayout* pLayout = new QVBoxLayout(pContainer);
    pLayout->setContentsMargins(0,0,0,0);
    pLayout->setSpacing(0);
    pContainer->setLayout(pLayout);
    
    QLabel* pTitle = new QLabel(title().toString(), pContainer);
    pLayout->addWidget(pTitle);
    
    TreeItemModel* pTreeModel = getChildModel();
    WLibrarySidebar* pSidebar = new WLibrarySidebar(pContainer);
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
    pLayout->addWidget(pSidebar);
    
    return pContainer;
}

void LibraryFeature::setFeatureFocus(int focus) {
    m_featureFocus = focus;
}

QStringList LibraryFeature::getPlaylistFiles(QFileDialog::FileMode mode) {
    QString lastPlaylistDirectory = m_pConfig->getValueString(
            ConfigKey("[Library]", "LastImportExportPlaylistDirectory"),
            QDesktopServices::storageLocation(QDesktopServices::MusicLocation));

    QFileDialog dialog(nullptr,
                       tr("Import Playlist"),
                       lastPlaylistDirectory,
                       tr("Playlist Files (*.m3u *.m3u8 *.pls *.csv)"));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(mode);
    dialog.setModal(true);

    // If the user refuses return
    if (!dialog.exec()) {
        return QStringList();
    }
    return dialog.selectedFiles();
}
