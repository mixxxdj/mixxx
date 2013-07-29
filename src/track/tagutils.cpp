#include <QtDebug>
#include <QSet>

#include "lastfm/lastfmclient.h"
#include "track/tagutils.h"

double TagUtils::jaccardSimilarity(const TagCounts& tags1,
                                   const TagCounts& tags2) {
    int iTagIntersection = 0;
    int iTagUnion = 0;
    QSet<QString> keys = QSet<QString>::fromList(tags1.keys());
    QSet<QString> keys2 = QSet<QString>::fromList(tags2.keys());
    keys.unite(keys2);
    foreach (QString key, keys)  {
//        int count1 = tags1.value(key);
//        int count2 = tags2.value(key);
//        iTagIntersection += std::min(count1, count2);
//        iTagUnion += count1 + count2;
        if (tags1.contains(key) && tags2.contains(key)) {
            iTagIntersection++;
        }
        iTagUnion++;
    }
    double result = iTagUnion > 0 ? double(iTagIntersection) / iTagUnion : 0.0;
    qDebug() << iTagIntersection << iTagUnion << result;
    return result;
}

// Also known as Simpson similarity; if either set is a subset of the other,
// this equals 1.
double TagUtils::overlapSimilarity(const TagCounts& tags1,
                                   const TagCounts& tags2) {
    int iTagIntersection = 0;
    QSet<QString> keys = QSet<QString>::fromList(tags1.keys());
    QSet<QString> keys2 = QSet<QString>::fromList(tags2.keys());
    keys.unite(keys2);
    foreach (QString key, keys)  {
        if (tags1.contains(key) && tags2.contains(key)) {
            iTagIntersection++;
        }
    }
    int iTagCardinality = std::min(keys.size(), keys2.size());
    return iTagCardinality != 0 ? double(iTagIntersection)/iTagCardinality: 0.0;
}
