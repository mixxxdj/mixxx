#ifndef SVGPARSER_H
#define SVGPARSER_H

#include <QHash>
#include <QString>
#include <QDomNode>
#include <QDomElement>

#include "skin/skincontext.h"


// A class for managing the svg files
class SvgParser {
  public:
    SvgParser();
    SvgParser(const SkinContext& parent);
    virtual ~SvgParser();

    QDomNode parseSvgTree(const QDomNode& svgSkinNode) const;
    QDomNode parseSvgFile(const QString& svgFileName) const;
    QString saveToTempFile(const QDomNode& svgNode) const;
    QByteArray saveToQByteArray(const QDomNode& svgNode) const;

  private:
    void scanTree(QDomNode& node) const;
    void parseElement(QDomNode& svgNode) const;
    void parseAttributes(QDomNode& node) const;
    QScriptValue evaluateTemplateExpression(QString expression,
                                            int lineNumber) const;
    
    mutable SkinContext m_context;
    QDomDocument m_document;
    mutable QString m_currentFile;
    
};

#endif /* SVGPARSER_H */
