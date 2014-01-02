#ifndef SKINCONTEXT_H
#define SKINCONTEXT_H

#include <QHash>
#include <QString>
#include <QDomNode>
#include <QDomElement>
#include <QScriptEngine>

class SkinContext {
  public:
    SkinContext();
    SkinContext(const SkinContext& parent);
    virtual ~SkinContext();

    SkinContext& operator=(const SkinContext& other);

    QString variable(const QString& name) const;
    const QHash<QString, QString>& variables() const {
        return m_variables;
    }
    void setVariable(const QString& name, const QString& value);
    void updateVariables(const QDomNode& node);
    void updateVariable(const QDomElement& element);

    bool hasNode(const QDomNode& node, const QString& nodeName) const;
    QDomNode selectNode(const QDomNode& node, const QString& nodeName) const;
    QDomElement selectElement(const QDomNode& node, const QString& nodeName) const;
    QString selectString(const QDomNode& node, const QString& nodeName) const;
    float selectFloat(const QDomNode& node, const QString& nodeName) const;
    double selectDouble(const QDomNode& node, const QString& nodeName) const;
    int selectInt(const QDomNode& node, const QString& nodeName) const;
    QString nodeToString(const QDomNode& node) const;

  private:
    QString variableNodeToText(const QDomElement& element) const;

    mutable QScriptEngine m_scriptEngine;
    QHash<QString, QString> m_variables;
};

#endif /* SKINCONTEXT_H */
