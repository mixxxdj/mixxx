#include <QtDebug>
#include <QStringList>
#include <QScriptValue>

#include "skin/skincontext.h"
#include "skin/svgparser.h"

SkinContext::SkinContext() {
}

SkinContext::SkinContext(const SkinContext& parent)
        : m_variables(parent.variables()),
          m_skinBasePath(parent.m_skinBasePath) {
    QScriptValue context = m_scriptEngine.currentContext()->activationObject();
    for (QHash<QString, QString>::const_iterator it = m_variables.begin();
         it != m_variables.end(); ++it) {
        context.setProperty(it.key(), it.value());
    }
}

SkinContext::~SkinContext() {
}

SkinContext& SkinContext::operator=(const SkinContext& other) {
    m_variables = other.variables();
    QScriptValue context = m_scriptEngine.currentContext()->activationObject();
    for (QHash<QString, QString>::const_iterator it = m_variables.begin();
         it != m_variables.end(); ++it) {
        context.setProperty(it.key(), it.value());
    }
    return *this;
}

QString SkinContext::variable(const QString& name) const {
    return m_variables.value(name, QString());
}

void SkinContext::setVariable(const QString& name, const QString& value) {
    m_variables[name] = value;
    QScriptValue context = m_scriptEngine.currentContext()->activationObject();
    context.setProperty(name, value);
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

bool SkinContext::hasNode(const QDomNode& node, const QString& nodeName) const {
    return !selectNode(node, nodeName).isNull();
}

QDomNode SkinContext::selectNode(const QDomNode& node,
                                 const QString& nodeName) const {
    QDomNode child = node.firstChild();
    while (!child.isNull()) {
        if (child.nodeName() == nodeName) {
            return child;
        }
        child = child.nextSibling();
    }
    return child;
}

QDomElement SkinContext::selectElement(const QDomNode& node,
                                       const QString& nodeName) const {
    QDomNode child = selectNode(node, nodeName);
    return child.toElement();
}

QString SkinContext::selectString(const QDomNode& node,
                                  const QString& nodeName) const {
    QDomElement child = selectElement(node, nodeName);
    return nodeToString(child);
}

float SkinContext::selectFloat(const QDomNode& node,
                               const QString& nodeName) const {
    bool ok = false;
    float conv = nodeToString(selectElement(node, nodeName)).toFloat(&ok);
    return ok ? conv : 0.0f;
}

double SkinContext::selectDouble(const QDomNode& node,
                                 const QString& nodeName) const {
    bool ok = false;
    double conv = nodeToString(selectElement(node, nodeName)).toDouble(&ok);
    return ok ? conv : 0.0;
}

int SkinContext::selectInt(const QDomNode& node,
                           const QString& nodeName,
                           bool* pOk) const {
    bool ok = false;
    int conv = nodeToString(selectElement(node, nodeName)).toInt(&ok);
    if (pOk != NULL) {
        *pOk = ok;
    }
    return ok ? conv : 0;
}

bool SkinContext::selectBool(const QDomNode& node,
                             const QString& nodeName,
                             bool defaultValue) const {
    QDomNode child = selectNode(node, nodeName);
    if (!child.isNull()) {
         QString stringValue = nodeToString(child);
        return stringValue.contains("true", Qt::CaseInsensitive);
    }
    return defaultValue;
}

bool SkinContext::hasNodeSelectString(const QDomNode& node,
                                      const QString& nodeName, QString *value) const {
    QDomNode child = selectNode(node, nodeName);
    if (!child.isNull()) {
        *value = nodeToString(child);
        return true;
    }
    return false;
}

bool SkinContext::hasNodeSelectBool(const QDomNode& node,
                                    const QString& nodeName, bool *value) const {
    QDomNode child = selectNode(node, nodeName);
    if (!child.isNull()) {
         QString stringValue = nodeToString(child);
        *value = stringValue.contains("true", Qt::CaseInsensitive);
        return true;
    }
    return false;
}

bool SkinContext::selectAttributeBool(const QDomElement& element,
                                      const QString& attributeName,
                                      bool defaultValue) const {
    if (element.hasAttribute(attributeName)) {
        QString stringValue = element.attribute(attributeName);
        return stringValue.contains("true", Qt::CaseInsensitive);
    }
    return defaultValue;
}

QString SkinContext::selectAttributeString(const QDomElement& element,
                                           const QString& attributeName,
                                           QString defaultValue) const {
    if (element.hasAttribute(attributeName)) {
        QString stringValue = element.attribute(attributeName);
        return stringValue == "" ? defaultValue :stringValue;
    }
    return defaultValue;
}

QString SkinContext::variableNodeToText(const QDomElement& variableNode) const {
    if (variableNode.hasAttribute("expression")) {
        QScriptValue result = m_scriptEngine.evaluate(
            variableNode.attribute("expression"));
        return result.toString();
    } else if (variableNode.hasAttribute("name")) {
        QString variableName = variableNode.attribute("name");
        if (variableNode.hasAttribute("format")) {
            QString formatString = variableNode.attribute("format");
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
    QStringList result;
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
    return result.join("");
}



QString SkinContext::getPixmapPath(const QDomNode& pixmapNode) const {
    QString pixmapPath, pixmapName;
    const SvgParser* pSvgParser = new SvgParser(*this);
    
    if (!pixmapNode.isNull()) {
        QDomNode svgNode = selectNode(pixmapNode, "svg");
        if (!svgNode.isNull()) {
            // inline svg
            pixmapPath = pSvgParser->saveToTempFile(
                pSvgParser->parseSvgTree(svgNode) );
        } else {
            // filename
            pixmapName = nodeToString(pixmapNode);
            if (!pixmapName.isEmpty()) {
                pixmapName = getSkinPath(pixmapName);
                if (pixmapName.endsWith(".svg", Qt::CaseInsensitive)) {
                    pixmapPath = pSvgParser->saveToTempFile(
                        pSvgParser->parseSvgFile(pixmapName) );
                } else {
                    pixmapPath = pixmapName;
                }
            }
        }
    }
    
    return pixmapPath;
}


PixmapSource* SkinContext::getPixmapSource(const QDomNode& pixmapNode) const {
    QString pixmapPath, pixmapName;
    PixmapSource* source = new PixmapSource();
    
    const SvgParser* pSvgParser = new SvgParser(*this);
    
    if (!pixmapNode.isNull()) {
        QDomNode svgNode = selectNode(pixmapNode, "svg");
        if (!svgNode.isNull()) {
            // inline svg
            const QByteArray rslt = pSvgParser->saveToQByteArray(
                    pSvgParser->parseSvgTree(svgNode) );
            source->setSVG( rslt );
        } else {
            // filename
            pixmapName = nodeToString(pixmapNode);
            if (!pixmapName.isEmpty()) {
                source->setPath( getSkinPath(pixmapName) );
                if (pixmapName.endsWith(".svg", Qt::CaseInsensitive)) {
                    const QByteArray rslt = pSvgParser->saveToQByteArray(
                            pSvgParser->parseSvgFile(source->getPath()) );
                    source->setSVG( rslt );
                } else {
                    source->setBitmap(getSkinPath(pixmapName));
                }
            }
        }
    }
    
    // qDebug() << "getPixmapSource out path" << source->getPath();
    return source;
}

QScriptValue SkinContext::evaluateScript(QString expression) {
    return m_scriptEngine.evaluate(expression);
}

QScriptEngine* SkinContext::getScriptEngine() const {
    return &m_scriptEngine;
}



