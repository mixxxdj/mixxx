#ifndef SKINCONTEXT_H
#define SKINCONTEXT_H

#include <QHash>
#include <QString>
#include <QDomNode>
#include <QDomElement>
#include <QScriptEngine>
#include <QDir>
#include <QScriptEngineDebugger>
#include <QtDebug>

#include "configobject.h"
#include "skin/pixmapsource.h"
#include "widget/wpixmapstore.h"

#define SKIN_WARNING(node, context) (context).logWarning(__FILE__, __LINE__, (node))

// A class for managing the current context/environment when processing a
// skin. Used hierarchically by LegacySkinParser to create new contexts and
// evaluate skin XML nodes while loading the skin.
class SkinContext {
  public:
    SkinContext(ConfigObject<ConfigValue>* pConfig, const QString& xmlPath);
    SkinContext(const SkinContext& parent);
    virtual ~SkinContext();

    // Gets a path relative to the skin path.
    QString getSkinPath(const QString& relativePath) const {
        return QDir(m_skinBasePath).filePath(relativePath);
    }

    // Sets the base path used by getSkinPath.
    void setSkinBasePath(const QString& skinBasePath) {
        m_skinBasePath = skinBasePath;
    }

    // Variable lookup and modification methods.
    QString variable(const QString& name) const;
    const QHash<QString, QString>& variables() const {
        return m_variables;
    }
    void setVariable(const QString& name, const QString& value);
    void setXmlPath(const QString& xmlPath);

    // Updates the SkinContext with all the <SetVariable> children of node.
    void updateVariables(const QDomNode& node);
    // Updates the SkinContext with 'element', a <SetVariable> node.
    void updateVariable(const QDomElement& element);

    // Methods for evaluating nodes given the context.
    bool hasNode(const QDomNode& node, const QString& nodeName) const;
    QDomNode selectNode(const QDomNode& node, const QString& nodeName) const;
    QDomElement selectElement(const QDomNode& node, const QString& nodeName) const;
    QString selectString(const QDomNode& node, const QString& nodeName) const;
    float selectFloat(const QDomNode& node, const QString& nodeName) const;
    double selectDouble(const QDomNode& node, const QString& nodeName) const;
    int selectInt(const QDomNode& node, const QString& nodeName, bool* pOk=NULL) const;
    bool selectBool(const QDomNode& node, const QString& nodeName, bool defaultValue) const;
    bool hasNodeSelectString(const QDomNode& node, const QString& nodeName, QString *value) const;
    bool hasNodeSelectBool(const QDomNode& node, const QString& nodeName, bool *value) const;
    bool selectAttributeBool(const QDomElement& element,
                             const QString& attributeName,
                             bool defaultValue) const;
    QString selectAttributeString(const QDomElement& element,
                                  const QString& attributeName,
                                  QString defaultValue) const;
    QString nodeToString(const QDomNode& node) const;
    PixmapSource getPixmapSource(const QDomNode& pixmapNode) const;
    Paintable::DrawMode selectScaleMode(const QDomElement& element,
                                        Paintable::DrawMode defaultDrawMode) const;

    QScriptValue evaluateScript(const QString& expression,
                                const QString& filename=QString(),
                                int lineNumber=1);
    QScriptValue importScriptExtension(const QString& extensionName);
    const QSharedPointer<QScriptEngine> getScriptEngine() const;
    void enableDebugger(bool state) const;

    QDebug logWarning(const char* file, const int line, const QDomNode& node) const;

  private:
    QString variableNodeToText(const QDomElement& element) const;

    QString m_xmlPath;
    QString m_skinBasePath;
    ConfigObject<ConfigValue>* m_pConfig;

    QHash<QString, QString> m_variables;
    QSharedPointer<QScriptEngine> m_pScriptEngine;
    QSharedPointer<QScriptEngineDebugger> m_pScriptDebugger;
    QScriptValue m_parentGlobal;
};

#endif /* SKINCONTEXT_H */
