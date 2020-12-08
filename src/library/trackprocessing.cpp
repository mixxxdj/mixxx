#include "library/trackprocessing.h"

#include <QThread>

#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "moc_trackprocessing.cpp"
#include "util/logger.h"

namespace mixxx {

namespace {

const Logger kLogger("ModalTrackBatchProcessor");

} // anonymous namespace

int ModalTrackBatchProcessor::processTracks(
        const QString& progressLabelText,
        TrackCollectionManager* pTrackCollectionManager,
        TrackPointerIterator* pTrackPointerIterator) {
    DEBUG_ASSERT(pTrackCollectionManager);
    DEBUG_ASSERT(pTrackPointerIterator);
    DEBUG_ASSERT(QThread::currentThread() ==
            pTrackCollectionManager->thread());
    int finishedTrackCount = 0;
    // The total count is initialized with the remaining count
    // before starting the iteration. If this value is unknown
    // we use 0 as the default until an estimation is available
    // (see update below).
    int estimatedTotalCount =
            pTrackPointerIterator->estimateItemsRemaining().value_or(0);
    m_bAborted = false;
    TaskMonitor taskMonitor(
            progressLabelText,
            m_minimumProgressDuration,
            this);
    taskMonitor.registerTask(this);
    while (auto nextTrackPointer = pTrackPointerIterator->nextItem()) {
        const auto pTrack = *nextTrackPointer;
        VERIFY_OR_DEBUG_ASSERT(pTrack) {
            kLogger.warning()
                    << progressLabelText
                    << "failed to load next track for processing";
            continue;
        }
        if (m_bAborted) {
            kLogger.info()
                    << "Aborting"
                    << progressLabelText
                    << "after processing"
                    << finishedTrackCount
                    << "of"
                    << estimatedTotalCount
                    << "track(s)";
            return finishedTrackCount;
        }
        switch (doProcessNextTrack(pTrack)) {
        case ProcessNextTrackResult::AbortProcessing:
            kLogger.info()
                    << progressLabelText
                    << "aborted while processing"
                    << finishedTrackCount + 1
                    << "of"
                    << estimatedTotalCount
                    << "track(s)";
            return finishedTrackCount;
        case ProcessNextTrackResult::ContinueProcessing:
            break;
        case ProcessNextTrackResult::SaveTrackAndContinueProcessing:
            pTrackCollectionManager->saveTrack(pTrack);
            break;
        }
        ++finishedTrackCount;
        if (finishedTrackCount > estimatedTotalCount) {
            // Update the total count which cannot be less than the
            // number of already finished items plus the estimated number
            // of remaining items.
            auto estimatedRemainingCount =
                    pTrackPointerIterator->estimateItemsRemaining().value_or(0);
            estimatedTotalCount = finishedTrackCount + estimatedRemainingCount;
        }
        DEBUG_ASSERT(finishedTrackCount <= estimatedTotalCount);
        taskMonitor.reportTaskProgress(
                this,
                kPercentageOfCompletionMin +
                        (kPercentageOfCompletionMax -
                                kPercentageOfCompletionMin) *
                                finishedTrackCount /
                                static_cast<PercentageOfCompletion>(
                                        estimatedTotalCount));
    }
    return finishedTrackCount;
}

ModalTrackBatchOperationProcessor::ModalTrackBatchOperationProcessor(
        const TrackPointerOperation* pTrackPointerOperation,
        Mode mode,
        Duration progressGracePeriod,
        QObject* parent)
        : ModalTrackBatchProcessor(progressGracePeriod, parent),
          m_pTrackPointerOperation(pTrackPointerOperation),
          m_mode(mode) {
    DEBUG_ASSERT(m_pTrackPointerOperation);
}

ModalTrackBatchProcessor::ProcessNextTrackResult
ModalTrackBatchOperationProcessor::doProcessNextTrack(
        const TrackPointer& pTrack) {
    m_pTrackPointerOperation->apply(pTrack);
    switch (m_mode) {
    case Mode::Apply:
        return ProcessNextTrackResult::ContinueProcessing;
    case Mode::ApplyAndSave:
        return ProcessNextTrackResult::SaveTrackAndContinueProcessing;
    }
    DEBUG_ASSERT(!"unreachable");
    return ProcessNextTrackResult::AbortProcessing;
}

} // namespace mixxx
