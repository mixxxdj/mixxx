#include "skin/skincontext.h"

SkinContext::SkinContext() {
}

SkinContext::SkinContext(const SkinContext& parent)
        : m_variables(parent.variables()) {
}

SkinContext::~SkinContext() {
}

QString SkinContext::variable(const QString& name) const {
    return m_variables.value(name, QString());
}

void SkinContext::setVariable(const QString& name, const QString& value) {
    m_variables[name] = value;
}

bool SkinContext::hasNode(const QDomNode& node, const QString& nodeName) const {
    return !selectNode(node, nodeName).isNull();
}

QDomNode SkinContext::selectNode(const QDomNode& node,
                                 const QString& nodeName) const {
    QDomNode child = node.firstChild();
    while (!child.isNull()) {
        if (child.nodeName() == nodeName) {
            return child;
        }
        child = child.nextSibling();
    }
    return child;
}

QDomElement SkinContext::selectElement(const QDomNode& node,
                                    const QString& nodeName) const {
    QDomNode child = selectNode(node, nodeName);
    return child.toElement();
}

QString SkinContext::selectString(const QDomNode& node,
                                  const QString& nodeName) const {
    QDomNode child = selectNode(node, nodeName);
    return child.isNull() ? "" : child.toElement().text();
}

float SkinContext::selectFloat(const QDomNode& node,
                               const QString& nodeName) const {
    bool ok = false;
    float conv = selectElement(node, nodeName).text().toFloat(&ok);
    return ok ? conv : 0.0f;
}

double SkinContext::selectDouble(const QDomNode& node,
                               const QString& nodeName) const {
    bool ok = false;
    double conv = selectElement(node, nodeName).text().toDouble(&ok);
    return ok ? conv : 0.0;
}

int SkinContext::selectInt(const QDomNode& node,
                               const QString& nodeName) const {
    bool ok = false;
    int conv = selectElement(node, nodeName).text().toInt(&ok);
    return ok ? conv : 0;
}
