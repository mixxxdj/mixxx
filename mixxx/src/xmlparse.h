//
// C++ Interface: xmlparse
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef XMLPARSE_H
#define XMLPARSE_H

#include <qdom.h>
#include <q3memarray.h>
#include <q3valuelist.h>

/**
@author Tue Haste Andersen
*/

class XmlParse
{
public:
    XmlParse();
    ~XmlParse();

    static QDomNode selectNode(const QDomNode &nodeHeader, const QString sNode);
    static int selectNodeInt(const QDomNode &nodeHeader, const QString sNode);
    static float selectNodeFloat(const QDomNode &nodeHeader, const QString sNode);
    static QString selectNodeQString(const QDomNode &nodeHeader, const QString sNode);
    static Q3MemArray<long> *selectNodeLongArray(const QDomNode &nodeHeader, const QString sNode);
    static Q3MemArray<char> *selectNodeCharArray(const QDomNode &nodeHeader, const QString sNode);
	static Q3MemArray<char> *selectNodeHexCharArray(const QDomNode &nodeHeader, const QString sNode);
    static Q3ValueList<long> *selectNodeLongList(const QDomNode &nodeHeader, const QString sNode);
    /** Add a node. Used to write xml document */
    static QDomElement addElement(QDomDocument &, QDomElement &, QString, QString);
    /** Add a binary node, long ints encoded as chars. Used to write xml document */
    static QDomElement addElement(QDomDocument &, QDomElement &, QString, Q3ValueList<long> *pData);
    /** Add a binary node of chars. Used to write xml document */
    static QDomElement addElement(QDomDocument &, QDomElement &, QString, Q3MemArray<char> *pData);
	// Take a binary char array, encode as hex pairs write to xml
	static QDomElement addHexElement(QDomDocument &, QDomElement &, QString, Q3MemArray<char> *pData);
};

#endif
