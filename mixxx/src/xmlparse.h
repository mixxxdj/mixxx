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
    static QMemArray<char> *selectNodeCharArray(const QDomNode &nodeHeader, const QString sNode);
    static QValueList<int> *selectNodeIntList(const QDomNode &nodeHeader, const QString sNode);
    /** Add a node. Used to write xml document */
    static QDomElement addElement(QDomDocument &, QDomElement &, QString, QString);
    /** Add a binary node, char encoding. Used to write xml document */
    static QDomElement addElement(QDomDocument &, QDomElement &, QString, QMemArray<char> *pData);
    /** Add a binary node, int encoding. Used to write xml document */
    static QDomElement addElement(QDomDocument &, QDomElement &, QString, QValueList<int> *pData);
};

#endif
