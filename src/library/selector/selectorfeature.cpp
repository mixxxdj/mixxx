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
#include "soundsourceproxy.h"

const QString SelectorFeature::m_sSelectorViewName = QString("Selector");

SelectorFeature::SelectorFeature(QObject* parent,
                               ConfigObject<ConfigValue>* pConfig,
                               TrackCollection* pTrackCollection)
        : LibraryFeature(parent),
          m_pConfig(pConfig),
          m_pTrackCollection(pTrackCollection),
          m_pSelectorView(NULL) {
}

SelectorFeature::~SelectorFeature() {
}

QVariant SelectorFeature::title() {
    return tr("Selector");
}

QIcon SelectorFeature::getIcon() {
    return QIcon(":/images/library/ic_library_selector.png");
}

void SelectorFeature::bindWidget(WLibrary* libraryWidget,
                                MixxxKeyboard* keyboard) {
    m_pSelectorView = new DlgSelector(libraryWidget,
                                      m_pConfig,
                                      m_pTrackCollection,
                                      keyboard);
    libraryWidget->registerView(m_sSelectorViewName, m_pSelectorView);

    connect(m_pSelectorView, SIGNAL(loadTrack(TrackPointer)),
            this, SIGNAL(loadTrack(TrackPointer)));
    connect(m_pSelectorView, SIGNAL(loadTrackToPlayer(TrackPointer, QString)),
            this, SIGNAL(loadTrackToPlayer(TrackPointer, QString)));
}

TreeItemModel* SelectorFeature::getChildModel() {
    return &m_childModel;
}

void SelectorFeature::activate() {
    //qDebug() << "SelectorFeature::activate()";
    emit(switchToView(m_sSelectorViewName));
}

void SelectorFeature::setSeedTrack(TrackPointer pTrack) {
    m_pSelectorView->setSeedTrack(pTrack);
}

bool SelectorFeature::dropAccept(QList<QUrl> urls, QWidget *pSource) {
    TrackDAO &trackDao = m_pTrackCollection->getTrackDAO();

    //If a track is dropped onto the selector name, but the track isn't in the library,
    //then add the track to the library before using it as a seed.
    QList<QFileInfo> files;
    foreach (QUrl url, urls) {
        QFileInfo file = url.toLocalFile();
        if (SoundSourceProxy::isFilenameSupported(file.fileName())) {
            files.append(file);
        }
    }
    QList<int> trackIds;
    if (pSource) {
        trackIds = m_pTrackCollection->getTrackDAO().getTrackIds(files);
    } else {
        trackIds = trackDao.addTracks(files, true);
    }

    // get rid of any tracks not added
    for (int trackId = 0; trackId < trackIds.size(); trackId++) {
        if (trackIds.at(trackId) < 0) {
            trackIds.removeAt(trackId--);
        }
    }

//    qDebug() << "Track ID: " << trackIds.first();

    m_pSelectorView->setSeedTrack(trackDao.getTrack(trackIds.first()));

    return true;
}

bool SelectorFeature::dragMoveAccept(QUrl url) {
    QFileInfo file(url.toLocalFile());
    return SoundSourceProxy::isFilenameSupported(file.fileName());
}
