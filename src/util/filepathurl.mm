#include "util/filepathurl.h"



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
    // OS X 10.10 sends file references instead of file paths
    if ([nsurl isFileReferenceURL]) {
        return urlFromNSURL([nsurl filePathURL]);
    }
    return url;
}
