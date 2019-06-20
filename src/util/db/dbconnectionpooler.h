#ifndef MIXXX_DBCONNECTIONPOOLER_H
#define MIXXX_DBCONNECTIONPOOLER_H


#include "util/db/dbconnectionpool.h"


namespace mixxx {

// Manages the lifetime of a thread-local database connection that is
// shared through DbConnectionPool. It is created and added to the pool
// upon construction and will be closed and removed from the pool upon
// destruction.
//
// Ultimately upon termination of a thread the corresponding connection
// would also be closed and removed implicitly by the pool, but that
// should never happen! Therefore this class should always be allocated
// on the stack and not dynamically on the heap so that it cannot outlive
// the corresponding thread.
class DbConnectionPooler final {
  public:
    explicit DbConnectionPooler(
            DbConnectionPoolPtr pDbConnectionPool = DbConnectionPoolPtr());
    DbConnectionPooler(const DbConnectionPooler&) = delete;
    DbConnectionPooler(DbConnectionPooler&&) = default;
    ~DbConnectionPooler();

    // Checks if a thread-local connection has actually been created
    // during construction and is owned by this instance.
    bool isPooling() const {
        return static_cast<bool>(m_pDbConnectionPool);
    }

    DbConnectionPooler& operator=(const DbConnectionPooler&) = delete;
    DbConnectionPooler& operator=(DbConnectionPooler&&) = default;

  private:
    // Prevent heap allocation
    static void * operator new(std::size_t);
    static void * operator new[](std::size_t);

    DbConnectionPoolPtr m_pDbConnectionPool;
};

} // namespace mixxx


#endif // MIXXX_DBCONNECTIONPOOLER_H
