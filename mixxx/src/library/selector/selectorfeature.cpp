// selectorfeature.cpp
// Created 3/17/2012 by Keith Salisbury (keithsalisbury@gmail.com)

#include <QtDebug>

#include "library/selector/selectorfeature.h"
#include "library/librarytablemodel.h"
#include "library/trackcollection.h"
#include "dlgselector.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "mixxxkeyboard.h"

const QString SelectorFeature::m_sSelectorViewName = QString("Selector");

SelectorFeature::SelectorFeature(QObject* parent,
                               ConfigObject<ConfigValue>* pConfig,
                               TrackCollection* pTrackCollection)
        : LibraryFeature(parent),
          m_pConfig(pConfig),
          m_pTrackCollection(pTrackCollection) {
}

SelectorFeature::~SelectorFeature() {
    // TODO(XXX) delete these
    //delete m_pLibraryTableModel;
}

QVariant SelectorFeature::title() {
    return tr("Selector");
}

QIcon SelectorFeature::getIcon() {
    return QIcon(":/images/library/ic_library_selector.png");
}

void SelectorFeature::bindWidget(WLibrarySidebar* sidebarWidget,
                                WLibrary* libraryWidget,
                                MixxxKeyboard* keyboard) {
    DlgSelector* pSelectorView = new DlgSelector(libraryWidget,
                                              m_pConfig,
                                              m_pTrackCollection);
    connect(pSelectorView, SIGNAL(loadTrack(TrackPointer)),
            this, SIGNAL(loadTrack(TrackPointer)));
    connect(pSelectorView, SIGNAL(loadTrackToPlayer(TrackPointer, QString)),
            this, SIGNAL(loadTrackToPlayer(TrackPointer, QString)));

    pSelectorView->installEventFilter(keyboard);

    libraryWidget->registerView(m_sSelectorViewName, pSelectorView);
}

TreeItemModel* SelectorFeature::getChildModel() {
    return &m_childModel;
}

void SelectorFeature::activate() {
    //qDebug() << "SelectorFeature::activate()";
    emit(switchToView(m_sSelectorViewName));
}

void SelectorFeature::activateChild(const QModelIndex& index) {
}

void SelectorFeature::onRightClick(const QPoint& globalPos) {
}

void SelectorFeature::onRightClickChild(const QPoint& globalPos,
                                            QModelIndex index) {
}

bool SelectorFeature::dropAccept(QUrl url) {
    return false;
}

bool SelectorFeature::dropAcceptChild(const QModelIndex& index, QUrl url) {
    return false;
}

bool SelectorFeature::dragMoveAccept(QUrl url) {
    return false;
}

bool SelectorFeature::dragMoveAcceptChild(const QModelIndex& index,
                                              QUrl url) {
    return false;
}

void SelectorFeature::onLazyChildExpandation(const QModelIndex &index){
    //Nothing to do because the childmodel is not of lazy nature.
}

