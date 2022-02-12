#pragma once

#include <QHash>
#include <QString>
#include <QVarLengthArray>
#include <QtDebug>
#include <memory>

#include "util/assert.h"
#include "util/compatibility/qhash.h"

constexpr int kMaxExpectedChannelGroups = 256;

// ChannelHandle defines a unique identifier for channels of audio in the engine
// (e.g. headphone output, master output, deck 1, microphone 3). Previously we
// used the group string of the channel in the engine to uniquely identify it
// and key associative containers (e.g. QMap, QHash) but the downside to this is
// that we waste a lot of callback time hashing and re-hashing the strings.
//
// To solve this problem we introduce ChannelHandle, a thin wrapper around an
// integer. As engine channels are registered they are assigned a ChannelHandle
// starting at 0 and incrementing. The benefit to this scheme is that the hash
// and equality of ChannelHandles are simple to calculate and a QVarLengthArray
// can be used to create a fast associative container backed by a simple array
// (since the keys are numbered [0, num_channels]).

/// A wrapper around an integer handle. Used to uniquely identify and refer to
/// channels (headphone output, master output, deck 1, microphone 4, etc.) while
/// avoiding slow QString comparisons incurred when using the group.
///
/// A helper class, ChannelHandleFactory, keeps a running count of handles that
/// have been assigned.
class ChannelHandle final {
  public:
    bool valid() const {
        return m_iHandle >= 0;
    }

    int handle() const {
        DEBUG_ASSERT(valid());
        return m_iHandle;
    }

  private:
    ChannelHandle()
            : m_iHandle(-1) {
    }
    // Channel handles are pinned at a fixed memory address and must
    // neither be copied nor moved!!!
    ChannelHandle(ChannelHandle&&) = delete;
    ChannelHandle(const ChannelHandle&) = delete;
    ChannelHandle& operator=(ChannelHandle&&) = delete;
    ChannelHandle& operator=(const ChannelHandle&) = delete;

    int m_iHandle;

    friend class ChannelHandleFactory;
};

inline bool operator>(const ChannelHandle& h1, const ChannelHandle& h2) {
    return h1.handle() > h2.handle();
}

inline bool operator<(const ChannelHandle& h1, const ChannelHandle& h2) {
    return h1.handle() < h2.handle();
}

inline bool operator==(const ChannelHandle& h1, const ChannelHandle& h2) {
    return h1.handle() == h2.handle();
}

inline bool operator!=(const ChannelHandle& h1, const ChannelHandle& h2) {
    return h1.handle() != h2.handle();
}

inline QDebug operator<<(QDebug stream, const ChannelHandle& h) {
    stream << "ChannelHandle(" << h.handle() << ")";
    return stream;
}

inline qhash_seed_t qHash(
        const ChannelHandle& handle,
        qhash_seed_t seed = 0) {
    return qHash(handle.handle(), seed);
}

// Convenience class that mimics QPair<ChannelHandle, QString> except with
// custom equality and hash methods that save the cost of touching the QString.
class ChannelHandleAndGroup {
  public:
    ChannelHandleAndGroup(const ChannelHandle* pHandle, const QString& name)
            : m_pHandle(pHandle),
              m_name(name) {
    }

    const QString& name() const {
        return m_name;
    }

    const ChannelHandle* handle() const {
        return m_pHandle;
    }

    const ChannelHandle* m_pHandle;
    const QString m_name;
};

inline bool operator==(const ChannelHandleAndGroup& g1, const ChannelHandleAndGroup& g2) {
    return g1.handle() == g2.handle();
}

inline bool operator!=(const ChannelHandleAndGroup& g1, const ChannelHandleAndGroup& g2) {
    return g1.handle() != g2.handle();
}

inline QDebug operator<<(QDebug stream, const ChannelHandleAndGroup& g) {
    stream << "ChannelHandleAndGroup(" << g.name() << "," << g.handle() << ")";
    return stream;
}

inline qhash_seed_t qHash(
        const ChannelHandleAndGroup& handleGroup,
        qhash_seed_t seed = 0) {
    return qHash(handleGroup.handle(), seed);
}

// A helper class used by EngineMaster to assign ChannelHandles to channel group
// strings. Warning: ChannelHandles produced by different ChannelHandleFactory
// objects are not compatible and will produce incorrect results when compared,
// stored in the same container, etc. In practice we only use one instance in
// EngineMaster.
class ChannelHandleFactory {
  public:
    ChannelHandleFactory() = default;

    /// Obtain a reference to a channel handle for a group
    ///
    /// The returned reference must be stable during a session and references
    /// an entry from an internal map. Thus it is safe to get the address of
    /// this reference. This is REQUIRED for passing ChannelHandle to
    /// EffectChainManager by a pointer!!!
    const ChannelHandle* getOrCreateHandle(const QString& group) {
        auto groupToHandleIter = m_groupToHandle.find(group);
        if (groupToHandleIter == m_groupToHandle.end()) {
            groupToHandleIter = m_groupToHandle.insert(group, m_groupToHandle.size());
            DEBUG_ASSERT(!m_handleToGroup.contains(groupToHandleIter.value()));
            m_handleToGroup.insert(groupToHandleIter.value(), group);
        }
        DEBUG_ASSERT(groupToHandleIter != m_groupToHandle.end());
        // Return reference from the map that is pinned to a fixed
        // memory address!!!
        const auto handleIndex = groupToHandleIter.value();
        DEBUG_ASSERT(handleIndex >= 0);
        DEBUG_ASSERT(handleIndex < static_cast<int>(std::size(m_groupHandles)));
        if (!m_groupHandles[handleIndex].valid()) {
            m_groupHandles[handleIndex].m_iHandle = handleIndex;
            qInfo() << "Created" << m_groupHandles[handleIndex] << "for group" << group;
        }
        DEBUG_ASSERT(m_groupHandles[handleIndex].m_iHandle == handleIndex);
        return &m_groupHandles[handleIndex];
    }

    const ChannelHandle* handleForGroup(const QString& group) const {
        auto groupToHandleIter = m_groupToHandle.find(group);
        if (groupToHandleIter == m_groupToHandle.end()) {
            return nullptr;
        }
        // Return reference from the map that is pinned to a fixed
        // memory address!!!
        const auto handleIndex = groupToHandleIter.value();
        DEBUG_ASSERT(handleIndex >= 0);
        DEBUG_ASSERT(handleIndex < static_cast<int>(std::size(m_groupHandles)));
        return &m_groupHandles[handleIndex];
    }

    QString groupForHandle(const ChannelHandle* pHandle) const {
        VERIFY_OR_DEBUG_ASSERT(pHandle) {
            return {};
        }
        return m_handleToGroup.value(pHandle->handle());
    }

  private:
    ChannelHandle m_groupHandles[kMaxExpectedChannelGroups];
    QHash<QString, int> m_groupToHandle;
    QHash<int, QString> m_handleToGroup;
};

typedef std::shared_ptr<ChannelHandleFactory> ChannelHandleFactoryPointer;

// An associative container mapping ChannelHandle to a template type T. Backed
// by a QVarLengthArray with ChannelHandleMap::kMaxExpectedChannelGroups pre-allocated
// entries. Insertions are amortized O(1) time (if less than kMaxExpectedChannelGroups
// exist then no allocation will occur -- insertion is a mere copy). Lookups are
// O(1) and quite fast -- a simple index into an array using the handle's
// integer value.
template <class T>
class ChannelHandleMap {
    typedef QVarLengthArray<T, kMaxExpectedChannelGroups> container_type;

  public:
    typedef typename QVarLengthArray<T, kMaxExpectedChannelGroups>::const_iterator const_iterator;
    typedef typename QVarLengthArray<T, kMaxExpectedChannelGroups>::iterator iterator;

    ChannelHandleMap()
            : m_dummy{} {
    }

    const T& at(const ChannelHandle* pHandle) const {
        if (!pHandle) {
            return m_dummy;
        }
        DEBUG_ASSERT(pHandle->valid());
        return m_data.at(pHandle->handle());
    }

    void insert(const ChannelHandle* pHandle, const T& value) {
        if (!pHandle) {
            return;
        }
        DEBUG_ASSERT(pHandle->valid());
        int iHandle = pHandle->handle();
        maybeExpand(iHandle + 1);
        m_data[iHandle] = value;
    }

    T& operator[](const ChannelHandle* pHandle) {
        if (!pHandle) {
            return m_dummy;
        }
        DEBUG_ASSERT(pHandle->valid());
        int iHandle = pHandle->handle();
        maybeExpand(iHandle + 1);
        return m_data[iHandle];
    }

    void clear() {
        m_data.clear();
    }

    typename container_type::iterator begin() {
        return m_data.begin();
    }

    typename container_type::const_iterator begin() const {
        return m_data.begin();
    }

    typename container_type::iterator end() {
        return m_data.end();
    }

    typename container_type::const_iterator end() const {
        return m_data.end();
    }

  private:
    inline void maybeExpand(int iSize) {
        if (QTypeInfo<T>::isComplex) {
            // The value for complex types is initialized by QVarLengthArray
            if (m_data.size() < iSize) {
                m_data.resize(iSize);
            }
        } else {
            // We need to initialize simple types ourselves
            while (m_data.size() < iSize) {
                m_data.append({});
            }
        }
    }
    container_type m_data;
    T m_dummy;
};
