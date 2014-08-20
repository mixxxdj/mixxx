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

    p_context = &parent;

}

SvgParser::~SvgParser() {
}

// look for the document of a node
QDomDocument SvgParser::getDocument(const QDomNode& node) const {
    
    QDomDocument document;
    QDomNode parentNode = node;
    while( !parentNode.isNull() ){
        if( parentNode.isDocument() )
            document = parentNode.toDocument();
        parentNode = parentNode.parentNode();
    }
    
    return document;
}

QString SvgParser::parseSvgFile(const QString& svgFileName) const {
    QFile* file = new QFile(svgFileName);
    if(file->open(QIODevice::ReadWrite | QIODevice::Text)){
        QDomDocument document;
        document.setContent(file);
        QDomNode svgNode = document.elementsByTagName("svg").item(0);
        QString pixmapPath = parseSvgTree(svgNode);
        file->close();
        return pixmapPath;
    } else {
        return svgFileName;
    }
}

QString SvgParser::parseSvgTree(const QDomNode& svgSkinNode) const {
    
    // clone svg to don't alter xml input
    QDomNode svgNode = svgSkinNode.cloneNode(true);
    
    setVariables(svgNode);
    
    parseScriptElements(svgNode);
    
    parseTree(svgNode, &SvgParser::setVariablesInAttributes);
    
    // Save the new svg in a temp file to use it with setPixmap
    QTemporaryFile svgFile;
    svgFile.setFileTemplate(QDir::temp().filePath("qt_temp.XXXXXX.svg"));
    
    // the file will be removed before being parsed in skin if set to true
    svgFile.setAutoRemove( false );
    
    QString svgTempFileName;
    if( svgFile.open() ){
        // qWarning() << "SVG : Temp filename" << svgFile.fileName() << " \n";
        QTextStream out(&svgFile);
        svgNode.save( out, 2 );
        svgFile.close();
        svgTempFileName = svgFile.fileName();
    } else {
        qDebug() << "Unable to open temp file for inline svg \n";
    }
    
    return svgTempFileName;
}

// replaces Variables nodes in an svg dom tree
void SvgParser::setVariables(const QDomNode& svgNode) const {
    
    QDomDocument document = getDocument(svgNode);
    QDomElement svgElement = svgNode.toElement();
    
    // replace variables
    QDomNodeList variablesElements = svgElement.elementsByTagName("Variable");
    int i=variablesElements.length()-1;
    QDomElement varElement;
    QString varName, varValue;
    QDomNode varNode, varParentNode, oldChild;
    QDomText varValueNode;
    
    while ( i >= 0 && (varNode = variablesElements.item(i)).isNull() == false ){
        
        // retrieve value
        varElement = varNode.toElement();
        varName = varElement.attribute("name");
        varValue = p_context->variable(varName);
        
        // replace node by its value
        varParentNode = varNode.parentNode();
        varValueNode = document.createTextNode(varValue);
        oldChild = varParentNode.replaceChild( varValueNode, varNode );
        
        if( oldChild.isNull() ){
            // replaceChild has a really weird behaviour so I add this check
            qDebug() << "SVG : unable to replace dom node changed. \n";
        }
        
        --i;
    }
}


void SvgParser::setVariablesInAttributes(const QDomNode& node) const {
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
    
    hookNames = global.property("hookNames").call( global );
    
    if( hookNames.toString().length() ){
        hooksPattern = hookNames.property("toPattern").call(hookNames).toString();
    } else {
        hooksPattern = "variable";
    }
    
    // qDebug() <<  "hooksPattern : " << hooksPattern << "\n";
    
    QRegExp rx("("+hooksPattern+")\\(([^\\(\\)]+)\\)\\s*;?");                   // hook( arg1 [, arg2]... )
    QRegExp nameRx("^expr-([^=\\s]+)$");                                        // expr-attribute_name="var_name";
    
    for (i=0; i < attributes.length(); i++){
        
        attribute = attributes.item(i).toAttr();
        attributeValue = attribute.value();
        attributeName = attribute.name();
        
        // searching variable attributes : var-attribute_name="variable_name"
        if (nameRx.indexIn(attributeName) != -1) {
            
            varValue = evaluateTemplateExpression(attributeValue).toString();
            
            if (varValue.length()){
                element.setAttribute( nameRx.cap(1), varValue);
            }
            
            continue;
        }
         
        pos = 0;
        // searching hooks in the attribute value
        while ((pos = rx.indexIn(attributeValue, pos)) != -1) {
            captured = rx.capturedTexts();
            match = rx.cap(0);
            QString tmp = "templateHooks." + match;
            // qDebug() <<  "expression : " << tmp << "\n";
            replacement = evaluateTemplateExpression( tmp ).toString();
            attributeValue.replace(pos, match.length(), replacement);
            pos += replacement.length();
        }
        
        attribute.setValue(attributeValue);
    }
    
}

void SvgParser::parseTree(const QDomNode& node, void (SvgParser::*callback)(const QDomNode& node)const) const {
    
    (this->*callback)( node );
    QDomNodeList children = node.childNodes();
    QDomNode child;
    uint i;
    
    for (i=0; i<children.length(); i++){
        child = children.at(i);
        if( child.isElement() )
            parseTree( child, callback );
    }
}


void SvgParser::parseScriptElements(const QDomNode& svgSkinNode) const {
    
    // parse script content
    QDomElement svgElement = svgSkinNode.toElement();
    QDomNodeList scriptElements = svgElement.elementsByTagName("script");
    int i = 0;
    QString expression;
    QDomNode scriptNode;
    
    while ( !(scriptNode = scriptElements.item(i)).isNull() && ++i ){
        if( scriptNode.toElement().hasAttribute("src") ){
            QFile scriptFile( p_context->getSkinPath( scriptNode.toElement().attribute("src") ) );
            scriptFile.open(QIODevice::ReadOnly | QIODevice::Text);
            QTextStream in(&scriptFile);
            m_scriptEngine.evaluate( in.readAll() );
        }
        
        expression = p_context->nodeToString(scriptNode);
        m_scriptEngine.evaluate(expression);
    }
    
}

QScriptValue SvgParser::evaluateTemplateExpression(QString expression) const {
    QScriptValue out = m_scriptEngine.evaluate(expression);
    if(m_scriptEngine.hasUncaughtException()){
        qDebug()
            << "SVG script exception : " << out.toString()
            << "Empty string returned";
        
        // return an empty string as remplacement for the in-attribute expression
        return m_scriptEngine.nullValue();
    } else {
        return out;
    }
}
