#include "library/export/coverartcopywizard.h"

bool CoverArtCopyWizard::copyCoverArt() {
    m_worker.reset(new CoverArtCopyWorker(m_coverArtImage, m_coverArtAbsolutePath));
    m_worker->run();
    return m_worker->isCoverUpdated();
}
