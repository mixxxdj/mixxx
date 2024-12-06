#include "util/gitinfostore.h"

#define GIT_INFO
#include "gitinfo.h"

QString GitInfoStore::branch() {
    return QStringLiteral(GIT_BRANCH);
};

QString GitInfoStore::describe() {
    return QStringLiteral(GIT_DESCRIBE);
};

QString GitInfoStore::date() {
    return QStringLiteral(GIT_COMMIT_DATE);
};

int GitInfoStore::commitCount() {
#ifdef GIT_COMMIT_COUNT
    return GIT_COMMIT_COUNT;
#else
    return 0;
#endif
};

bool GitInfoStore::dirty() {
#ifdef GIT_DIRTY
    return true;
#else
    return false;
#endif
};
