#import <AVFAudio/AVFAudio.h>
#import <AppKit/AppKit.h>
#import <AudioToolbox/AudioToolbox.h>
#import <AudioUnit/AUCocoaUIView.h>
#import <CoreAudioKit/CoreAudioKit.h>
#import <CoreAudioTypes/CoreAudioBaseTypes.h>
#import <CoreAudioTypes/CoreAudioTypes.h>

#include "effects/backends/audiounit/audiounitmanager.h"
#include "effects/backends/audiounit/dlgaudiounit.h"

#include "moc_dlgaudiounit.cpp"

#include <QString>
#include <QtGlobal>

namespace {

NSView* _Nullable createNativeUI(
        AudioUnitManagerPointer pManager, CGSize size, QString& outError) {
    QString name = pManager->getName();
    qDebug() << "Loading UI of Audio Unit" << name << "with width" << size.width
             << "and height" << size.height;

    AudioUnit _Nullable audioUnit = pManager->getAudioUnit();

    if (audioUnit == nil) {
        outError = "Cannot create UI of Audio Unit " + name +
                " without an instance";
        return nil;
    }

    // See
    // https://developer.apple.com/library/archive/samplecode/CocoaAUHost/Listings/Source_CAUHWindowController_mm.html

    uint32_t dataSize;

    OSStatus infoStatus = AudioUnitGetPropertyInfo(audioUnit,
            kAudioUnitProperty_CocoaUI,
            kAudioUnitScope_Global,
            0,
            &dataSize,
            nullptr);
    if (infoStatus != noErr) {
        outError = "No Cocoa UI available for Audio Unit " + name;
        return nil;
    }

    uint32_t numberOfClasses =
            (dataSize - sizeof(CFURLRef)) / sizeof(CFStringRef);
    if (numberOfClasses == 0) {
        outError = "No view classes available for Audio Unit " + name;
        return nil;
    }

    std::unique_ptr<AudioUnitCocoaViewInfo[]> cocoaViewInfo =
            std::make_unique<AudioUnitCocoaViewInfo[]>(numberOfClasses);

    OSStatus status = AudioUnitGetProperty(audioUnit,
            kAudioUnitProperty_CocoaUI,
            kAudioUnitScope_Global,
            0,
            cocoaViewInfo.get(),
            &dataSize);
    if (status != noErr) {
        outError = "Could not fetch Cocoa UI for Audio Unit " + name;
        return nil;
    }

    NSURL* viewBundleLocation =
            (__bridge NSURL*)cocoaViewInfo.get()->mCocoaAUViewBundleLocation;
    if (viewBundleLocation == nil) {
        outError = "Cannot create UI of Audio Unit " + name +
                " without view bundle path";
        return nil;
    }

    // We only use the first view as in the Cocoa AU host example linked earlier
    NSString* factoryClassName =
            (__bridge NSString*)cocoaViewInfo.get()->mCocoaAUViewClass[0];
    ;
    if (factoryClassName == nil) {
        outError = "Cannot create UI of Audio Unit " + name +
                " without factory class name";
        return nil;
    }

    NSBundle* viewBundle = [NSBundle bundleWithURL:viewBundleLocation];
    if (viewBundle == nil) {
        outError = "Could not load view bundle of Audio Unit " + name;
        return nil;
    }

    Class factoryClass = [viewBundle classNamed:factoryClassName];
    if (factoryClass == nil) {
        outError =
                "Could not load view factory class from bundle of Audio Unit " +
                name;
        return nil;
    }

    id<AUCocoaUIBase> factoryInstance = [[factoryClass alloc] init];
    if (factoryInstance == nil) {
        outError =
                "Could not instantiate factory for view of Audio Unit " + name;
        return nil;
    }

    NSView* view = [factoryInstance uiViewForAudioUnit:audioUnit withSize:size];
    if (view == nil) {
        outError = "Could not instantiate view of Audio Unit " + name;
        return nil;
    }

    qDebug() << "Successfully loaded UI of Audio Unit" << name;
    return view;
}

}; // anonymous namespace

DlgAudioUnit::DlgAudioUnit(AudioUnitManagerPointer pManager) {
    QString name = pManager->getName();

    setWindowTitle(name);

    // See
    // https://lists.qt-project.org/pipermail/interest/2014-January/010655.html
    // for why we need this slightly convoluted cast
    NSView* dialogView = (__bridge NSView*)reinterpret_cast<void*>(winId());

    // Style effect UI as a floating, but non-modal, HUD window
    NSWindow* dialogWindow = [dialogView window];
    [dialogWindow setStyleMask:NSWindowStyleMaskTitled |
            NSWindowStyleMaskClosable | NSWindowStyleMaskResizable |
            NSWindowStyleMaskUtilityWindow | NSWindowStyleMaskHUDWindow];
    [dialogWindow setLevel:NSFloatingWindowLevel];

    QString error = "Could not load UI of Audio Unit";
    NSView* audioUnitView = createNativeUI(pManager, size().toCGSize(), error);

    if (audioUnitView != nil) {
        [dialogView addSubview:audioUnitView];

        // Automatically resize the dialog to fit the view after layout
        [audioUnitView setPostsFrameChangedNotifications:YES];
        [[NSNotificationCenter defaultCenter]
                addObserverForName:NSViewFrameDidChangeNotification
                            object:audioUnitView
                             queue:[NSOperationQueue mainQueue]
                        usingBlock:^(NSNotification* notification) {
                            NSView* audioUnitView =
                                    (NSView*)notification.object;
                            NSWindow* dialogWindow = audioUnitView.window;
                            CGSize size = audioUnitView.frame.size;
                            [dialogWindow setContentSize:size];
                        }];
    } else {
        qWarning() << error;

        // Fall back to a generic UI if possible
        AudioUnit _Nullable audioUnit = pManager->getAudioUnit();
        if (audioUnit != nil) {
            AUGenericView* genericView =
                    [[AUGenericView alloc] initWithAudioUnit:audioUnit];
            genericView.showsExpertParameters = YES;
            [dialogView addSubview:genericView];
        }
    }
}
