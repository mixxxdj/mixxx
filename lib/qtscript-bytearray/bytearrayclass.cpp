/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtScript/QScriptClassPropertyIterator>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptEngine>
#include "bytearrayclass.h"
#include "bytearrayprototype.h"

#include <stdlib.h>

Q_DECLARE_METATYPE(QByteArray*)
Q_DECLARE_METATYPE(ByteArrayClass*)

class ByteArrayClassPropertyIterator : public QScriptClassPropertyIterator
{
public:
    ByteArrayClassPropertyIterator(const QScriptValue &object);
    ~ByteArrayClassPropertyIterator();

    bool hasNext() const;
    void next();

    bool hasPrevious() const;
    void previous();

    void toFront();
    void toBack();

    QScriptString name() const;
    uint id() const;

private:
    int m_index;
    int m_last;
};

//! [0]
ByteArrayClass::ByteArrayClass(QScriptEngine *engine)
    : QObject(engine), QScriptClass(engine)
{
    qScriptRegisterMetaType<QByteArray>(engine, toScriptValue, fromScriptValue);

    length = engine->toStringHandle(QLatin1String("length"));

    proto = engine->newQObject(new ByteArrayPrototype(this),
                               QScriptEngine::QtOwnership,
                               QScriptEngine::SkipMethodsInEnumeration
                               | QScriptEngine::ExcludeSuperClassMethods
                               | QScriptEngine::ExcludeSuperClassProperties);
    QScriptValue global = engine->globalObject();
    proto.setPrototype(global.property("Object").property("prototype"));

    ctor = engine->newFunction(construct, proto);
    ctor.setData(engine->toScriptValue(this));
}
//! [0]

ByteArrayClass::~ByteArrayClass()
{
}

//! [3]
QScriptClass::QueryFlags ByteArrayClass::queryProperty(const QScriptValue &object,
                                                       const QScriptString &name,
                                                       QueryFlags flags, uint *id)
{
    QByteArray *ba = qscriptvalue_cast<QByteArray*>(object.data());
    if (!ba)
        return 0;
    if (name == length) {
        return flags;
    } else {
        bool isArrayIndex;
        qint32 pos = name.toArrayIndex(&isArrayIndex);
        if (!isArrayIndex)
            return 0;
        *id = pos;
        if ((flags & HandlesReadAccess) && (pos >= ba->size()))
            flags &= ~HandlesReadAccess;
        return flags;
    }
}
//! [3]

//! [4]
QScriptValue ByteArrayClass::property(const QScriptValue &object,
                                      const QScriptString &name, uint id)
{
    QByteArray *ba = qscriptvalue_cast<QByteArray*>(object.data());
    if (!ba)
        return QScriptValue();
    if (name == length) {
        return ba->length();
    }
    qint32 pos = id;
    if ((pos < 0) || (pos >= ba->size())) {
        return QScriptValue();
    }
    return uint(ba->at(pos)) & 255;
}
//! [4]

//! [5]
void ByteArrayClass::setProperty(QScriptValue &object,
                                 const QScriptString &name,
                                 uint id, const QScriptValue &value)
{
    QByteArray *ba = qscriptvalue_cast<QByteArray*>(object.data());
    if (!ba)
        return;
    if (name == length) {
        resize(*ba, value.toInt32());
    } else {
        qint32 pos = id;
        if (pos < 0)
            return;
        if (ba->size() <= pos)
            resize(*ba, pos + 1);
        (*ba)[pos] = char(value.toInt32());
    }
}
//! [5]

//! [6]
QScriptValue::PropertyFlags ByteArrayClass::propertyFlags(
    const QScriptValue &/*object*/, const QScriptString &name, uint /*id*/)
{
    if (name == length) {
        return QScriptValue::Undeletable
            | QScriptValue::SkipInEnumeration;
    }
    return QScriptValue::Undeletable;
}
//! [6]

//! [7]
QScriptClassPropertyIterator *ByteArrayClass::newIterator(const QScriptValue &object)
{
    return new ByteArrayClassPropertyIterator(object);
}
//! [7]

QString ByteArrayClass::name() const
{
    return QLatin1String("ByteArray");
}

QScriptValue ByteArrayClass::prototype() const
{
    return proto;
}

QScriptValue ByteArrayClass::constructor()
{
    return ctor;
}

//! [10]
QScriptValue ByteArrayClass::newInstance(int size)
{
    // reportAdditionalMemoryCost is only available on >= Qt 4.7.0.
#if QT_VERSION >= QT_VERSION_CHECK(4, 7, 0)
    engine()->reportAdditionalMemoryCost(size);
#endif
    return newInstance(QByteArray(size, /*ch=*/0));
}
//! [10]

//! [1]
QScriptValue ByteArrayClass::newInstance(const QByteArray &ba)
{
    QScriptValue data = engine()->newVariant(QVariant::fromValue(ba));
    return engine()->newObject(this, data);
}
//! [1]

//! [2]
QScriptValue ByteArrayClass::construct(QScriptContext *ctx, QScriptEngine *)
{
    ByteArrayClass *cls = qscriptvalue_cast<ByteArrayClass*>(ctx->callee().data());
    if (!cls)
        return QScriptValue();
    QScriptValue arg = ctx->argument(0);
    if (arg.instanceOf(ctx->callee()))
        return cls->newInstance(qscriptvalue_cast<QByteArray>(arg));
    int size = arg.toInt32();
    return cls->newInstance(size);
}
//! [2]

QScriptValue ByteArrayClass::toScriptValue(QScriptEngine *eng, const QByteArray &ba)
{
    QScriptValue ctor = eng->globalObject().property("ByteArray");
    ByteArrayClass *cls = qscriptvalue_cast<ByteArrayClass*>(ctor.data());
    if (!cls)
        return eng->newVariant(QVariant::fromValue(ba));
    return cls->newInstance(ba);
}

void ByteArrayClass::fromScriptValue(const QScriptValue &obj, QByteArray &ba)
{
    ba = qvariant_cast<QByteArray>(obj.data().toVariant());
}

//! [9]
void ByteArrayClass::resize(QByteArray &ba, int newSize)
{
    int oldSize = ba.size();
    ba.resize(newSize);
    // reportAdditionalMemoryCost is only available on >= Qt 4.7.0.
#if QT_VERSION >= QT_VERSION_CHECK(4, 7, 0)
    if (newSize > oldSize)
        engine()->reportAdditionalMemoryCost(newSize - oldSize);
#endif
}
//! [9]



ByteArrayClassPropertyIterator::ByteArrayClassPropertyIterator(const QScriptValue &object)
    : QScriptClassPropertyIterator(object)
{
    toFront();
}

ByteArrayClassPropertyIterator::~ByteArrayClassPropertyIterator()
{
}

//! [8]
bool ByteArrayClassPropertyIterator::hasNext() const
{
    QByteArray *ba = qscriptvalue_cast<QByteArray*>(object().data());
    return m_index < ba->size();
}

void ByteArrayClassPropertyIterator::next()
{
    m_last = m_index;
    ++m_index;
}

bool ByteArrayClassPropertyIterator::hasPrevious() const
{
    return (m_index > 0);
}

void ByteArrayClassPropertyIterator::previous()
{
    --m_index;
    m_last = m_index;
}

void ByteArrayClassPropertyIterator::toFront()
{
    m_index = 0;
    m_last = -1;
}

void ByteArrayClassPropertyIterator::toBack()
{
    QByteArray *ba = qscriptvalue_cast<QByteArray*>(object().data());
    m_index = ba->size();
    m_last = -1;
}

QScriptString ByteArrayClassPropertyIterator::name() const
{
    return object().engine()->toStringHandle(QString::number(m_last));
}

uint ByteArrayClassPropertyIterator::id() const
{
    return m_last;
}
//! [8]
