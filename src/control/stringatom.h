#ifndef STRINGATOM_H
#define STRINGATOM_H

#include <QHash>
#include <QMutex>
#include <QString>
#include <QAtomicPointer>

class StringAtom {
  public:
    StringAtom();
    StringAtom(const char* asciiString);
    StringAtom(const QString& string);
    ~StringAtom();

    QByteArray toAscii() const;
    const QString& toString() const;

    static const StringAtom get(const char* asciiString);
    static const StringAtom get(const QString& string);

    inline bool isValid() {
        return m_pString ? true : false;
    }

    inline bool operator==(const StringAtom& other) const {
        return this->m_pString == other.m_pString;
    }

    inline bool operator==(const QString& other) const {
        return this->m_pString == get(other).m_pString;
    }

    // For QMap
    inline bool operator<(const StringAtom& other) const {
        return this->m_pString < other.m_pString;
    }

    inline operator QString() {
        return QString(*m_pString);
    }
/*
    StringAtom& operator= (char* asciiString);
    StringAtom& operator= (const QString & other);
*/

  private:
    StringAtom(const QString* pString);
    void initalize(const QString& string);
    static const QString* getInner(const QString& string);

    const QString* m_pString;
    // Hash of all StringAtomic instantiations.
    static QHash<QString, const QString*> s_stringHash;
    // Mutex guarding access to s_stringHash
    static QMutex s_stringHashMutex;
};

#endif // STRINGATOM_H
