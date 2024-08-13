#include <QDebug>
#include <QtGlobal>

#import <AVFAudio/AVFAudio.h>

namespace mixxx {

void initializeAVAudioSession() {
    AVAudioSession* session = AVAudioSession.sharedInstance;
    AVAudioSessionCategory category = AVAudioSessionCategoryPlayback;
    AVAudioSessionMode mode = AVAudioSessionModeDefault;
    AVAudioSessionCategoryOptions options =
            AVAudioSessionCategoryOptionMixWithOthers;

    NSError* error = nil;
    [session setCategory:category mode:mode options:options error:&error];
    if (error != nil) {
        qWarning() << "Could not initialize AVAudioSession:"
                   << error.localizedDescription;
    }

    [session setActive:true error:&error];
    if (error != nil) {
        qWarning() << "Could not activate AVAudioSession:"
                   << error.localizedDescription;
    }
}

}; // namespace mixxx
