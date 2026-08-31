#include "util/filepromisedrophelper.h"

#ifdef Q_OS_MACOS

#import <Cocoa/Cocoa.h>
#import <objc/runtime.h>

#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>

#include "util/logger.h"

namespace {
mixxx::Logger kLogger("MacDropHelper");

mixxx::mac::FilePromiseDropHelper::DropCallback s_dropCallback = nullptr;
IMP s_originalPerformDragOperation = nullptr;
IMP s_originalDraggingEntered = nullptr;

QString getCloudImportsCacheDirectory() {
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation) +
            QStringLiteral("/CloudImports");
}
} // namespace

@interface QNSViewDropHook : NSObject
- (NSDragOperation)swizzled_draggingEntered:(id<NSDraggingInfo>)sender;
- (BOOL)swizzled_performDragOperation:(id<NSDraggingInfo>)sender;
@end

@implementation QNSViewDropHook

- (NSDragOperation)swizzled_draggingEntered:(id<NSDraggingInfo>)sender {
    NSPasteboard* pboard = [sender draggingPasteboard];

    // Check if the pasteboard contains file promises (e.g. un-hydrated cloud storage files)
    NSArray* classes = @[[NSFilePromiseReceiver class]];
    if ([pboard canReadObjectForClasses:classes options:@{}]) {
        return NSDragOperationCopy;
    }

    // Fall back to original Qt Cocoa implementation for standard local files
    if (s_originalDraggingEntered) {
        typedef NSDragOperation (*OrigFunc)(id, SEL, id<NSDraggingInfo>);
        return ((OrigFunc)s_originalDraggingEntered)(self, _cmd, sender);
    }
    return NSDragOperationNone;
}

- (BOOL)swizzled_performDragOperation:(id<NSDraggingInfo>)sender {
    NSPasteboard* pboard = [sender draggingPasteboard];
    NSArray* classes = @[[NSFilePromiseReceiver class]];
    NSDictionary* options = @{};

    if ([pboard canReadObjectForClasses:classes options:options]) {
        NSArray* promiseReceivers = [pboard readObjectsForClasses:classes options:options];
        if (promiseReceivers.count > 0) {
            NSPoint windowLocation = [sender draggingLocation];
            NSWindow* window = [sender draggingDestinationWindow];
            NSPoint screenLocation = [window convertPointToScreen:windowLocation];

            // Convert Cocoa screen coordinates (origin at bottom-left) to Qt global coordinates (origin at top-left)
            NSScreen* primaryScreen = [NSScreen screens].firstObject;
            CGFloat screenHeight = primaryScreen ? primaryScreen.frame.size.height : 0.0;
            QPoint globalDropPoint(static_cast<int>(screenLocation.x),
                    static_cast<int>(screenHeight - screenLocation.y));

            // Target staging directory for cloud file downloads
            QString cachePath = getCloudImportsCacheDirectory();
            QDir().mkpath(cachePath);
            NSURL* destinationURL = [NSURL fileURLWithPath:cachePath.toNSString() isDirectory:YES];

            NSOperationQueue* workQueue = [[NSOperationQueue alloc] init];
            workQueue.name = @"org.mixxx.filepromise.receiver";
            workQueue.maxConcurrentOperationCount = 4;

            __block NSUInteger remainingFiles = promiseReceivers.count;
            NSMutableArray<NSURL*>* downloadedUrls = [NSMutableArray array];

            for (NSFilePromiseReceiver* receiver in promiseReceivers) {
                [receiver receivePromisedFilesAtDestination:destinationURL
                                                   options:@{}
                                            operationQueue:workQueue
                                                    reader:^(NSURL* fileURL, NSError* errorOrNil) {
                    dispatch_async(dispatch_get_main_queue(), ^{
                        if (errorOrNil) {
                            kLogger.warning()
                                    << "Failed to fulfill cloud file promise:"
                                    << QString::fromCFString((__bridge CFStringRef)
                                                    errorOrNil.localizedDescription);
                        } else if (fileURL) {
                            [downloadedUrls addObject:fileURL];
                        }

                        remainingFiles--;
                        if (remainingFiles == 0 && downloadedUrls.count > 0 && s_dropCallback) {
                            QList<QUrl> qtUrls;
                            for (NSURL* url in downloadedUrls) {
                                qtUrls.append(QUrl::fromLocalFile(
                                        QString::fromCFString((__bridge CFStringRef)url.path)));
                            }
                            s_dropCallback(qtUrls, globalDropPoint);
                        }
                    });
                }];
            }
            return YES; // Acknowledge drop immediately to prevent OS snapback
        }
    }

    // Fall back to original Qt handler for standard drops
    if (s_originalPerformDragOperation) {
        typedef BOOL (*OrigFunc)(id, SEL, id<NSDraggingInfo>);
        return ((OrigFunc)s_originalPerformDragOperation)(self, _cmd, sender);
    }
    return NO;
}

@end

namespace mixxx {
namespace mac {

void FilePromiseDropHelper::initialize(DropCallback callback) {
    s_dropCallback = std::move(callback);

    // In Qt 6 Cocoa platform plugin, top-level window NSViews are of class QNSView
    Class qnsViewClass = objc_getClass("QNSView");
    if (!qnsViewClass) {
        kLogger.warning() << "Could not locate QNSView class for drag-and-drop swizzling.";
        return;
    }

    // Swizzle performDragOperation:
    SEL performSel = @selector(performDragOperation:);
    Method origPerformMethod = class_getInstanceMethod(qnsViewClass, performSel);
    Method swizzledPerformMethod = class_getInstanceMethod(
            [QNSViewDropHook class], @selector(swizzled_performDragOperation:));

    if (origPerformMethod && swizzledPerformMethod) {
        s_originalPerformDragOperation = method_getImplementation(origPerformMethod);
        method_exchangeImplementations(origPerformMethod, swizzledPerformMethod);
    }

    // Swizzle draggingEntered:
    SEL enterSel = @selector(draggingEntered:);
    Method origEnterMethod = class_getInstanceMethod(qnsViewClass, enterSel);
    Method swizzledEnterMethod = class_getInstanceMethod(
            [QNSViewDropHook class], @selector(swizzled_draggingEntered:));

    if (origEnterMethod && swizzledEnterMethod) {
        s_originalDraggingEntered = method_getImplementation(origEnterMethod);
        method_exchangeImplementations(origEnterMethod, swizzledEnterMethod);
    }

    kLogger.info() << "Successfully registered NSFilePromiseReceiver hook on QNSView.";
}

void FilePromiseDropHelper::purgeStaleCache() {
    QString cachePath = getCloudImportsCacheDirectory();
    QDir cacheDir(cachePath);
    if (cacheDir.exists()) {
        cacheDir.removeRecursively();
    }
}

} // namespace mac
} // namespace mixxx

#endif // Q_OS_MACOS
