#ifndef TAGUTILS_H
#define TAGUTILS_H

#include <QString>
#include <QHash>

typedef QHash<QString, int> TagCounts;

class TagUtils {
  public:
    static double jaccardSimilarity(const TagCounts& tags1,
                                    const TagCounts& tags2);
};

#endif // TAGUTILS_H
