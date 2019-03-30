#include "util/filepathurl.h"


// Taken from src/platformsupport/clipboard/qmacmime.mm

QUrl ensureFilePathUrl(const QUrl& url) {
    const QByteArray &a = url.toEncoded();
    NSString *urlString = [[[NSString alloc] initWithBytesNoCopy:(void *)a.data() length:a.size()
                                             encoding:NSUTF8StringEncoding freeWhenDone:NO] autorelease];
    NSURL *nsurl = [NSURL URLWithString:urlString];
    QUrl url;
    // OS X 10.10 sends file references instead of file paths
    if ([nsurl isFileReferenceURL]) {
        url = QUrl::fromNSURL([nsurl filePathURL]);
    } else {
        url = QUrl::fromNSURL(nsurl);
    }

	return url; 
} 

