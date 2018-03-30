#ifndef SVGPARSER_H
#define SVGPARSER_H

#include <QString>
#include <QDomNode>
#include <QDomElement>
#include <QScopedPointer>

#include "skin/skincontext.h"


// A class for managing the svg files
class SvgParser {
  public:
    // Assumes SkinContext lives for the lifetime of SvgParser.
    SvgParser(const SkinContext& parent);
    virtual ~SvgParser();

    QDomNode parseSvgTree(const QDomNode& svgSkinNode,
                          const QString& sourcePath);
    QByteArray saveToQByteArray(const QDomNode& svgNode) const;

  private:
    void scanTree(QDomElement* node) const;
    void parseElement(QDomElement* svgNode) const;
    void parseAttributes(QDomElement* element) const;
    QScriptValue evaluateTemplateExpression(
            const QString& expression, int lineNumber) const;

    const SkinContext& m_parentContext;
    QScopedPointer<SkinContext> m_pChildContext;
    QString m_currentFile;
};

#endif /* SVGPARSER_H */
