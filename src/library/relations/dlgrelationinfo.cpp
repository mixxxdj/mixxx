#include "library/relations/dlgrelationinfo.h"

#include <QIcon>

#include "defs_urls.h"
#include "library/library.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "moc_dlgrelationinfo.cpp"

DlgRelationInfo::DlgRelationInfo(RelationPointer relation, Library* pLibrary)
        : QDialog(nullptr),
          m_pRelation(relation),
          m_pLibrary(pLibrary) {
    init();
}

void DlgRelationInfo::init() {
    setupUi(this);
    setWindowIcon(QIcon(MIXXX_ICON_PATH));

    // QDialog buttons
    connect(btnOK,
            &QPushButton::clicked,
            this,
            &DlgRelationInfo::slotOk);
    connect(btnCancel,
            &QPushButton::clicked,
            this,
            &DlgRelationInfo::slotCancel);

    if (!m_pRelation) {
        return;
    }

    if (!m_pLibrary) {
        return;
    }

    // Fetch Tracks metadata
    m_trackRecordA = m_pLibrary
                             ->trackCollectionManager()
                             ->getTrackById(m_pRelation->getTracks()[0])
                             ->getRecord();
    m_trackRecordB = m_pLibrary
                             ->trackCollectionManager()
                             ->getTrackById(m_pRelation->getTracks()[1])
                             ->getRecord();

    updateRelationMetadataFields();
}

void DlgRelationInfo::slotOk() {
    saveRelation();
    accept();
}

void DlgRelationInfo::slotCancel() {
    reject();
}

void DlgRelationInfo::updateRelationMetadataFields() {
    txtTitleA->setText(m_trackRecordA.getMetadata().getTrackInfo().getTitle());
    txtArtistA->setText(m_trackRecordA.getMetadata().getTrackInfo().getArtist());
    txtAlbumA->setText(m_trackRecordA.getMetadata().getAlbumInfo().getTitle());
    txtTitleB->setText(m_trackRecordB.getMetadata().getTrackInfo().getTitle());
    txtArtistB->setText(m_trackRecordB.getMetadata().getTrackInfo().getArtist());
    txtAlbumB->setText(m_trackRecordB.getMetadata().getAlbumInfo().getTitle());
    txtComment->setPlainText(m_pRelation->getComment());
    txtDateAdded->setText(m_pRelation->getDateAdded().date().toString());
}

void DlgRelationInfo::saveRelation() {
    if (!m_pRelation) {
        return;
    }

    m_pRelation->setComment(txtComment->toPlainText());

    m_pLibrary
            ->trackCollectionManager()
            ->internalCollection()
            ->getRelationDAO()
            .saveRelation(m_pRelation);
}
