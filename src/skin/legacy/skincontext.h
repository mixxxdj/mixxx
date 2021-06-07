#pragma once

#include <memory>

#include <QHash>
#include <QString>
#include <QDomNode>
#include <QDomElement>
#include <QDir>
#include <QtDebug>
#include <QRegExp>

#include "preferences/usersettings.h"
#include "skin/legacy/pixmapsource.h"
#include "util/color/color.h"
#include "widget/wsingletoncontainer.h"
#include "widget/wpixmapstore.h"

#define SKIN_WARNING(node, context) (context).logWarning(__FILE__, __LINE__, (node))

// A class for managing the current context/environment when processing a
// skin. Used hierarchically by LegacySkinParser to create new contexts and
// evaluate skin XML nodes while loading the skin.
class SkinContext {
  public:
    SkinContext(
            UserSettingsPointer pConfig,
            const QString& xmlPath);
    SkinContext(
            const SkinContext* parent);
    virtual ~SkinContext() = default;

    // Not copiable
    SkinContext(const SkinContext&) = delete;
    SkinContext& operator=(const SkinContext&) = delete;

    // Moveable
    SkinContext(SkinContext&&) = default;
    SkinContext& operator=(SkinContext&&) = default;

    // Gets a path relative to the skin path.
    QString makeSkinPath(const QString& relativePath) const {
        if (relativePath.isEmpty() || relativePath.startsWith("/")
                || relativePath.contains(":")) {
            // This is already an absolute path start with the root folder "/"
            // a windows drive letter e.g. "C:" or a qt search path prefix
            return relativePath;
        }
        return QString("skin:").append(relativePath);
    }

    // Sets the base path used by getSkinPath.
    void setSkinBasePath(const QString& skinBasePath) {
        QStringList skinPaths(skinBasePath);
        QDir::setSearchPaths("skin", skinPaths);
        m_skinBasePath = skinBasePath;
    }

    // Sets the base path used by getSkinPath.
    void setSkinTemplatePath(const QString& skinTemplatePath) {
        QStringList skinPaths(m_skinBasePath);
        skinPaths.append(skinTemplatePath);
        QDir::setSearchPaths("skin", skinPaths);
    }

    // Variable lookup and modification methods.
    QString variable(const QString& name) const;
    const QHash<QString, QString>& variables() const {
        return m_variables;
    }
    void setVariable(const QString& name, const QString& value);
    void setXmlPath(const QString& xmlPath);

    // Returns whether the node has a <SetVariable> node.
    bool hasVariableUpdates(const QDomNode& node) const;
    // Updates the SkinContext with all the <SetVariable> children of node.
    void updateVariables(const QDomNode& node);
    // Updates the SkinContext with 'element', a <SetVariable> node.
    void updateVariable(const QDomElement& element);

    static inline QDomNode selectNode(const QDomNode& node, const QString& nodeName) {
        QDomNode child = node.firstChild();
        while (!child.isNull()) {
            if (child.nodeName() == nodeName) {
                return child;
            }
            child = child.nextSibling();
        }
        return QDomNode();
    }

    static inline QDomElement selectElement(const QDomNode& node, const QString& nodeName) {
        QDomNode child = selectNode(node, nodeName);
        return child.toElement();
    }

    inline QString selectString(const QDomNode& node, const QString& nodeName) const {
        QDomElement child = selectElement(node, nodeName);
        return nodeToString(child);
    }

    inline float selectFloat(const QDomNode& node, const QString& nodeName, float defaultValue = 0.0) const {
        bool ok = false;
        float conv = nodeToString(selectElement(node, nodeName)).toFloat(&ok);
        return ok ? conv : defaultValue;
    }

    inline double selectDouble(const QDomNode& node, const QString& nodeName, double defaultValue = 0.0) const {
        bool ok = false;
        double conv = nodeToString(selectElement(node, nodeName)).toDouble(&ok);
        return ok ? conv : defaultValue;
    }

    inline int selectInt(const QDomNode& node, const QString& nodeName,
                         bool* pOk = nullptr) const {
            bool ok = false;
            int conv = nodeToString(selectElement(node, nodeName)).toInt(&ok);
            if (pOk != nullptr) {
                *pOk = ok;
            }
            return ok ? conv : 0;
    }

    inline QColor selectColor(const QDomNode& node, const QString& nodeName) const {
        QString sColorString = nodeToString(selectElement(node, nodeName));
        return QColor(sColorString);
    }

    inline bool selectBool(const QDomNode& node, const QString& nodeName,
                           bool defaultValue) const {
        QDomNode child = selectNode(node, nodeName);
        if (!child.isNull()) {
            QString stringValue = nodeToString(child);
            return stringValue.contains("true", Qt::CaseInsensitive);
        }
        return defaultValue;
    }

    inline bool hasNodeSelectElement(const QDomNode& node, const QString& nodeName,
                                     QDomElement* value) const {
        QDomElement child = selectElement(node, nodeName);
        if (!child.isNull()) {
            *value = child;
            return true;
        }
        return false;
    }

    inline bool hasNodeSelectString(const QDomNode& node, const QString& nodeName,
                                    QString *value) const {
        QDomNode child = selectNode(node, nodeName);
        if (!child.isNull()) {
            *value = nodeToString(child);
            return true;
        }
        return false;
    }

    inline bool hasNodeSelectBool(const QDomNode& node, const QString& nodeName,
                                  bool* value) const {
        QDomNode child = selectNode(node, nodeName);
        if (!child.isNull()) {
            QString stringValue = nodeToString(child);
            *value = stringValue.contains("true", Qt::CaseInsensitive);
            return true;
        }
        return false;
    }

    inline bool hasNodeSelectInt(const QDomNode& node, const QString& nodeName,
                                 int* value) const {
        QDomNode child = selectNode(node, nodeName);
        if (!child.isNull()) {
            bool ok = false;
            int result = nodeToString(child).toInt(&ok);
            if (ok) {
                *value = result;
                return true;
            }
        }
        return false;
    }

    inline bool hasNodeSelectDouble(const QDomNode& node, const QString& nodeName,
                                    double* value) const {
        QDomNode child = selectNode(node, nodeName);
        if (!child.isNull()) {
            bool ok = false;
            double result = nodeToString(child).toDouble(&ok);
            if (ok) {
                *value = result;
                return true;
            }
        }
        return false;
    }

    inline bool selectAttributeBool(const QDomElement& element,
                                    const QString& attributeName,
                                    bool defaultValue) const {
        QString stringValue;
        if (hasAttributeSelectString(element, attributeName, &stringValue)) {
            return stringValue.contains("true", Qt::CaseInsensitive);
        }
        return defaultValue;
    }

    inline bool hasAttributeSelectString(const QDomElement& element,
                                         const QString& attributeName,
                                         QString* result) const {
        *result = element.attribute(attributeName);
        return !result->isNull();
    }

    QString nodeToString(const QDomNode& node) const;
    PixmapSource getPixmapSource(const QDomNode& pixmapNode) const;
    PixmapSource getPixmapSource(const QString& filename) const;

    inline Paintable::DrawMode selectScaleMode(
            const QDomElement& element,
            Paintable::DrawMode defaultDrawMode) const {
        QString drawModeStr;
        if (hasAttributeSelectString(element, "scalemode", &drawModeStr)) {
            return Paintable::DrawModeFromString(drawModeStr);
        }
        return defaultDrawMode;
    }

    QDebug logWarning(const char* file, const int line, const QDomNode& node) const;

    void defineSingleton(const QString& objectName, QWidget* widget) {
        m_pSharedState->singletons.insertSingleton(objectName, widget);
    }

    QWidget* getSingletonWidget(const QString& objectName) const {
        return m_pSharedState->singletons.getSingletonWidget(objectName);
    }

    const QRegExp& getHookRegex() const {
        return m_hookRx;
    }

    int scaleToWidgetSize(QString& size) const;

    double getScaleFactor() const {
        return m_scaleFactor;
    }

    UserSettingsPointer getConfig() const {
        return m_pConfig;
    }

    QString getSkinBasePath() const {
        return m_skinBasePath;
    }

  private:
    PixmapSource getPixmapSourceInner(const QString& filename) const;

    QDomElement loadSvg(const QString& filename) const;

    QString variableNodeToText(const QDomElement& element) const;

    UserSettingsPointer m_pConfig;

    QString m_xmlPath;
    QString m_skinBasePath;

    struct SharedState final {
        SharedState() = default;
        SharedState(const SharedState&) = delete;
        SharedState(SharedState&&) = delete;

        QHash<QString, QDomElement> svgCache;
        // The SingletonContainer map is passed to child SkinContexts, so that all
        // templates in the tree can share a single map.
        SingletonMap singletons;
    };
    // Use std::shared_ptr instead of QSharedPointer to guarantee
    // correct move semantics!
    std::shared_ptr<SharedState> m_pSharedState;

    QHash<QString, QString> m_variables;
    QRegExp m_hookRx;

    double m_scaleFactor;
};
