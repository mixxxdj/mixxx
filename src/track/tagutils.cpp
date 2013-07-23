#include "lastfm/lastfmclient.h"
#include "track/tagutils.h"

double TagUtils::jaccardSimilarity(const TagCounts& tags1,
                                   const TagCounts& tags2) {
    int iTagIntersection = 0;
    int iTagUnion = 0;
    for (TagCounts::const_iterator it = tags1.constBegin();
         it != tags1.constEnd(); ++it) {
        QString tag = it.key();
        if (tags2.contains(tag)) {

        } else {

        }
    }
    return 0.0;
}
