/// Utilities for executing operations on a selection of multiple
/// tracks while displaying an application modal progress dialog.

#pragma once

#include <QObject>

#include "track/trackiterator.h"
#include "util/duration.h"
#include "util/taskmonitor.h"

class TrackCollectionManager;

namespace mixxx {

/// Processes a selection of tracks in the foreground.
///
/// Shows a modal progress dialog while processing. This dialog
/// only appears if processing takes longer than the given grace
/// period. This avoids that an open context menu gets closed
/// while processing only a few tracks.
class ModalTrackBatchProcessor
        : public Task {
    Q_OBJECT

  public:
    virtual ~ModalTrackBatchProcessor() = default;

    /// Subsequently load and process a list of tracks.
    ///
    /// Returns the number of processed tracks.
    int processTracks(
            const QString& progressLabelText,
            TrackCollectionManager* pTrackCollectionManager,
            TrackPointerIterator* pTrackPointerIterator);

  protected:
    explicit ModalTrackBatchProcessor(
            Duration minimumProgressDuration =
                    TaskMonitor::kDefaultMinimumProgressDuration,
            QObject* parent = nullptr)
            : Task(parent),
              m_minimumProgressDuration(minimumProgressDuration) {
    }

    enum class ProcessNextTrackResult {
        AbortProcessing,
        ContinueProcessing,
        SaveTrackAndContinueProcessing,
    };

  private slots:
    void slotAbortTask() override {
        m_bAborted = true;
    }

  private:
    ModalTrackBatchProcessor(const ModalTrackBatchProcessor&) = delete;
    ModalTrackBatchProcessor(ModalTrackBatchProcessor&&) = delete;

    /// Template method to process the next available track.
    virtual ProcessNextTrackResult doProcessNextTrack(
            const TrackPointer& pTrack) = 0;

    const Duration m_minimumProgressDuration;

    bool m_bAborted;
};

/// Apply an operation on individual track pointers.
//
/// The operation is supposed to be applied subsequently to multiple
/// tracks in a batch. The order of tracks should not matter. The
/// `const` classifier in the function signature indicates that all
/// internal state mutations should not affect the actual processing.
//
/// Derived classes may store results of the last invocation or any
/// kind of internal state (i.e. for caching) in a mutable member if
/// needed.
class TrackPointerOperation {
  public:
    virtual ~TrackPointerOperation() = default;

    /// Non-overridable public method.
    ///
    /// Future extension: Might contain pre/post-processing actions.
    void apply(
            const TrackPointer& pTrack) const {
        doApply(pTrack);
    }

  private:
    /// Overridable template method that is supposed to handle or
    /// modify the given track object.
    virtual void doApply(
            const TrackPointer& pTrack) const = 0;
};

class ModalTrackBatchOperationProcessor
        : public ModalTrackBatchProcessor {
    Q_OBJECT

  public:
    enum class Mode {
        /// Apply the operation. Modified track objects will
        /// only be saved implicitly when their pointer goes
        /// out of scope.
        Apply,

        /// Explicitly save modified track objects after
        /// applying the operation.
        ApplyAndSave,
    };

    /// Construct a new processing instance.
    ///
    /// The pointer to the actual track operation must be valid
    /// for the whole lifetime of the created instance.
    ModalTrackBatchOperationProcessor(
            const TrackPointerOperation* pTrackPointerOperation,
            Mode mode,
            Duration minimumProgressDuration =
                    TaskMonitor::kDefaultMinimumProgressDuration,
            QObject* parent = nullptr);
    ~ModalTrackBatchOperationProcessor() override = default;

  private:
    ProcessNextTrackResult doProcessNextTrack(
            const TrackPointer& pTrack) override;

    const TrackPointerOperation* const m_pTrackPointerOperation;
    const Mode m_mode;
};

} // namespace mixxx
