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

QValueList<int> *XmlParse::selectNodeIntList(const QDomNode &nodeHeader, const QString sNode)
{
    QString s;
    QDomNode node = selectNode(nodeHeader, sNode);
    if (!node.isNull())
    {
        QDomNode node2 = selectNode(node, "#cdata-section");
        if (!node2.isNull())
            s = node2.toCDATASection().data();
    }

    QValueList<int> *data = new QValueList<int>;
    for (unsigned int i=0; i<s.length(); ++i)
        data->append((unsigned char)(QChar)(s.at(i)));
    return data;
}

QDomElement XmlParse::addElement(QDomDocument &doc, QDomElement &header, QString sElementName, QString sText)
{
    QDomElement element = doc.createElement(sElementName);
    element.appendChild(doc.createTextNode(sText));
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

