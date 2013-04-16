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

#include <QFile>
#include <QDebug>

XmlParse::XmlParse() {
}

XmlParse::~XmlParse() {
}

int XmlParse::selectNodeInt(const QDomNode &nodeHeader, const QString sNode) {
    return selectNode(nodeHeader, sNode).toElement().text().toInt();
}

float XmlParse::selectNodeFloat(const QDomNode &nodeHeader, const QString sNode) {
    return selectNode(nodeHeader, sNode).toElement().text().toFloat();
}

QDomNode XmlParse::selectNode(const QDomNode &nodeHeader, const QString sNode) {
    QDomNode node = nodeHeader.firstChild();
    while (!node.isNull()) {
        if (node.nodeName() == sNode)
            return node;
        node = node.nextSibling();
    }
    return node;
}

QString XmlParse::selectNodeQString(const QDomNode &nodeHeader, const QString sNode) {
    QString ret;
    QDomNode node = selectNode(nodeHeader, sNode);
    if (!node.isNull())
        ret = node.toElement().text();
    else
        ret = "";
    return ret;
}

QVector<long> * XmlParse::selectNodeLongArray(const QDomNode &nodeHeader, const QString sNode) {
    QString s;
    QDomNode node = selectNode(nodeHeader, sNode);
    if (!node.isNull()) {
        QDomNode node2 = selectNode(node, "#cdata-section");
        if (!node2.isNull()) {
            QDomCDATASection node2cdata = node2.toCDATASection();
            s = node2cdata.data();
        }
    }
    QVector<long> *data = new QVector<long>(s.length()/4);
    for (int i=0; i<s.length()/4; ++i) {
        unsigned char c4 = s.at(i*4).toLatin1();
        unsigned char c3 = s.at(i*4+1).toLatin1();
        unsigned char c2 = s.at(i*4+2).toLatin1();
        unsigned char c1 = s.at(i*4+3).toLatin1();

        long v = (long)(c1) + (long)(c2)*0x100 + (long)(c3)*0x10000 + (long)(c4)*0x1000000;

        (*data)[i] = v;
    }
    return data;
}

QVector<char> * XmlParse::selectNodeCharArray(const QDomNode &nodeHeader, const QString sNode) {
    QString s;
    QDomNode node = selectNode(nodeHeader, sNode);
    if (!node.isNull()) {
        QDomNode node2 = selectNode(node, "#cdata-section");
        if (!node2.isNull()) {
            QDomCDATASection node2cdata = node2.toCDATASection();
            s = node2cdata.data();
        }
    }
    QVector<char> *data = new QVector<char>(s.length());
    for (int i=0; i<s.length(); ++i)
        (*data)[i] = s.at(i).toLatin1();
    return data;
}

QVector<char> * XmlParse::selectNodeHexCharArray(const QDomNode &nodeHeader, const QString sNode) {
    QString hexdata;
    QDomNode node = selectNode(nodeHeader, sNode);
    if (node.isNull()) {
        return 0;
    }

    hexdata = node.toElement().text();
    int wavebytes = hexdata.length() / 2;
    if (wavebytes == 0) { 
        return 0;
    }

    QVector<char> *data = new QVector<char>(wavebytes);

    bool ok = true;
    for (int i=0; i<wavebytes; ++i) {
        int byte = hexdata.mid(i*2, 2).toInt(&ok, 16);
        (*data)[i] = (char)byte;
    }
    return data;
}

QList<long> * XmlParse::selectNodeLongList(const QDomNode &nodeHeader, const QString sNode) {
    QVector<long> *p = selectNodeLongArray(nodeHeader, sNode);
    QList<long> *data = new QList<long>;
    for (int i=0; i<p->size(); ++i)
        data->append(p->at(i));
    return data;
}

QDomElement XmlParse::addElement(QDomDocument &doc, QDomElement &header, QString sElementName, QString sText) {
    QDomElement element = doc.createElement(sElementName);
    element.appendChild(doc.createTextNode(sText));
    header.appendChild(element);
    return element;
}

QDomElement XmlParse::openXMLFile(QString path, QString name) {
    QDomDocument doc(name);
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open xml file:" << file.fileName();
        return QDomElement();
    }
    if (!doc.setContent(&file)) {
        qWarning() << "Error parsing xml file:" << file.fileName();
        file.close();
        return QDomElement();
    }

    file.close();
    return doc.documentElement();
}

QDomElement XmlParse::addElement(QDomDocument &doc, QDomElement &header, QString sElementName, QList<long> * pData) {
    // Create a string, binstring, that contains the data contained pointet to by pData, and save it in XML
    // by use of QDomCDATASection
    QString binstring;
    for (int i=0; i<pData->size(); ++i) {
        long v = (pData->at(i));

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

QDomElement XmlParse::addElement(QDomDocument &doc, QDomElement &header, QString sElementName, QVector<char> * pData) {
    // Create a string, binstring, that contains the data contained pointet to by pData, and save it in XML
    // by use of QDomCDATASection
    QString binstring;
    for (int i=0; i<pData->size(); ++i)
        binstring.append(pData->at(i));

    QDomElement element = doc.createElement(sElementName);
    element.appendChild(doc.createCDATASection(binstring));
    header.appendChild(element);
    return element;
}

QDomElement XmlParse::addHexElement(QDomDocument &doc, QDomElement &header, QString sElementName, QVector<char> * pData) {
    QDomElement element = doc.createElement(sElementName);

    QString hexdata("");
    QVector<char>::ConstIterator ci;
    for (ci = pData->begin(); ci != pData->end(); ci++) {
        char raw = *ci;
        hexdata.append(QString::number((raw & 0xf0) >> 4, 16));
        hexdata.append(QString::number(raw & 0x0f, 16));
    }

    element.appendChild(doc.createTextNode(hexdata));
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

