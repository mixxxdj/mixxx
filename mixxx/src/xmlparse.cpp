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

QMemArray<long> *XmlParse::selectNodeLongArray(const QDomNode &nodeHeader, const QString sNode)
{
    QString s;
    QDomNode node = selectNode(nodeHeader, sNode);
    if (!node.isNull())
    {
        QDomNode node2 = selectNode(node, "#cdata-section");
        if (!node2.isNull())
            s = node2.toCDATASection().data();
    }

    QMemArray<long> *data = new QMemArray<long>(s.length()/4);
    for (unsigned int i=0; i<s.length()/4; ++i)
    {    
        QChar c4 = s.at(i*4);
        QChar c3 = s.at(i*4+1);
        QChar c2 = s.at(i*4+2);
        QChar c1 = s.at(i*4+3);

	long v = (long)((unsigned char)c1) + (long)((unsigned char)c2)*0x100 + (long)((unsigned char)c3)*0x10000 + (long)((unsigned char)c4)*0x1000000;

        data->at(i) = v;
    }
    return data;
}

QMemArray<char> *XmlParse::selectNodeCharArray(const QDomNode &nodeHeader, const QString sNode)
{
    QString s;
    QDomNode node = selectNode(nodeHeader, sNode);
    if (!node.isNull())
    {
        QDomNode node2 = selectNode(node, "#cdata-section");
        if (!node2.isNull())
            s = node2.toCDATASection().data();
    }

    QMemArray<char> *data = new QMemArray<char>(s.length());
    for (unsigned int i=0; i<s.length(); ++i)
        data->at(i) = (QChar)(s.at(i));
    return data;
}

QValueList<long> *XmlParse::selectNodeLongList(const QDomNode &nodeHeader, const QString sNode)
{
    QMemArray<long> *p = selectNodeLongArray(nodeHeader, sNode);
    
    QValueList<long> *data = new QValueList<long>;
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

QDomElement XmlParse::addElement(QDomDocument &doc, QDomElement &header, QString sElementName, QValueList<long> *pData)
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

QDomElement XmlParse::addElement(QDomDocument &doc, QDomElement &header, QString sElementName, QMemArray<char> *pData)
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

