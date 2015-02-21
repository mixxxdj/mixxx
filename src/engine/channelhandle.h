#ifndef CHANNELHANDLE_H
#define CHANNELHANDLE_H

#include <QtDebug>
#include <QHash>
#include <QString>
#include <QVarLengthArray>

#include "util/assert.h"

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

inline uint qHash(const ChannelHandle& handle) {
    return qHash(handle.handle());
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

    ChannelHandle m_handle;
    QString m_name;
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

inline uint qHash(const ChannelHandleAndGroup& group) {
    return qHash(group.handle());
}

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

template <class T>
class ChannelHandleMap {
    static const int kMaxExpectedGroups = 256;
    typedef QVarLengthArray<T, kMaxExpectedGroups> container_type;
  public:
    typedef typename QVarLengthArray<T, kMaxExpectedGroups>::const_iterator const_iterator;
    typedef typename QVarLengthArray<T, kMaxExpectedGroups>::iterator iterator;

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
        if (m_data.size() < iSize) {
            m_data.resize(iSize);
        }
    }
    container_type m_data;
    T m_dummy;
};

#endif /* CHANNELHANDLE,_H */
