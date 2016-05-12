#include <QtDebug>
#include <QStringList>
#include <QScriptValue>

#include "skin/svgparser.h"

SvgParser::SvgParser(const SkinContext& parent)
        : m_parentContext(parent) {
}

SvgParser::~SvgParser() {
}

QDomNode SvgParser::parseSvgTree(const QDomNode& svgSkinNode,
                                 const QString& sourcePath) const {
    m_currentFile = sourcePath;
    // clone svg to don't alter xml input
    QDomElement svgNode = svgSkinNode.cloneNode(true).toElement();
    scanTree(&svgNode);
    return svgNode;
}

void SvgParser::scanTree(QDomElement* node) const {
    parseElement(node);
    QDomNodeList children = node->childNodes();
    for (int i = 0; i < children.count(); ++i) {
        QDomElement child = children.at(i).toElement();
        if (!child.isNull()) {
            scanTree(&child);
        }
    }
}

void SvgParser::parseElement(QDomElement* element) const {
    parseAttributes(element);

    QString tagName = element->tagName();
    if (tagName == "text") {
        if (element->hasAttribute("value")) {
            QString expression = element->attribute("value");
            QString result = evaluateTemplateExpression(
                expression, element->lineNumber()).toString();

            if (!result.isNull()) {
                QDomNodeList children = element->childNodes();
                for (int i = 0; i < children.count(); ++i) {
                    element->removeChild(children.at(i));
                }

                QDomNode newChild = element->ownerDocument().createTextNode(result);
                element->appendChild(newChild);
            }
        }

    } else if (tagName == "Variable") {
        QString value;
        if (element->hasAttribute("expression")) {
            QString expression = element->attribute("expression");
            value = evaluateTemplateExpression(
                expression, element->lineNumber()).toString();
        } else if (element->hasAttribute("name")) {
            /* TODO (jclaveau) : Getting the variable from the context or the
             * script engine have the same result here (in the skin context two)
             * Isn't it useless?
             * m_context.variable(name) <=> m_scriptEngine.evaluate(name)
             */
            value = m_parentContext.variable(element->attribute("name"));
        }

        if (!value.isNull()) {
            // replace node by its value
            QDomNode varParentNode = element->parentNode();
            QDomNode varValueNode = element->ownerDocument().createTextNode(value);
            QDomNode oldChild = varParentNode.replaceChild(varValueNode, *element);

            if (oldChild.isNull()) {
                // replaceChild has a really weird behaviour so I add this check
                qDebug() << "SVG : unable to replace dom node changed. \n";
            }
        }

    } else if (tagName == "script") {
        // Look for a filepath in the "src" attribute
        // QString scriptPath = element->toElement().attribute("src");

        auto childContext = lazyChildContext();

        QString scriptPath = element->attribute("src");
        if (!scriptPath.isNull()) {
            QFile scriptFile(childContext.getSkinPath(scriptPath));
            if (!scriptFile.open(QIODevice::ReadOnly|QIODevice::Text)) {
                qDebug() << "ERROR: Failed to open script file";
            }
            QTextStream in(&scriptFile);
            QScriptValue result = childContext.evaluateScript(
                in.readAll(), scriptPath);
        } else {
            // Evaluates the content of the script element
            // QString expression = m_context.nodeToString(*element);
            QScriptValue result = childContext.evaluateScript(
                element->text(), m_currentFile, element->lineNumber());
        }
    }
}


void SvgParser::parseAttributes(QDomElement* element) const {
    QDomNamedNodeMap attributes = element->attributes();

    // expr-attribute_name="var_name";
    static QRegExp nameRx("^expr-([^=\\s]+)$");
    // TODO (jclaveau) : move this pattern definition to the script extension?
    for (int i = 0; i < attributes.count(); i++) {
        QDomAttr attribute = attributes.item(i).toAttr();
        QString attributeValue = attribute.value();
        QString attributeName = attribute.name();

        // searching variable attributes :
        // expr-attribute_name="variable_name|expression"
        if (nameRx.indexIn(attributeName) != -1) {
            QString varValue = evaluateTemplateExpression(
                attributeValue, element->lineNumber()).toString();
            if (!varValue.isEmpty()) {
                element->setAttribute(nameRx.cap(1), varValue);
            }
            continue;
        }

        const QRegExp& hookRx = m_parentContext.getHookRegex();
        if (!hookRx.isEmpty()) {
            // searching hooks in the attribute value
            int pos = 0;
            while ((pos = hookRx.indexIn(attributeValue, pos)) != -1) {
                QStringList captured = hookRx.capturedTexts();
                QString match = hookRx.cap(0);
                QString tmp = "svg.templateHooks." + match;
                QString replacement = evaluateTemplateExpression(
                    tmp, element->lineNumber()).toString();
                attributeValue.replace(pos, match.length(), replacement);
                pos += replacement.length();
            }
            attribute.setValue(attributeValue);
        }
    }
}

QByteArray SvgParser::saveToQByteArray(const QDomNode& svgNode) const {
    // TODO (jclaveau) : a way to look the svg after the parsing would be nice!
    QByteArray out;
    QTextStream textStream(&out);
    // cloning avoid segfault during save()
    svgNode.cloneNode().save(textStream, 2);
    return out;
}

QScriptValue SvgParser::evaluateTemplateExpression(const QString& expression,
                                                   int lineNumber) const {
    auto childContext = lazyChildContext();
    QScriptValue out = childContext.evaluateScript(
        expression, m_currentFile, lineNumber);
    if (childContext.getScriptEngine()->hasUncaughtException()) {
        // return an empty string as replacement for the in-attribute expression
        return QScriptValue();
    } else {
        return out;
    }
}
