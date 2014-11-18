#include <QtDebug>
#include <QStringList>
#include <QScriptValue>
#include <QTemporaryFile>

#include "skin/skincontext.h"

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


// look for the document of a node
QDomDocument SkinContext::getDocument(const QDomNode& node) const {
    
    QDomDocument document;
    QDomNode parentNode = node;
    while( !parentNode.isNull() ){
        if( parentNode.isDocument() )
            document = parentNode.toDocument();
        parentNode = parentNode.parentNode();
    }
    
    return document;
}


// replaces Variables nodes in an svg dom tree
QString SkinContext::setVariablesInSvg(const QDomNode& svgSkinNode) const {
    
    // clone svg to don't alter xml input
    QDomNode svgNode = svgSkinNode.cloneNode(true);
    QDomDocument document = getDocument(svgNode);
    QDomElement svgElement = svgNode.toElement();
    
    // replace variables
    QDomNodeList variablesElements = svgElement.elementsByTagName("Variable");
    uint variableIndex;
    QDomElement varElement;
    QString varName, varValue;
    QDomNode varNode, varParentNode, oldChild;
    QDomText varValueNode;
    
    for (variableIndex=0; variableIndex < variablesElements.length(); variableIndex++){
        
        // retrieve value
        varNode = variablesElements.item(variableIndex);
        varElement = varNode.toElement();
        varName = varElement.attribute("name");
        varValue = variable(varName);
        
        // replace node by its value
        varParentNode = varNode.parentNode();
        varValueNode = document.createTextNode(varValue);
        oldChild = varParentNode.replaceChild( varValueNode, varNode );
        
        if( oldChild.isNull() ){
            // replaceChild has a really weird behaviour so I add this check
            qDebug() << "SVG : unable to replace dom node changed. \n";
        }
    }
    
    // Save the new svg in a temp file to use it with setPixmap
    QTemporaryFile svgFile;
    svgFile.setFileTemplate(QDir::temp().filePath("qt_temp.XXXXXX.svg"));
    
    // the file will be removed before being parsed in skin if set to true
    svgFile.setAutoRemove( false );
    
    QString svgTempFileName;
    if( svgFile.open() ){
        // qWarning() << "SVG : Temp filename" << svgFile.fileName() << " \n";
        QTextStream out(&svgFile);
        svgNode.save( out, -1 );
        svgFile.close();
        svgTempFileName = svgFile.fileName();
    } else {
        qDebug() << "Unable to open temp file for inline svg \n";
    }
    
    return svgTempFileName;
}

QString SkinContext::getPixmapPath(const QDomNode& pixmapNode) const {
    QString pixmapPath, pixmapName;
    
    if (!pixmapNode.isNull()) {
        QDomNode svgNode = selectNode(pixmapNode, "svg");
        if (!svgNode.isNull()) {
            // inline svg
            pixmapPath = setVariablesInSvg(svgNode);
        } else {
            // filename
            pixmapName = nodeToString(pixmapNode);
            if (!pixmapName.isEmpty()) {
                pixmapName = getSkinPath(pixmapName);
                if (pixmapName.endsWith(".svg", Qt::CaseInsensitive)) {
                    
                    QFile* file = new QFile(pixmapName);
                    if(file->open(QIODevice::ReadWrite | QIODevice::Text)){
                        QDomDocument document;
                        document.setContent(file);
                        QDomNode svgNode = document.elementsByTagName("svg").item(0);
                        
                        pixmapPath = setVariablesInSvg(svgNode);
                        file->close();
                    }
                } else {
                    pixmapPath = pixmapName;
                }
            }
        }
    }
    
    return pixmapPath;
}
