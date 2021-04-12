#include <QFile>
#include <QtDebug>

#include "util/xml.h"
#include "errordialoghandler.h"

int XmlParse::selectNodeInt(const QDomNode& nodeHeader,
                            const QString& sNode, bool* ok) {
    return selectNode(nodeHeader, sNode).toElement().text().toInt(ok);
}

float XmlParse::selectNodeFloat(const QDomNode& nodeHeader,
                                const QString& sNode, bool* ok) {
    return selectNode(nodeHeader, sNode).toElement().text().toFloat(ok);
}

double XmlParse::selectNodeDouble(const QDomNode& nodeHeader,
                                  const QString& sNode, bool* ok) {
    return selectNode(nodeHeader, sNode).toElement().text().toDouble(ok);
}

QDomNode XmlParse::selectNode(const QDomNode& nodeHeader,
                              const QString& sNode) {
    QDomNode node = nodeHeader.firstChild();
    while (!node.isNull()) {
        if (node.nodeName() == sNode) {
            return node;
        }
        node = node.nextSibling();
    }
    return node;
}

QDomElement XmlParse::selectElement(const QDomNode& nodeHeader,
                                    const QString& sNode) {
    QDomNode node = nodeHeader.firstChild();
    while (!node.isNull()) {
        if (node.nodeName() == sNode) {
            if (node.isElement()) {
                return node.toElement();
            } else {
                break;
            }
        }
        node = node.nextSibling();
    }
    return QDomElement();
}

QString XmlParse::selectNodeQString(const QDomNode& nodeHeader,
                                    const QString& sNode) {
    QDomNode node = selectNode(nodeHeader, sNode);
    if (!node.isNull()) {
        return node.toElement().text();
    }
    return QString("");
}

QDomElement XmlParse::openXMLFile(const QString& path, const QString& name) {
    QDomDocument doc(name);
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open xml file:" << file.fileName();
        return QDomElement();
    }
    QString error;
    int line, col;
    if (!doc.setContent(&file, &error, &line, &col)) {
        QString errorString = QString("%1 at line %2, column %3")
                                .arg(error).arg(line).arg(col);

        QString errorLog = QString("Error parsing XML file %1: %2")
                            .arg(file.fileName(), errorString);
        qWarning() << errorLog;

        // Set up error dialog
        ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
        props->setType(DLG_WARNING);
        props->setTitle("Error parsing XML file");
        props->setText(QString("There was a problem parsing the XML file %1.").arg(file.fileName()));
        props->setInfoText(errorString);
        props->setModal(false); // Don't block the GUI

        // Display it
        ErrorDialogHandler* dialogHandler = ErrorDialogHandler::instance();
        if (dialogHandler) {
            dialogHandler->requestErrorDialog(props);
        }

        file.close();
        return QDomElement();
    }

    file.close();
    return doc.documentElement();
}

QDomElement XmlParse::addElement(QDomDocument& doc,
                                 QDomElement& header,
                                 const QString& sElementName,
                                 const QString& sText) {
    QDomElement element = doc.createElement(sElementName);
    element.appendChild(doc.createTextNode(sText));
    header.appendChild(element);
    return element;
}
