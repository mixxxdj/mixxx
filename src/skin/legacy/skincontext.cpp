#include <QtDebug>
#include <QStringList>
#include <QAction>

#include "skin/legacy/skincontext.h"
#include "util/cmdlineargs.h"
#include "util/math.h"

SkinContext::SkinContext(UserSettingsPointer pConfig,
                         const QString& xmlPath)
        : m_pConfig(pConfig),
          m_xmlPath(xmlPath),
          m_pSharedState(std::make_shared<SharedState>()),
          m_scaleFactor(1.0) {
}

SkinContext::SkinContext(const SkinContext* parent)
        : m_pConfig(parent->m_pConfig),
          m_xmlPath(parent->m_xmlPath),
          m_skinBasePath(parent->m_skinBasePath),
          m_pSharedState(parent->m_pSharedState),
          m_variables(parent->variables()),
          m_hookRx(parent->m_hookRx),
          m_scaleFactor(parent->m_scaleFactor) {
}

QString SkinContext::variable(const QString& name) const {
    return m_variables.value(name, QString());
}

void SkinContext::setVariable(const QString& name, const QString& value) {
    m_variables[name] = value;
}

void SkinContext::setXmlPath(const QString& xmlPath) {
    m_xmlPath = xmlPath;
}

bool SkinContext::hasVariableUpdates(const QDomNode& node) const {
    QDomNode child = node.firstChild();
    while (!child.isNull()) {
        if (child.isElement() && child.nodeName() == "SetVariable") {
            return true;
        }
        child = child.nextSibling();
    }
    return false;
}

void SkinContext::updateVariables(const QDomNode& node) {
    QDomNode child = node.firstChild();
    while (!child.isNull()) {
        if (child.isElement() && child.nodeName() == "SetVariable") {
            updateVariable(child.toElement());
        }
        child = child.nextSibling();
    }
}

void SkinContext::updateVariable(const QDomElement& element) {
    if (!element.hasAttribute("name")) {
        qDebug() << "Can't update variable without name:" << element.text();
        return;
    }
    QString name = element.attribute("name");
    QString value = variableNodeToText(element);
    setVariable(name, value);
}

QString SkinContext::variableNodeToText(const QDomElement& variableNode) const {
    QString variableName = variableNode.attribute("name");
    if (!variableName.isNull()) {
        QString formatString = variableNode.attribute("format");
        if (!formatString.isNull()) {
            return formatString.arg(variable(variableName));
        } else if (variableNode.nodeName() == "SetVariable") {
            // If we are setting the variable name and we didn't get a format
            // string then return the node text. Use nodeToString to translate
            // embedded variable references.
            return nodeToString(variableNode);
        } else {
            return variable(variableName);
        }
    }
    return nodeToString(variableNode);
}

QString SkinContext::nodeToString(const QDomNode& node) const {
    QString result;
    QDomNode child = node.firstChild();
    while (!child.isNull()) {
        if (child.isElement()) {
            if (child.nodeName() == "Variable") {
                result.append(variableNodeToText(child.toElement()));
            } else {
                qDebug() << "Unhandled tag in node:" << child.nodeName();
            }
        } else if (child.isText()) {
            result.append(child.nodeValue());
        }
        // Ignore all other node types.
        child = child.nextSibling();
    }
    return result;
}

PixmapSource SkinContext::getPixmapSource(const QDomNode& pixmapNode) const {
    return getPixmapSourceInner(nodeToString(pixmapNode));
}

PixmapSource SkinContext::getPixmapSource(const QString& filename) const {
    return getPixmapSourceInner(filename);
}

QDomElement SkinContext::loadSvg(const QString& filename) const {
    QDomElement& cachedSvg = m_pSharedState->svgCache[filename];
    if (cachedSvg.isNull()) {
        QFile file(filename);
        if (file.open(QIODevice::ReadOnly|QIODevice::Text)) {
            QDomDocument document;
            if (!document.setContent(&file)) {
                qDebug() << "ERROR: Failed to set content on QDomDocument";
            }
            cachedSvg = document.elementsByTagName("svg").item(0).toElement();
            file.close();
        }
    }
    return cachedSvg;
}

PixmapSource SkinContext::getPixmapSourceInner(const QString& filename) const {
    if (!filename.isEmpty()) {
        return PixmapSource(makeSkinPath(filename));
    }
    return PixmapSource();
}

QDebug SkinContext::logWarning(const char* file, const int line,
                               const QDomNode& node) const {
    return qWarning() << QString("%1:%2 SKIN ERROR at %3:%4 <%5>:")
                             .arg(file, QString::number(line), m_xmlPath,
                                  QString::number(node.lineNumber()),
                                  node.nodeName())
                             .toUtf8()
                             .constData();
}

int SkinContext::scaleToWidgetSize(QString& size) const {
    bool widthOk = false;
    double dSize = size.toDouble(&widthOk);
    if (widthOk && dSize >= 0) {
        return static_cast<int>(round(dSize * m_scaleFactor));
    }
    return -1;
}
