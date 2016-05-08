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
                          const QString& sourcePath) const;
    QByteArray saveToQByteArray(const QDomNode& svgNode) const;

  private:
    const SkinContext& lazyChildContext() const {
        if (m_pLazyContext.isNull()) {
            m_pLazyContext.reset(new SkinContext(m_parentContext));
        }
        return *m_pLazyContext;
    }
    void scanTree(QDomElement* node) const;
    void parseElement(QDomElement* svgNode) const;
    void parseAttributes(QDomElement* element) const;
    QScriptValue evaluateTemplateExpression(const QString& expression,
                                            int lineNumber) const;

    const SkinContext& m_parentContext;
    mutable QScopedPointer<SkinContext> m_pLazyContext;
    mutable QString m_currentFile;
    mutable QRegExp m_hookRx;
};

#endif /* SVGPARSER_H */
