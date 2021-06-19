#pragma once

#include <QHash>
#include <QString>
#include <QVarLengthArray>
#include <QtDebug>
#include <memory>

#include "util/assert.h"

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
class ChannelHandle {
  public:
    ChannelHandle() : m_iHandle(-1) {
    }

    inline bool valid() const {
        return m_iHandle >= 0;
    }

    inline int handle() const {
        return m_iHandle;
    }

  private:
    ChannelHandle(int iHandle)
            : m_iHandle(iHandle) {
    }

    void setHandle(int iHandle) {
        m_iHandle = iHandle;
    }

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

inline uint qHash(
        const ChannelHandle& handle,
        uint seed = 0) {
    return qHash(handle.handle(), seed);
}

// Convenience class that mimics QPair<ChannelHandle, QString> except with
// custom equality and hash methods that save the cost of touching the QString.
class ChannelHandleAndGroup {
  public:
    ChannelHandleAndGroup(const ChannelHandle& handle, const QString& name)
            : m_handle(handle),
              m_name(name) {
    }

    inline const QString& name() const {
        return m_name;
    }

    inline const ChannelHandle& handle() const {
        return m_handle;
    }

    const ChannelHandle m_handle;
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

inline uint qHash(
        const ChannelHandleAndGroup& handleGroup,
        uint seed = 0) {
    return qHash(handleGroup.handle(), seed);
}

// A helper class used by EngineMaster to assign ChannelHandles to channel group
// strings. Warning: ChannelHandles produced by different ChannelHandleFactory
// objects are not compatible and will produce incorrect results when compared,
// stored in the same container, etc. In practice we only use one instance in
// EngineMaster.
class ChannelHandleFactory {
  public:
    ChannelHandleFactory() : m_iNextHandle(0) {
    }

    ChannelHandle getOrCreateHandle(const QString& group) {
        ChannelHandle& handle = m_groupToHandle[group];
        if (!handle.valid()) {
            handle.setHandle(m_iNextHandle++);
            DEBUG_ASSERT(handle.valid());
            DEBUG_ASSERT(!m_handleToGroup.contains(handle));
            m_handleToGroup.insert(handle, group);
        }
        return handle;
    }

    ChannelHandle handleForGroup(const QString& group) const {
        return m_groupToHandle.value(group, ChannelHandle());
    }

    QString groupForHandle(const ChannelHandle& handle) const {
        return m_handleToGroup.value(handle, QString());
    }

  private:
    int m_iNextHandle;
    QHash<QString, ChannelHandle> m_groupToHandle;
    QHash<ChannelHandle, QString> m_handleToGroup;
};

typedef std::shared_ptr<ChannelHandleFactory> ChannelHandleFactoryPointer;

// An associative container mapping ChannelHandle to a template type T. Backed
// by a QVarLengthArray with ChannelHandleMap::kMaxExpectedGroups pre-allocated
// entries. Insertions are amortized O(1) time (if less than kMaxExpectedGroups
// exist then no allocation will occur -- insertion is a mere copy). Lookups are
// O(1) and quite fast -- a simple index into an array using the handle's
// integer value.
template <class T>
class ChannelHandleMap {
    static const int kMaxExpectedGroups = 256;
    typedef QVarLengthArray<T, kMaxExpectedGroups> container_type;
  public:
    typedef typename QVarLengthArray<T, kMaxExpectedGroups>::const_iterator const_iterator;
    typedef typename QVarLengthArray<T, kMaxExpectedGroups>::iterator iterator;

    ChannelHandleMap()
            : m_dummy{} {
    }

    const T& at(const ChannelHandle& handle) const {
        if (!handle.valid()) {
            return m_dummy;
        }
        return m_data.at(handle.handle());
    }

    void insert(const ChannelHandle& handle, const T& value) {
        if (!handle.valid()) {
            return;
        }

        int iHandle = handle.handle();
        maybeExpand(iHandle + 1);
        m_data[iHandle] = value;
    }

    T& operator[](const ChannelHandle& handle) {
        if (!handle.valid()) {
            return m_dummy;
        }
        int iHandle = handle.handle();
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
