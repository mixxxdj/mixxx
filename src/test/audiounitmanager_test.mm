#ifdef __APPLE__

#import <AVFAudio/AVFAudio.h>
#import <AudioToolbox/AudioToolbox.h>
#import <dispatch/dispatch.h>

#include <gtest/gtest.h>

#include <QDebug>
#include <QElapsedTimer>
#include <QList>

#include "effects/backends/audiounit/audiounitmanager.h"

class AudioUnitManagerTest : public ::testing::Test {};

// When AudioComponentInstantiate fails, dispatch_group_leave must still be
// called so that waitForAudioUnit returns promptly instead of blocking for
// the full timeout duration. A missing dispatch_group_leave on the error
// path caused threads to block for 2 seconds per failing AU, leading to
// dispatch thread pool exhaustion with 64+ plugins installed.
TEST_F(AudioUnitManagerTest, AsyncOutOfProcessCompletesWithinTimeout) {
    // Find any available AU effect to test with
    AudioComponentDescription desc = {
            .componentType = kAudioUnitType_Effect,
            .componentSubType = 0,
            .componentManufacturer = 0,
            .componentFlags = 0,
            .componentFlagsMask = 0,
    };

    auto manager =
            [AVAudioUnitComponentManager sharedAudioUnitComponentManager];
    auto components = [manager componentsMatchingDescription:desc];

    if ([components count] == 0) {
        GTEST_SKIP() << "No AU effect plugins available on this system";
    }

    // Pick the first available component
    AVAudioUnitComponent* component = components[0];

    AudioUnitManagerPointer pManager = AudioUnitManager::create(component);

    QElapsedTimer timer;
    timer.start();

    const int TIMEOUT_MS = 5000;
    bool result = pManager->waitForAudioUnit(TIMEOUT_MS);

    qint64 elapsed = timer.elapsed();

    // Whether the AU succeeds or fails, dispatch_group_leave should be
    // called in either case so we don't block for the full timeout.
    EXPECT_LT(elapsed, TIMEOUT_MS)
            << "waitForAudioUnit took " << elapsed
            << "ms, suggesting dispatch_group_leave was not called";

    if (result) {
        EXPECT_NE(pManager->getAudioUnit(), nullptr);
    }
}

// Test that multiple concurrent AU instantiations complete without
// exhausting the dispatch thread pool. This verifies the concurrency
// limiting fix.
TEST_F(AudioUnitManagerTest, ConcurrentInstantiationsDoNotExhaustThreadPool) {
    AudioComponentDescription desc = {
            .componentType = kAudioUnitType_Effect,
            .componentSubType = 0,
            .componentManufacturer = 0,
            .componentFlags = 0,
            .componentFlagsMask = 0,
    };

    auto manager =
            [AVAudioUnitComponentManager sharedAudioUnitComponentManager];
    auto components = [manager componentsMatchingDescription:desc];

    if ([components count] == 0) {
        GTEST_SKIP() << "No AU effect plugins available on this system";
    }

    // Instantiate up to 16 AUs concurrently (a subset of what a user
    // might have, but enough to verify the pattern works)
    const NSUInteger count = MIN((NSUInteger)16, [components count]);

    QList<AudioUnitManagerPointer> managers;

    QElapsedTimer timer;
    timer.start();

    for (NSUInteger i = 0; i < count; i++) {
        AVAudioUnitComponent* component = components[i];
        AudioUnitManagerPointer pManager = AudioUnitManager::create(component);
        managers.append(pManager);
    }

    // Wait for all to complete
    const int TIMEOUT_MS = 10000;
    for (auto& pManager : managers) {
        pManager->waitForAudioUnit(TIMEOUT_MS);
    }

    qint64 elapsed = timer.elapsed();

    // All instantiations should complete well within the timeout.
    EXPECT_LT(elapsed, TIMEOUT_MS) << "Concurrent AU instantiation took "
                                   << elapsed << "ms for " << count << " AUs";

    qDebug() << "Concurrently instantiated" << count << "AUs in" << elapsed
             << "ms";
}

// This simulates what happens inside AudioUnitManager when
// AudioComponentInstantiate fails. Before the fix, the error callback
// returned without calling dispatch_group_leave, causing
// dispatch_group_wait to block for the full timeout.
TEST_F(AudioUnitManagerTest, DispatchGroupLeaveMustBeCalledOnError) {
    // Simulate the BROKEN pattern (before fix):
    // dispatch_group_enter is called but dispatch_group_leave is NOT
    // called on the error path. This causes dispatch_group_wait to
    // block for the full timeout.
    {
        dispatch_group_t group = dispatch_group_create();
        dispatch_group_enter(group);

        // Simulate error callback that does NOT call dispatch_group_leave
        // (the old buggy behavior)
        dispatch_async(
                dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                ^{
                        // Error occurred! But we "forgot" to call
                        // dispatch_group_leave. This is the bug.
                });

        QElapsedTimer timer;
        timer.start();

        const int SHORT_TIMEOUT_MS = 200;
        long result = dispatch_group_wait(group,
                dispatch_time(DISPATCH_TIME_NOW, SHORT_TIMEOUT_MS * 1000000));

        qint64 elapsed = timer.elapsed();

        // Without dispatch_group_leave, the wait MUST time out
        EXPECT_NE(result, 0) << "dispatch_group_wait should have timed out "
                                "because dispatch_group_leave was never called";
        EXPECT_GE(elapsed, SHORT_TIMEOUT_MS - 50)
                << "Should have blocked for at least the timeout duration";

        // Clean up: we must leave the group to avoid a crash on destruction
        dispatch_group_leave(group);
    }

    // Simulate the FIXED pattern (after fix):
    // dispatch_group_leave IS called on the error path.
    // dispatch_group_wait returns immediately.
    {
        dispatch_group_t group = dispatch_group_create();
        dispatch_group_enter(group);

        // Simulate error callback that DOES call dispatch_group_leave
        // (the fixed behavior)
        dispatch_async(
                dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                ^{
                    // Error occurred, but we properly call leave.
                    dispatch_group_leave(group);
                });

        QElapsedTimer timer;
        timer.start();

        const int TIMEOUT_MS = 2000;
        long result = dispatch_group_wait(
                group, dispatch_time(DISPATCH_TIME_NOW, TIMEOUT_MS * 1000000));

        qint64 elapsed = timer.elapsed();

        // With dispatch_group_leave, the wait returns almost immediately
        EXPECT_EQ(result, 0) << "dispatch_group_wait should have succeeded "
                                "because dispatch_group_leave was called";
        EXPECT_LT(elapsed, 500)
                << "Should have returned almost immediately, not blocked "
                << "for " << elapsed << "ms";
    }
}

#endif // __APPLE__
