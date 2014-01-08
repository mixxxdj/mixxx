#ifndef SKINCONTEXT_H
#define SKINCONTEXT_H

#include <QHash>
#include <QString>
#include <QDomNode>
#include <QDomElement>
#include <QScriptEngine>

// A class for managing the current context/environment when processing a
// skin. Used hierarchically by LegacySkinParser to create new contexts and
// evaluate skin XML nodes while loading the skin.
class SkinContext {
  public:
    SkinContext();
    SkinContext(const SkinContext& parent);
    virtual ~SkinContext();

    SkinContext& operator=(const SkinContext& other);

    // Gets a path relative to the skin path.
    QString getSkinPath(const QString& relativePath) const {
        QString l(relativePath);
        return l.prepend(m_skinBasePath);
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
    int selectInt(const QDomNode& node, const QString& nodeName) const;
    bool selectBool(const QDomNode& node, const QString& nodeName) const;
    QString nodeToString(const QDomNode& node) const;

  private:
    QString variableNodeToText(const QDomElement& element) const;

    mutable QScriptEngine m_scriptEngine;
    QHash<QString, QString> m_variables;
    QString m_skinBasePath;
};

#endif /* SKINCONTEXT_H */
