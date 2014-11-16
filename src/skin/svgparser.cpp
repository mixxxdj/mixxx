#include <QtDebug>
#include <QStringList>
#include <QScriptValue>
#include <QTemporaryFile>

#include "skin/svgparser.h"

SvgParser::SvgParser(const SkinContext& parent)
        : m_context(parent) {
    m_context.importScriptExtension("console");
    m_context.importScriptExtension("svg");
}

SvgParser::~SvgParser() {
}

QDomNode SvgParser::parseSvgFile(const QString& svgFileName) const {
    m_currentFile = svgFileName;
    QFile file(svgFileName);
    QDomNode svgNode;
    if (file.open(QIODevice::ReadOnly|QIODevice::Text)) {
        QDomDocument document;
        document.setContent(&file);
        svgNode = document.elementsByTagName("svg").item(0);
        scanTree(svgNode);
        file.close();
    }
    return svgNode;
}

QDomNode SvgParser::parseSvgTree(const QDomNode& svgSkinNode,
                                 const QString& sourcePath) const {
    m_currentFile = sourcePath;
    // clone svg to don't alter xml input
    QDomNode svgNode = svgSkinNode.cloneNode(true);
    scanTree(svgNode);
    return svgNode;
}

void SvgParser::scanTree(QDomNode& node) const {
    parseElement(node);
    QDomNodeList children = node.childNodes();
    for (uint i=0; i<children.length(); i++) {
        QDomNode child = children.at(i);
        if (child.isElement()) {
            scanTree(child);
        }
    }
}

// replaces Variables nodes in an svg dom tree
void SvgParser::parseElement(QDomNode& node) const {
    QDomElement element = node.toElement();
    
    parseAttributes(node);
    
    if (element.tagName() == "text"){
        if (element.hasAttribute("value")){
            QString expression = element.attribute("value");
            QString result = evaluateTemplateExpression(
                expression, node.lineNumber() ).toString();
            
            if (!result.isNull()){
                QDomNodeList children = node.childNodes();
                for (uint i=0; i<children.length(); i++) {
                    node.removeChild(children.at(i));
                }
                
                QDomNode newChild = node.ownerDocument().createTextNode(result);
                node.appendChild(newChild);
            }
        }
        
    } else if (element.tagName() == "Variable"){
        QString value;
        if (element.hasAttribute("expression")){
            QString expression = element.attribute("expression");
            value = evaluateTemplateExpression(
                expression, node.lineNumber() ).toString();
        } else if (element.hasAttribute("name")){
            /* TODO (jclaveau) : Getting the variable fro the context or the
             * script engine have the same result here (in the skin context two)
             * Isn't it useless?
             * m_context.variable(name) <=> m_scriptEngine.evaluate(name)
             */
            value = m_context.variable(element.attribute("name"));
        }
        
        if (!value.isNull()){
            // replace node by its value
            QDomNode varParentNode = node.parentNode();
            QDomNode varValueNode = node.ownerDocument().createTextNode(value);
            QDomNode oldChild = varParentNode.replaceChild(varValueNode, node);
            
            if (oldChild.isNull()) {
                // replaceChild has a really weird behaviour so I add this check
                qDebug() << "SVG : unable to replace dom node changed. \n";
            }
        }
        
    } else if (element.tagName() == "script"){
        // Look for a filepath in the "src" attribute
        QString scriptPath = node.toElement().attribute("src");
        if (!scriptPath.isNull()) {
            QFile scriptFile(m_context.getSkinPath(scriptPath));
            scriptFile.open(QIODevice::ReadOnly|QIODevice::Text);
            QTextStream in(&scriptFile);
            QScriptValue result = m_context.evaluateScript(in.readAll(),
                                                           scriptPath);
        }
        // Evaluates the content of the script element
        QString expression = m_context.nodeToString(node);
        QScriptValue result = m_context.evaluateScript(
            expression, m_currentFile, node.lineNumber());
    }
}


void SvgParser::parseAttributes(QDomNode& node) const {
    
    QDomNamedNodeMap attributes = node.attributes();
    QDomElement element = node.toElement();
    
    // Retrieving hooks pattern from script extension
    QScriptValue global = m_context.getScriptEngine().globalObject();
    QScriptValue hooksPattern = global.property("svg")
        .property("getHooksPattern").call(global.property("svg"));
    QRegExp hookRx;
    if (!hooksPattern.isNull())
        hookRx.setPattern(hooksPattern.toString());
    
    // expr-attribute_name="var_name";
    QRegExp nameRx("^expr-([^=\\s]+)$");
    // TODO (jclaveau) : move this pattern definition to the script extension?
    for (uint i=0; i < attributes.length(); i++) {
        
        QDomAttr attribute = attributes.item(i).toAttr();
        QString attributeValue = attribute.value();
        QString attributeName = attribute.name();
        
        // searching variable attributes :
        // expr-attribute_name="variable_name|expression"
        if (nameRx.indexIn(attributeName) != -1) {
            QString varValue = evaluateTemplateExpression(
                attributeValue, node.lineNumber()).toString();
            if (varValue.length()) {
                element.setAttribute(nameRx.cap(1), varValue);
            }
            continue;
        }
        
        if (!hookRx.isEmpty()) {
            // searching hooks in the attribute value
            int pos = 0;
            while ((pos = hookRx.indexIn(attributeValue, pos)) != -1) {
                QStringList captured = hookRx.capturedTexts();
                QString match = hookRx.cap(0);
                QString tmp = "svg.templateHooks." + match;
                QString replacement = evaluateTemplateExpression(
                    tmp, node.lineNumber()).toString();
                attributeValue.replace(pos, match.length(), replacement);
                pos += replacement.length();
            }
        }
        
        attribute.setValue(attributeValue);
    }
}

QString SvgParser::saveToTempFile(const QDomNode& svgNode) const {
    // TODO (jclaveau) : this method could be removed if the Spinny widget
    // didn't require it. 
    
    // Save the new svg in a temp file to use it with setPixmap
    QTemporaryFile svgFile;
    svgFile.setFileTemplate(QDir::temp().filePath("qt_temp.XXXXXX.svg"));
    // the file will be removed before being parsed in skin if set to true
    svgFile.setAutoRemove(false);
    
    if (svgFile.open()) {
        QTextStream out(&svgFile);
        svgNode.save(out, 2);
        svgFile.close();
    } else {
        qDebug() << "Unable to open temp file for inline svg \n";
    }
    
    return svgFile.fileName();
}

QByteArray SvgParser::saveToQByteArray(const QDomNode& svgNode) const {
    // TODO (jclaveau) : a way to look the svg after the parsing would be nice!
    QByteArray out;
    QTextStream textStream(&out);
    // TODO (jclaveau) : saving crashes if there are comments before 
    // the svg root node. Need help :D
    svgNode.save(textStream, 2);
    
    return out;
}

QScriptValue SvgParser::evaluateTemplateExpression(const QString& expression,
                                                   int lineNumber) const {
    QScriptValue out = m_context.evaluateScript(
        expression, m_currentFile, lineNumber);
    if (m_context.getScriptEngine().hasUncaughtException()) {
        // return an empty string as replacement for the in-attribute expression
        QScriptValue nullValue;
        return nullValue;
    } else {
        return out;
    }
}
