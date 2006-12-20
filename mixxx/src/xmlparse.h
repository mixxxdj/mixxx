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
#include <qmemarray.h>
#include <qvaluelist.h>

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
    static QMemArray<long> *selectNodeLongArray(const QDomNode &nodeHeader, const QString sNode);
    static QMemArray<char> *selectNodeCharArray(const QDomNode &nodeHeader, const QString sNode);
	static QMemArray<char> *selectNodeHexCharArray(const QDomNode &nodeHeader, const QString sNode);
    static QValueList<long> *selectNodeLongList(const QDomNode &nodeHeader, const QString sNode);
    /** Add a node. Used to write xml document */
    static QDomElement addElement(QDomDocument &, QDomElement &, QString, QString);
    /** Add a binary node, long ints encoded as chars. Used to write xml document */
    static QDomElement addElement(QDomDocument &, QDomElement &, QString, QValueList<long> *pData);
    /** Add a binary node of chars. Used to write xml document */
    static QDomElement addElement(QDomDocument &, QDomElement &, QString, QMemArray<char> *pData);
	// Take a binary char array, encode as hex pairs write to xml
	static QDomElement addHexElement(QDomDocument &, QDomElement &, QString, QMemArray<char> *pData);
};

#endif
