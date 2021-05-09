#include "util/gitinfostore.h"

#define GIT_INFO
#include "gitinfo.h"

//static
const char* GitInfoStore::branch() {
    return GIT_BRANCH;
};

//static
const char* GitInfoStore::describe() {
    return GIT_DESCRIBE;
};

//static
const char* GitInfoStore::date() {
    return GIT_COMMIT_DATE;
};

//static
int GitInfoStore::commitCount() {
#ifdef GIT_COMMIT_COUNT
    return GIT_COMMIT_COUNT;
#else
    return 0;
#endif
};

//static
bool GitInfoStore::dirty() {
#ifdef GIT_DIRTY
    return true;
#else
    return false;
#endif
};
