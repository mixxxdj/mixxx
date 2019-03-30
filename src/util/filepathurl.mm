#include "util/filepathurl.h"

#include <QtCore/qsystemdetection.h>
#if defined(Q_OS_IOS)
#import <UIKit/UIKit.h>
#elif defined(Q_OS_OSX)
#import <Cocoa/Cocoa.h>
#endif

#import <Foundation/Foundation.h>


// Taken from src/platformsupport/clipboard/qmacmime.mm

QString stringFromNSString(const NSString *string) {
    if (!string)
        return QString();
   QString qstring;
   qstring.resize([string length]);
   [string getCharacters: reinterpret_cast<unichar*>(qstring.data()) range: NSMakeRange(0, [string length])];
   return qstring;
}


QUrl urlFromNSURL(const NSURL *url) {
    return QUrl(stringFromNSString([url absoluteString]));
}


QUrl ensureFilePathUrl(const QUrl& url) {
    const QByteArray &a = url.toEncoded();
    NSString *urlString = [[[NSString alloc] initWithBytesNoCopy:(void *)a.data() length:a.size()
                                             encoding:NSUTF8StringEncoding freeWhenDone:NO] autorelease];
    NSURL *nsurl = [NSURL URLWithString:urlString];
    QUrl url;
    // OS X 10.10 sends file references instead of file paths
    if ([nsurl isFileReferenceURL]) {
        url = urlFromNSURL([nsurl filePathURL]);
    } else {
        url = urlFromNSURL(nsurl);
    }

    return url; 
} 

