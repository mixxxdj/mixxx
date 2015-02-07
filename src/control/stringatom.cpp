#include <QtDebug>
#include <QMutexLocker>

#include "control/stringatom.h"

#include "util/stat.h"
#include "util/timer.h"

// Static member variable definition
QHash<QString, const QString*> StringAtom::s_stringHash;
QMutex StringAtom::s_stringHashMutex;

StringAtom::StringAtom()
        : m_pString(NULL) {
}

StringAtom::StringAtom(const char* asciiString) {
    QString string(asciiString);
    initalize(string);
}

StringAtom::StringAtom(const QString& string) {
    initalize(string);
}

StringAtom::StringAtom(const QString* pString)
        : m_pString(pString) {
}

StringAtom::~StringAtom() {
    // do not delete m_pString here, it is owned by the s_stringHash
}

void StringAtom::initalize(const QString& string) {
    QMutexLocker locker(&s_stringHashMutex);
    const QString* pString = getInner(string);
    if (!pString) {
        pString = new QString(string);
        QString key = *pString;
        s_stringHash.insert(key, pString);
    }
    m_pString = pString;
}

/*
StringAtom& StringAtom::operator= (const QString & string) {
    return *this;
}
*/

QByteArray StringAtom::toAscii() const {
    return m_pString->toAscii();
}

const QString& StringAtom::toString() const {
    return *m_pString;
}

// static
const StringAtom StringAtom::get(const char* asciiString) {
    QString string(asciiString);
    return get(string);
}

// static
const StringAtom StringAtom::get(const QString& string) {
    QMutexLocker locker(&s_stringHashMutex);
    const QString* pString = getInner(string);
    return StringAtom(pString);
}

// static
const QString* StringAtom::getInner(const QString& string) {
    return s_stringHash.value(string);
}
