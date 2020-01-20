#pragma once

#include <QString>
#include <QDomNode>
#include <QDomElement>

#include "skin/skincontext.h"


// A class for managing the svg files
class SvgParser {
  public:
    // Assumes SkinContext lives for the lifetime of SvgParser.
    explicit SvgParser(const SkinContext* pParentContext);
    virtual ~SvgParser() = default;

    QDomNode parseSvgTree(const QDomNode& svgSkinNode,
                          const QString& sourcePath);
    QByteArray saveToQByteArray(const QDomNode& svgNode) const;

  private:
    void scanTree(QDomElement* node) const;
    void parseElement(QDomElement* svgNode) const;
    void parseAttributes(QDomElement* element) const;
    QScriptValue evaluateTemplateExpression(
            const QString& expression, int lineNumber) const;

    const SkinContext* m_pParentContext;
    SkinContext m_childContext;
    QString m_currentFile;
};
