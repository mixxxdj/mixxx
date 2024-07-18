#ifndef UTIL_USE_COUNTING_PTR_H
#define UTIL_USE_COUNTING_PTR_H

#include <QMutex>
#include <QAtomicInt>
#include "util/assert.h"

// This is a wrapper class that can be used to protect pointers that are owned 
// by an one object but stored at an other. It prevents that the pointer is removed 
// from the other object while currently executing an callback from a second thread.
// It uses a lock free implementations except the final clear if the object is still
// in use.
// This can be used to implement callbacks to an object with shorter lifetime. 
// This can be used as a save replacement for Qt's direct connections, which must not 
// be used across threads in this case.
//
// Usage:
// call set(po) and clear() to store and remove a pinter from the object.
// call the callback from a scope like this
//{
//    auto obj = m_obj.get();
//    if (obj) {
//        obj->callback();
//    }
//}


template <typename T>
class use_counting_ptr;

template <typename T>
class used_ptr {
  public:
    explicit used_ptr(use_counting_ptr<T>* pUcp)
        : m_pUcp(pUcp),
          m_p(nullptr) {
        if(m_pUcp) {
          m_p = pUcp->use();
        }
    }

    ~used_ptr() {
        if (m_pUcp && m_p) {
            m_pUcp->release(&m_p);
        }
    }

    /* If U* is convertible to T* then we also want parented_ptr<U> convertible to parented_ptr<T> */
    template <typename U>
    used_ptr(used_ptr<U>&& u, typename std::enable_if<std::is_convertible<U*, T*>::value, void>::type * = 0)
            : m_pUcp(reinterpret_cast<use_counting_ptr<T>*>(u.m_pUcp)),
              m_p(u.m_p) {
        u.m_pUcp = nullptr;
        u.m_p = nullptr;
    }

    // Delete copy constructor and copy assignment operator
    // because this object should be not copied arround.
    // This should only live during a call
    used_ptr(const used_ptr<T>& up) = delete;
    used_ptr& operator=(const used_ptr<T>&) = delete;

    T& operator* () const {
        return *m_p;
    }

    T* operator-> () const {
        return m_p;
    }

    operator bool() const {
        return m_p != nullptr;
    }

    template<typename> friend class used_ptr;

  private:
    use_counting_ptr<T>* m_pUcp;
    T* m_p;
};


template <typename T>
class use_counting_ptr {
  public:
    explicit use_counting_ptr(T* p)
        : m_p(p),
          m_useCount(0) {
        m_mutex.lock();
    }

    use_counting_ptr()
        : m_p(nullptr),
          m_useCount(0) {
    }

    // Delete copy constructor and copy assignment operator
    use_counting_ptr(const use_counting_ptr<T>&) = delete;
    use_counting_ptr& operator=(const use_counting_ptr<T>&) = delete;

    void set(T* p) {
        if (load_atomic_pointer(p)) {
            clear();
        }
        if (p) {
            m_mutex.lock();
            m_p = p;
        }
        qDebug() << "use_counting_ptr::set()";
    }

    used_ptr<T> get() {
        return used_ptr<T>(this);
    }

    void clear() {
        m_p = nullptr;
        if (load_atomic(m_useCount)) {
            qDebug() << "use_counting_ptr::clear() pointer in use, lock itself";
            m_mutex.lock(); // lock itselfe by a second lock call
        }
        qDebug() << "use_counting_ptr::clear()" << load_atomic(m_useCount);
    }

    T* use() {
        qDebug() << "use_counting_ptr::use()" << load_atomic(m_useCount);
        m_useCount.fetchAndAddAcquire(1);
        return load_atomic_pointer(m_p);
    }

    void release(T** pp) {
        *pp = nullptr;
        qDebug() << "use_counting_ptr::release()" << load_atomic(m_useCount);
        if(m_useCount.fetchAndAddRelease(-1) == 1 && !load_atomic_pointer(m_p)) {
            // This happens when the owner has cleared the pointer in the meanwhile
            // and has locked itself
            m_mutex.unlock();
        }
    }

  private:
    QAtomicPointer<T> m_p;
    QAtomicInt m_useCount;
    QMutex m_mutex;
};


#endif // UTIL_USE_COUNTING_PTR_H
