#include <QtDebug>
#include <QStringList>
#include <QScriptValue>
#include <QTemporaryFile>

#include "skin/svgparser.h"

SvgParser::SvgParser() {
}

SvgParser::SvgParser(const SkinContext& parent)
        : m_variables(parent.variables()) {
    QScriptValue context = m_scriptEngine.currentContext()->activationObject();
    for (QHash<QString, QString>::const_iterator it = m_variables.begin();
            it != m_variables.end(); ++it) {
        context.setProperty(it.key(), it.value());
    }
    
    m_pContext = &parent;
}

SvgParser::~SvgParser() {
}

// look for the document of a node
QDomDocument SvgParser::getDocument(const QDomNode& node) const {
    
    QDomDocument document;
    QDomNode parentNode = node;
    while (!parentNode.isNull()) {
        if (parentNode.isDocument()) {
            document = parentNode.toDocument();
        }
        parentNode = parentNode.parentNode();
    }
    
    return document;
}

QDomNode SvgParser::parseSvgFile(const QString& svgFileName) const {
    QFile* file = new QFile(svgFileName);
    QDomNode out;
    if (file->open(QIODevice::ReadWrite|QIODevice::Text)) {
        QDomDocument document;
        document.setContent(file);
        QDomNode svgNode = document.elementsByTagName("svg").item(0);
        out = parseSvgTree(svgNode);
        file->close();
        return out;
    }
    
    return out;
}

QDomNode SvgParser::parseSvgTree(const QDomNode& svgSkinNode) const {
    
    // clone svg to don't alter xml input
    QDomNode svgNode = svgSkinNode.cloneNode(true);
    
    parseVariableElements(svgNode);
    parseScriptElements(svgNode);
    scanTree(svgNode, &SvgParser::parseAttributes);
    
    // return saveToTempFile(svgNode);
    return svgNode;
}

QString SvgParser::saveToTempFile(const QDomNode& svgNode) const {
    
    // Save the new svg in a temp file to use it with setPixmap
    QTemporaryFile svgFile;
    svgFile.setFileTemplate(QDir::temp().filePath("qt_temp.XXXXXX.svg"));
    
    // the file will be removed before being parsed in skin if set to true
    svgFile.setAutoRemove(false);
    
    QString svgTempFileName;
    if (svgFile.open()) {
        // qWarning() << "SVG : Temp filename" << svgFile.fileName() << " \n";
        QTextStream out(&svgFile);
        svgNode.save(out, 2);
        svgFile.close();
        svgTempFileName = svgFile.fileName();
    } else {
        qDebug() << "Unable to open temp file for inline svg \n";
    }
    
    return svgTempFileName;
}

QByteArray SvgParser::saveToQByteArray(const QDomNode& svgNode) const {
    QByteArray out;
    QTextStream textStream(&out);
    svgNode.save(textStream, 2);
    return out;
}

// replaces Variables nodes in an svg dom tree
void SvgParser::parseVariableElements(const QDomNode& svgNode) const {
    
    QDomDocument document = getDocument(svgNode);
    QDomElement svgElement = svgNode.toElement();
    
    // replace variables
    QDomNodeList variablesElements = svgElement.elementsByTagName("Variable");
    int i=variablesElements.length()-1;
    QDomElement varElement;
    QString varName, varValue;
    QDomNode varNode, varParentNode, oldChild;
    QDomText varValueNode;
    
    while (i >= 0 && (varNode = variablesElements.item(i)).isNull() == false) {
        
        // retrieve value
        varElement = varNode.toElement();
        varName = varElement.attribute("name");
        varValue = m_pContext->variable(varName);
        
        // replace node by its value
        varParentNode = varNode.parentNode();
        varValueNode = document.createTextNode(varValue);
        oldChild = varParentNode.replaceChild(varValueNode, varNode);
        
        if (oldChild.isNull()) {
            // replaceChild has a really weird behaviour so I add this check
            qDebug() << "SVG : unable to replace dom node changed. \n";
        }
        
        --i;
    }
}


void SvgParser::parseAttributes(const QDomNode& node) const {
    QDomNamedNodeMap attributes = node.attributes();
    QDomElement element = node.toElement();
    uint i;
    int pos;
    QString varName, varValue, attributeValue, attributeName,
            propName, match, replacement;
    QStringList captured;
    QDomAttr attribute;
    
    
    QScriptValue global = m_scriptEngine.globalObject();
    QScriptValue hookNames;
    QString hooksPattern;
    QRegExp hookRx, nameRx;
    
    hookNames = global.property("hookNames").call(global);
    
    if (hookNames.toString().length()) {
        hooksPattern = hookNames.property("toPattern")
            .call(hookNames).toString();
        
        // hook_name( arg1 [, arg2]... )
        hookRx.setPattern("("+hooksPattern+")\\(([^\\(\\)]+)\\)\\s*;?");
        // qDebug() <<  "hooksPattern : " << hooksPattern << "\n";
    }
    
    // expr-attribute_name="var_name";
    nameRx.setPattern("^expr-([^=\\s]+)$");
    
    for (i=0; i < attributes.length(); i++) {
        
        attribute = attributes.item(i).toAttr();
        attributeValue = attribute.value();
        attributeName = attribute.name();
        
        // searching variable attributes :
        // expr-attribute_name="variable_name|expression"
        if (nameRx.indexIn(attributeName) != -1) {
            varValue = evaluateTemplateExpression(attributeValue).toString();
            if (varValue.length()) {
                element.setAttribute(nameRx.cap(1), varValue);
            }
            continue;
        }
        
        if (!hookRx.isEmpty()) {
            // searching hooks in the attribute value
            pos = 0;
            while ((pos = hookRx.indexIn(attributeValue, pos)) != -1) {
                captured = hookRx.capturedTexts();
                match = hookRx.cap(0);
                QString tmp = "templateHooks." + match;
                // qDebug() <<  "expression : " << tmp << "\n";
                replacement = evaluateTemplateExpression(tmp).toString();
                attributeValue.replace(pos, match.length(), replacement);
                pos += replacement.length();
            }
        }
        
        attribute.setValue(attributeValue);
    }
    
}

void SvgParser::scanTree(const QDomNode& node, void (SvgParser::*callback)(const QDomNode& node)const) const {
    
    (this->*callback)(node);
    QDomNodeList children = node.childNodes();
    QDomNode child;
    uint i;
    
    for (i=0; i<children.length(); i++) {
        child = children.at(i);
        if (child.isElement()) {
            scanTree( child, callback);
        }
    }
}

void SvgParser::parseScriptElements(const QDomNode& svgSkinNode) const {
    
    // parse script content
    QDomElement svgElement = svgSkinNode.toElement();
    QDomNodeList scriptElements = svgElement.elementsByTagName("script");
    int i = 0;
    QString expression, scriptPath;
    QDomNode scriptNode;
    QScriptValue result;
    
    while (!(scriptNode = scriptElements.item(i)).isNull() && ++i) {
        if (!(scriptPath = scriptNode.toElement().attribute("src")).isNull()) {
            QFile scriptFile(m_pContext->getSkinPath(scriptPath));
            scriptFile.open(QIODevice::ReadOnly|QIODevice::Text);
            QTextStream in(&scriptFile);
            result = m_scriptEngine.evaluate(in.readAll());
            if (m_scriptEngine.hasUncaughtException()) {
                qDebug() << "SVG script exception : " << result.toString()
                        << "in" << scriptPath;
            }
        }
        
        expression = m_pContext->nodeToString(scriptNode);
        result = m_scriptEngine.evaluate(expression);
        if (m_scriptEngine.hasUncaughtException()) {
            qDebug() << "SVG script exception : " << result.toString();
        }
    }
    
}

QScriptValue SvgParser::evaluateTemplateExpression(QString expression) const {
    QScriptValue out = m_scriptEngine.evaluate(expression);
    if (m_scriptEngine.hasUncaughtException()) {
        qDebug()
            << "SVG script exception : " << out.toString()
            << "Empty string returned";
        
        // return an empty string as remplacement for the in-attribute expression
        return m_scriptEngine.nullValue();
    } else {
        return out;
    }
}
