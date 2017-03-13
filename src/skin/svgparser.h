#ifndef SVGPARSER_H
#define SVGPARSER_H

#include <QString>
#include <QDomNode>
#include <QDomElement>

#include "skin/skincontext.h"


// A class for managing the svg files
class SvgParser {
  public:
    SvgParser(const SkinContext& parent);
    virtual ~SvgParser();

    QDomNode parseSvgTree(const QDomNode& svgSkinNode,
                          const QString& sourcePath) const;
    QDomNode parseSvgFile(const QString& svgFileName) const;
    QByteArray saveToQByteArray(const QDomNode& svgNode) const;

  private:
    void scanTree(QDomNode* node) const;
    void parseElement(QDomNode* svgNode) const;
    void parseAttributes(const QDomNode& node) const;
    QScriptValue evaluateTemplateExpression(const QString& expression,
                                            int lineNumber) const;

    mutable SkinContext m_context;
    mutable QString m_currentFile;
    QDomDocument m_document;
};

#endif /* SVGPARSER_H */
