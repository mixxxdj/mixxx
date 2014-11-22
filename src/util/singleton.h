#ifndef SINGLETON_H
#define SINGLETON_H

#include <QtDebug>

template<class T>
class Singleton
{
public:
    static T* create() {
        if (!m_instance) {
            m_instance = new T();
        }
        return m_instance;
    }

    static T* instance() {
        if (m_instance == NULL) {
            qWarning() << "Singleton class has not been created yet, returning NULL";
        }
        return m_instance;
    }

    static void destroy() {
        if (m_instance) {
            delete m_instance;
        }
    }

protected:
    Singleton() {}
    virtual ~Singleton() {}

private:
    //hide copy constructor and assign operator
    Singleton(const Singleton&) {}
    const Singleton& operator= (const Singleton&) {}

    static T* m_instance;
};

template<class T> T* Singleton<T>::m_instance = 0;

#endif // SINGLETON_H
