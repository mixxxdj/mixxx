#include "library/export/coverartcopywizard.h"

void CoverArtCopyWizard::copyCoverArt() {
    m_worker.reset(new CoverArtCopyWorker(m_coverArtImage, m_coverArtCopyPath));
    m_dialog.reset(new DlgCoverArtCopy(m_parent, m_pConfig, m_worker.data()));
    m_dialog->exec();
}
