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
#include <QWidget>
#include <QWindow>
#include <QtGlobal>
#include <variant>

namespace {

std::variant<NSView* _Nonnull, QString> createNativeUI(
        AudioUnitManagerPointer pManager, CGSize size) {
    QString name = pManager->getName();
    qDebug() << "Loading UI of Audio Unit" << name << "with width" << size.width
             << "and height" << size.height;

    const int TIMEOUT_MS = 2000;
    if (!pManager->waitForAudioUnit(TIMEOUT_MS)) {
        return "Instantiating the UI took more than " +
                QString::number(TIMEOUT_MS) +
                "ms, try closing and reopening the UI!";
    }

    AudioUnit _Nullable audioUnit = pManager->getAudioUnit();

    if (audioUnit == nil) {
        return "Cannot create UI of Audio Unit " + name +
                " without an instance";
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
        return "No Cocoa UI available for Audio Unit " + name;
    }

    uint32_t numberOfClasses =
            (dataSize - sizeof(CFURLRef)) / sizeof(CFStringRef);
    if (numberOfClasses == 0) {
        return "No view classes available for Audio Unit " + name;
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
        return "Could not fetch Cocoa UI for Audio Unit " + name;
    }

    NSURL* viewBundleLocation =
            (__bridge NSURL*)cocoaViewInfo.get()->mCocoaAUViewBundleLocation;
    if (viewBundleLocation == nil) {
        return "Cannot create UI of Audio Unit " + name +
                " without view bundle path";
    }

    // We only use the first view as in the Cocoa AU host example linked earlier
    NSString* factoryClassName =
            (__bridge NSString*)cocoaViewInfo.get()->mCocoaAUViewClass[0];
    ;
    if (factoryClassName == nil) {
        return "Cannot create UI of Audio Unit " + name +
                " without factory class name";
    }

    NSBundle* viewBundle = [NSBundle bundleWithURL:viewBundleLocation];
    if (viewBundle == nil) {
        return "Could not load view bundle of Audio Unit " + name;
    }

    Class factoryClass = [viewBundle classNamed:factoryClassName];
    if (factoryClass == nil) {
        return "Could not load view factory class from bundle of Audio Unit " +
                name;
    }

    id<AUCocoaUIBase> factoryInstance = [[factoryClass alloc] init];
    if (factoryInstance == nil) {
        return "Could not instantiate factory for view of Audio Unit " + name;
    }

    NSView* view = [factoryInstance uiViewForAudioUnit:audioUnit withSize:size];
    if (view == nil) {
        return "Could not instantiate view of Audio Unit " + name;
    }

    qDebug() << "Successfully loaded UI of Audio Unit" << name;
    return view;
}

}; // anonymous namespace

DlgAudioUnit::DlgAudioUnit(AudioUnitManagerPointer pManager) {
    QString name = pManager->getName();

    setWindowTitle(name);

    auto result = createNativeUI(pManager, size().toCGSize());
    NSView* _Nullable audioUnitView = nil;

    if (std::holds_alternative<NSView* _Nonnull>(result)) {
        audioUnitView = std::get<NSView* _Nonnull>(result);

        // Automatically resize the dialog to fit the view when it resizes.
        // This is needed, because most plugins provide a fixed-size UI that our
        // window should adapt to.
        [audioUnitView setPostsFrameChangedNotifications:YES];
        m_resizeObserver = [[NSNotificationCenter defaultCenter]
                addObserverForName:NSViewFrameDidChangeNotification
                            object:audioUnitView
                             queue:[NSOperationQueue mainQueue]
                        usingBlock:^(NSNotification* notification) {
                            NSView* audioUnitView =
                                    (NSView*)notification.object;
                            CGSize cgSize = audioUnitView.frame.size;
                            QSize qSize(static_cast<int>(cgSize.width),
                                    static_cast<int>(cgSize.height));
                            resize(qSize);
                        }];
    } else {
        QString error = std::get<QString>(result);
        qWarning() << error;

        // Fall back to a generic UI if possible
        AudioUnit _Nullable audioUnit = pManager->getAudioUnit();
        if (audioUnit != nil) {
            AUGenericView* genericView =
                    [[AUGenericView alloc] initWithAudioUnit:audioUnit];
            genericView.showsExpertParameters = YES;
            audioUnitView = genericView;
        }
    }

    if (audioUnitView != nil) {
        // Wrap the audio unit view in QWindow/QWidget
        QWindow* wrapper =
                QWindow::fromWinId(reinterpret_cast<WId>(audioUnitView));
        QWidget* wrapperContainer = QWidget::createWindowContainer(wrapper);
        setCustomUI(wrapperContainer);
    }
}

DlgAudioUnit::~DlgAudioUnit() {
    [[NSNotificationCenter defaultCenter] removeObserver:m_resizeObserver];
}
