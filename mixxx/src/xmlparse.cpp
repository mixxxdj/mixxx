//
// C++ Implementation: xmlparse
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "xmlparse.h"
#ifdef QT3_SUPPORT
#include <Q3ValueList>
#include <Q3MemArray>
#endif

XmlParse::XmlParse()
{
}

XmlParse::~XmlParse()
{
}

int XmlParse::selectNodeInt(const QDomNode &nodeHeader, const QString sNode)
{
    return selectNode(nodeHeader, sNode).toElement().text().toInt();
}

float XmlParse::selectNodeFloat(const QDomNode &nodeHeader, const QString sNode)
{
    return selectNode(nodeHeader, sNode).toElement().text().toFloat();
}

QDomNode XmlParse::selectNode(const QDomNode &nodeHeader, const QString sNode)
{
    QDomNode node = nodeHeader.firstChild();
    while (!node.isNull())
    {
        if (node.nodeName() == sNode)
            return node;
        node = node.nextSibling();
    }
    return node;
}

QString XmlParse::selectNodeQString(const QDomNode &nodeHeader, const QString sNode)
{
    QString ret;
    QDomNode node = selectNode(nodeHeader, sNode);
    if (!node.isNull())
        ret = node.toElement().text();
    else
        ret = "";
    return ret;
}

#ifdef QT3_SUPPORT
Q3MemArray<long> *XmlParse::selectNodeLongArray(const QDomNode &nodeHeader, const QString sNode)
#else
QMemArray<long> *XmlParse::selectNodeLongArray(const QDomNode &nodeHeader, const QString sNode)
#endif
{
    QString s;
    QDomNode node = selectNode(nodeHeader, sNode);
    if (!node.isNull())
    {
        QDomNode node2 = selectNode(node, "#cdata-section");
        if (!node2.isNull())
            s = node2.toCDATASection().data();
    }
#ifdef QT3_SUPPORT
    Q3MemArray<long> *data = new Q3MemArray<long>(s.length()/4);
#else
    QMemArray<long> *data = new QMemArray<long>(s.length()/4);
#endif
    for (unsigned int i=0; i<s.length()/4; ++i)
    {    
        QChar c4 = s.at(i*4);
        QChar c3 = s.at(i*4+1);
        QChar c2 = s.at(i*4+2);
        QChar c1 = s.at(i*4+3);

#ifdef QT3_SUPPORT
 	long v = (long)((unsigned char)c1.latin1()) + (long)((unsigned char)c2.latin1())*0x100 + (long)((unsigned char)c3.latin1())*0x10000 + (long)((unsigned char)c4.latin1())*0x1000000;
#else
	long v = (long)((unsigned char)c1) + (long)((unsigned char)c2)*0x100 + (long)((unsigned char)c3)*0x10000 + (long)((unsigned char)c4)*0x1000000;
#endif

        data->at(i) = v;
    }
    return data;
}

#ifdef QT3_SUPPORT
Q3MemArray<char> *XmlParse::selectNodeCharArray(const QDomNode &nodeHeader, const QString sNode)
#else
QMemArray<char> *XmlParse::selectNodeCharArray(const QDomNode &nodeHeader, const QString sNode)
#endif
{
    QString s;
    QDomNode node = selectNode(nodeHeader, sNode);
    if (!node.isNull())
    {
        QDomNode node2 = selectNode(node, "#cdata-section");
        if (!node2.isNull())
            s = node2.toCDATASection().data();
    }
#ifdef QT3_SUPPORT
    Q3MemArray<char> *data = new Q3MemArray<char>(s.length());
    for (unsigned int i=0; i<s.length(); ++i)
        data->at(i) = s.at(i).latin1();
#else
    QMemArray<char> *data = new QMemArray<char>(s.length());
    for (unsigned int i=0; i<s.length(); ++i)
        data->at(i) = (QChar)(s.at(i));
#endif
    return data;
}

#ifdef QT3_SUPPORT
Q3ValueList<long> *XmlParse::selectNodeLongList(const QDomNode &nodeHeader, const QString sNode)
#else
QValueList<long> *XmlParse::selectNodeLongList(const QDomNode &nodeHeader, const QString sNode)
#endif
{
#ifdef QT3_SUPPORT
    Q3MemArray<long> *p = selectNodeLongArray(nodeHeader, sNode);    
    Q3ValueList<long> *data = new Q3ValueList<long>;
#else
    QMemArray<long> *p = selectNodeLongArray(nodeHeader, sNode);    
    QValueList<long> *data = new QValueList<long>;
#endif
    for (unsigned int i=0; i<p->size(); ++i)
        data->append(p->at(i));
    return data;
}

QDomElement XmlParse::addElement(QDomDocument &doc, QDomElement &header, QString sElementName, QString sText)
{
    QDomElement element = doc.createElement(sElementName);
    element.appendChild(doc.createTextNode(sText));
    header.appendChild(element);
    return element;
}

#ifdef QT3_SUPPORT
QDomElement XmlParse::addElement(QDomDocument &doc, QDomElement &header, QString sElementName, Q3ValueList<long> *pData)
#else
QDomElement XmlParse::addElement(QDomDocument &doc, QDomElement &header, QString sElementName, QValueList<long> *pData)
#endif
{
    // Create a string, binstring, that contains the data contained pointet to by pData, and save it in XML
    // by use of QDomCDATASection
    QString binstring;
    for (unsigned int i=0; i<pData->size(); ++i)
    {
        long v = (*pData->at(i));
	
        // Split long value into four chars
        unsigned char c1 = v&0x000000ff;
        unsigned char c2 = (v&0x0000ff00)>>8;
        unsigned char c3 = (v&0x00ff0000)>>16;
        unsigned char c4 = (v&0xff000000)>>24;
        
	binstring.append(c4);
        binstring.append(c3);
        binstring.append(c2);
        binstring.append(c1);
    }
    
    QDomElement element = doc.createElement(sElementName);
    element.appendChild(doc.createCDATASection(binstring));
    header.appendChild(element);
    return element;
}

#ifdef QT3_SUPPORT
QDomElement XmlParse::addElement(QDomDocument &doc, QDomElement &header, QString sElementName, Q3MemArray<char> *pData)
#else
QDomElement XmlParse::addElement(QDomDocument &doc, QDomElement &header, QString sElementName, QMemArray<char> *pData)
#endif
{
    // Create a string, binstring, that contains the data contained pointet to by pData, and save it in XML
    // by use of QDomCDATASection
    QString binstring;
    for (unsigned int i=0; i<pData->size(); ++i)
        binstring.append(pData->at(i));
    
    QDomElement element = doc.createElement(sElementName);
    element.appendChild(doc.createCDATASection(binstring));
    header.appendChild(element);
    return element;
}

/*
QDomElement XmlParse::addElement(QDomDocument &doc, QDomElement &header, QString sElementName, QValueList<int> *pData)
{
    // Create a string, binstring, that contains the data contained pointet to by pData, and save it in XML
    // by use of QDomCDATASection
    QString binstring;
    for (unsigned int i=0; i<pData->size(); ++i)
        binstring.append((unsigned char)(*pData->at(i)));

    QDomElement element = doc.createElement(sElementName);
    element.appendChild(doc.createCDATASection(binstring));
    header.appendChild(element);
    return element;
}
*/

