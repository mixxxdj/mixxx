/**
  @file
  @author Stefan Frings
*/

#include "httpsession.h"
#include <QDateTime>
#include <QUuid>

using namespace stefanfrings;

HttpSession::HttpSession(bool canStore)
{
    if (canStore)
    {
        dataPtr=new HttpSessionData();
        dataPtr->refCount=1;
        dataPtr->lastAccess=QDateTime::currentMSecsSinceEpoch();
        dataPtr->id=QUuid::createUuid().toString().toLocal8Bit();
#ifdef SUPERVERBOSE
        qDebug("HttpSession: (constructor) new session %s with refCount=1",dataPtr->id.constData());
#endif
    }
    else
    {
        dataPtr=nullptr;
    }
}

HttpSession::HttpSession(const HttpSession& other)
{
    dataPtr=other.dataPtr;
    if (dataPtr)
    {
        dataPtr->lock.lockForWrite();
        dataPtr->refCount++;
#ifdef SUPERVERBOSE
        qDebug("HttpSession: (constructor) copy session %s refCount=%i",dataPtr->id.constData(),dataPtr->refCount);
#endif
        dataPtr->lock.unlock();
    }
}

HttpSession& HttpSession::operator= (const HttpSession& other)
{
    HttpSessionData* oldPtr=dataPtr;
    dataPtr=other.dataPtr;
    if (dataPtr)
    {
        dataPtr->lock.lockForWrite();
        dataPtr->refCount++;
#ifdef SUPERVERBOSE
        qDebug("HttpSession: (operator=) session %s refCount=%i",dataPtr->id.constData(),dataPtr->refCount);
#endif
        dataPtr->lastAccess=QDateTime::currentMSecsSinceEpoch();
        dataPtr->lock.unlock();
    }
    if (oldPtr)
    {
        int refCount;
        oldPtr->lock.lockForWrite();
        refCount=--oldPtr->refCount;
#ifdef SUPERVERBOSE
        qDebug("HttpSession: (operator=) session %s refCount=%i",oldPtr->id.constData(),oldPtr->refCount);
#endif
        oldPtr->lock.unlock();
        if (refCount==0)
        {
            qDebug("HttpSession: deleting old data");
            delete oldPtr;
        }
    }
    return *this;
}

HttpSession::~HttpSession()
{
    if (dataPtr) {
        int refCount;
        dataPtr->lock.lockForWrite();
        refCount=--dataPtr->refCount;
#ifdef SUPERVERBOSE
        qDebug("HttpSession: (destructor) session %s refCount=%i",dataPtr->id.constData(),dataPtr->refCount);
#endif
        dataPtr->lock.unlock();
        if (refCount==0)
        {
            qDebug("HttpSession: deleting data");
            delete dataPtr;
        }
    }
}


QByteArray HttpSession::getId() const
{
    if (dataPtr)
    {
        return dataPtr->id;
    }
    else
    {
        return QByteArray();
    }
}

bool HttpSession::isNull() const {
    return dataPtr==nullptr;
}

void HttpSession::set(const QByteArray& key, const QVariant& value)
{
    if (dataPtr)
    {
        dataPtr->lock.lockForWrite();
        dataPtr->values.insert(key,value);
        dataPtr->lock.unlock();
    }
}

void HttpSession::remove(const QByteArray& key)
{
    if (dataPtr)
    {
        dataPtr->lock.lockForWrite();
        dataPtr->values.remove(key);
        dataPtr->lock.unlock();
    }
}

QVariant HttpSession::get(const QByteArray& key) const
{
    QVariant value;
    if (dataPtr)
    {
        dataPtr->lock.lockForRead();
        value=dataPtr->values.value(key);
        dataPtr->lock.unlock();
    }
    return value;
}

bool HttpSession::contains(const QByteArray& key) const
{
    bool found=false;
    if (dataPtr)
    {
        dataPtr->lock.lockForRead();
        found=dataPtr->values.contains(key);
        dataPtr->lock.unlock();
    }
    return found;
}

QMap<QByteArray,QVariant> HttpSession::getAll() const
{
    QMap<QByteArray,QVariant> values;
    if (dataPtr)
    {
        dataPtr->lock.lockForRead();
        values=dataPtr->values;
        dataPtr->lock.unlock();
    }
    return values;
}

qint64 HttpSession::getLastAccess() const
{
    qint64 value=0;
    if (dataPtr)
    {
        dataPtr->lock.lockForRead();
        value=dataPtr->lastAccess;
        dataPtr->lock.unlock();
    }
    return value;
}


void HttpSession::setLastAccess()
{
    if (dataPtr)
    {
        dataPtr->lock.lockForWrite();
        dataPtr->lastAccess=QDateTime::currentMSecsSinceEpoch();
        dataPtr->lock.unlock();
    }
}
