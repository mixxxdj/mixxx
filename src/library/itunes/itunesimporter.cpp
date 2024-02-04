#include "library/itunes/itunesfeature.h"
#include "library/itunes/itunesxmlimporter.h"

ITunesImporter::ITunesImporter(ITunesFeature* pParentFeature)
        : m_pParentFeature(pParentFeature) {
}

bool ITunesImporter::canceled() const {
    // The parent feature may be null during testing
    if (m_pParentFeature) {
        return m_pParentFeature->isImportCanceled();
    }
    return false;
}
