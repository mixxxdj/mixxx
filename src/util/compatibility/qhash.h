#pragma once

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
typedef size_t qhash_seed_t;
#else
typedef uint qhash_seed_t;
#endif
