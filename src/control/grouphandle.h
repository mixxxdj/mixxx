#pragma once

#include <QString>
#include <QtDebug>

#include "util/assert.h"
#include "util/compatibility/qhash.h"

namespace mixxx {
namespace grouphandle_private {
class Descriptor;
} // namespace grouphandle_private
} // namespace mixxx

typedef const mixxx::grouphandle_private::Descriptor* GroupHandle;

constexpr GroupHandle kNullGroupHandle = nullptr;

GroupHandle getOrCreateGroupHandleByName(
        const QString& name,
        bool create = true);

inline GroupHandle getGroupHandleByName(const QString& name) {
    return getOrCreateGroupHandleByName(name, false);
}

/// Finalize the registration of groups
///
/// After invoking this functions no handles for new groups
/// could be created. Accessing the existing handles by their
/// name will be lock-free from now on.
void freezeAllGroupHandles();

/// Free all group handles
///
/// Do not call while handles are in use!!! Mainly needed for testing.
///
/// Returns the number of groups.
int resetAllGroupHandles();

namespace mixxx {
namespace grouphandle_private {

class Descriptor final {
  public:
    static constexpr int kInvalidIndex = -1;

    // Trailing return type declaration is required for Clang 14
    friend auto ::getOrCreateGroupHandleByName(
            const QString& name,
            bool create) -> GroupHandle;

    Descriptor() = default;
    Descriptor(Descriptor&&) = delete;
    Descriptor(const Descriptor&) = delete;
    Descriptor& operator=(Descriptor&&) = delete;
    Descriptor& operator=(const Descriptor&) = delete;

    /// Index
    ///
    /// 0-based integer identifier. The maximum number is limited by the
    /// total number of different descriptors, i.e. the total number of
    /// distinct group names.
    friend int indexOfGroupHandle(GroupHandle handle) {
        if (!handle) {
            return kInvalidIndex;
        }
        DEBUG_ASSERT(handle->valid());
        return handle->m_index;
    }

    /// Group name
    ///
    /// String identifier.
    friend QString nameOfGroupHandle(GroupHandle handle) {
        if (!handle) {
            return {};
        }
        DEBUG_ASSERT(handle->valid());
        return handle->m_name;
    }

    friend bool operator<(const Descriptor& lhs, const Descriptor& rhs) {
        return lhs.m_index < rhs.m_index;
    }

    friend bool operator==(const Descriptor& lhs, const Descriptor& rhs) {
        return lhs.m_index == rhs.m_index;
    }

    friend qhash_seed_t qHash(const Descriptor& arg, qhash_seed_t seed = 0) {
        return qHash(arg.m_index, seed);
    }

    friend QDebug operator<<(QDebug dbg, const Descriptor& arg);

  private:
    bool valid() const {
        DEBUG_ASSERT(m_index == kInvalidIndex || m_index >= 0);
        DEBUG_ASSERT((m_index == kInvalidIndex) == m_name.isEmpty());
        return m_index != kInvalidIndex;
    }

    Descriptor(int index, QString name)
            : m_index(index),
              m_name(std::move(name)) {
    }

    int m_index = kInvalidIndex;
    QString m_name;
};

inline bool operator!=(
        const Descriptor& lhs,
        const Descriptor& rhs) {
    return !(lhs == rhs);
}

} // namespace grouphandle_private

} // namespace mixxx

inline qhash_seed_t qHash(GroupHandle arg, qhash_seed_t seed = 0) {
    if (arg) {
        return qHash(*arg, seed);
    } else {
        return qHash(mixxx::grouphandle_private::Descriptor{}, seed);
    }
}

inline QDebug operator<<(QDebug dbg, GroupHandle arg) {
    if (arg) {
        return dbg << *arg;
    } else {
        return dbg << mixxx::grouphandle_private::Descriptor{};
    }
}
