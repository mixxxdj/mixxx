#include "qml/qmltablefromlistmodelcolumn.h"

#include <QJSValue>
#include <QString>
#include <QtDebug>

namespace {

const QString displayRoleName = QStringLiteral("display");
const QString decorationRoleName = QStringLiteral("decoration");
const QString editRoleName = QStringLiteral("edit");
const QString toolTipRoleName = QStringLiteral("toolTip");
const QString statusTipRoleName = QStringLiteral("statusTip");
const QString whatsThisRoleName = QStringLiteral("whatsThis");
const QString fontRoleName = QStringLiteral("font");
const QString textAlignmentRoleName = QStringLiteral("textAlignment");
const QString backgroundRoleName = QStringLiteral("background");
const QString foregroundRoleName = QStringLiteral("foreground");
const QString checkStateRoleName = QStringLiteral("checkState");
const QString accessibleTextRoleName = QStringLiteral("accessibleText");
const QString accessibleDescriptionRoleName = QStringLiteral("accessibleDescription");
const QString sizeHintRoleName = QStringLiteral("sizeHint");

} // namespace

namespace mixxx {
namespace qml {
QmlTableFromListModelColumn::QmlTableFromListModelColumn(QObject* parent)
        : QObject(parent) {
}
QmlTableFromListModelColumn::~QmlTableFromListModelColumn() {
}
#define DEFINE_ROLE_PROPERTIES(getterGetterName,                              \
        getterSetterName,                                                     \
        getterSignal,                                                         \
        setterGetterName,                                                     \
        setterSetterName,                                                     \
        setterSignal,                                                         \
        roleName)                                                             \
    QJSValue QmlTableFromListModelColumn::getterGetterName() const {          \
        return m_getters.value(roleName);                                     \
    }                                                                         \
                                                                              \
    void QmlTableFromListModelColumn::getterSetterName(                       \
            const QJSValue& stringOrFunction) {                               \
        if (!stringOrFunction.isString() && !stringOrFunction.isCallable()) { \
            qWarning().quote()                                                \
                    << "getter for " << roleName << " must be a function";    \
            return;                                                           \
        }                                                                     \
                                                                              \
        if (stringOrFunction.strictlyEquals(getterGetterName())) {            \
            return;                                                           \
        }                                                                     \
                                                                              \
        m_getters[roleName] = stringOrFunction;                               \
        emit getterSignal();                                                  \
    }                                                                         \
                                                                              \
    QJSValue QmlTableFromListModelColumn::setterGetterName() const {          \
        return m_setters.value(roleName);                                     \
    }                                                                         \
                                                                              \
    void QmlTableFromListModelColumn::setterSetterName(                       \
            const QJSValue& function) {                                       \
        if (!function.isCallable()) {                                         \
            qWarning().quote()                                                \
                    << "setter for " << roleName << " must be a function";    \
            return;                                                           \
        }                                                                     \
                                                                              \
        if (function.strictlyEquals(setterGetterName())) {                    \
            return;                                                           \
        }                                                                     \
                                                                              \
        m_setters[roleName] = function;                                       \
        emit setterSignal();                                                  \
    }
DEFINE_ROLE_PROPERTIES(display,
        setDisplay,
        displayChanged,
        getSetDisplay,
        setSetDisplay,
        setDisplayChanged,
        displayRoleName)
DEFINE_ROLE_PROPERTIES(decoration,
        setDecoration,
        decorationChanged,
        getSetDecoration,
        setSetDecoration,
        setDecorationChanged,
        decorationRoleName)
DEFINE_ROLE_PROPERTIES(edit,
        setEdit,
        editChanged,
        getSetEdit,
        setSetEdit,
        setEditChanged,
        editRoleName)
DEFINE_ROLE_PROPERTIES(toolTip,
        setToolTip,
        toolTipChanged,
        getSetToolTip,
        setSetToolTip,
        setToolTipChanged,
        toolTipRoleName)
DEFINE_ROLE_PROPERTIES(statusTip,
        setStatusTip,
        statusTipChanged,
        getSetStatusTip,
        setSetStatusTip,
        setStatusTipChanged,
        statusTipRoleName)
DEFINE_ROLE_PROPERTIES(whatsThis,
        setWhatsThis,
        whatsThisChanged,
        getSetWhatsThis,
        setSetWhatsThis,
        setWhatsThisChanged,
        whatsThisRoleName)
DEFINE_ROLE_PROPERTIES(font,
        setFont,
        fontChanged,
        getSetFont,
        setSetFont,
        setFontChanged,
        fontRoleName)
DEFINE_ROLE_PROPERTIES(textAlignment,
        setTextAlignment,
        textAlignmentChanged,
        getSetTextAlignment,
        setSetTextAlignment,
        setTextAlignmentChanged,
        textAlignmentRoleName)
DEFINE_ROLE_PROPERTIES(background,
        setBackground,
        backgroundChanged,
        getSetBackground,
        setSetBackground,
        setBackgroundChanged,
        backgroundRoleName)
DEFINE_ROLE_PROPERTIES(foreground,
        setForeground,
        foregroundChanged,
        getSetForeground,
        setSetForeground,
        setForegroundChanged,
        foregroundRoleName)
DEFINE_ROLE_PROPERTIES(checkState,
        setCheckState,
        checkStateChanged,
        getSetCheckState,
        setSetCheckState,
        setCheckStateChanged,
        checkStateRoleName)
DEFINE_ROLE_PROPERTIES(accessibleText,
        setAccessibleText,
        accessibleTextChanged,
        getSetAccessibleText,
        setSetAccessibleText,
        setAccessibleTextChanged,
        accessibleTextRoleName)
DEFINE_ROLE_PROPERTIES(accessibleDescription,
        setAccessibleDescription,
        accessibleDescriptionChanged,
        getSetAccessibleDescription,
        setSetAccessibleDescription,
        setAccessibleDescriptionChanged,
        accessibleDescriptionRoleName)
DEFINE_ROLE_PROPERTIES(sizeHint,
        setSizeHint,
        sizeHintChanged,
        getSetSizeHint,
        setSetSizeHint,
        setSizeHintChanged,
        sizeHintRoleName)
QJSValue QmlTableFromListModelColumn::getterAtRole(const QString& roleName) {
    auto it = m_getters.find(roleName);
    if (it == m_getters.end())
        return QJSValue();
    return *it;
}
QJSValue QmlTableFromListModelColumn::setterAtRole(const QString& roleName) {
    auto it = m_setters.find(roleName);
    if (it == m_setters.end())
        return QJSValue();
    return *it;
}
const QHash<QString, QJSValue> QmlTableFromListModelColumn::getters() const {
    return m_getters;
}
const QHash<int, QString> QmlTableFromListModelColumn::supportedRoleNames() {
    QHash<int, QString> names;
    names[Qt::DisplayRole] = QLatin1String("display");
    names[Qt::DecorationRole] = QLatin1String("decoration");
    names[Qt::EditRole] = QLatin1String("edit");
    names[Qt::ToolTipRole] = QLatin1String("toolTip");
    names[Qt::StatusTipRole] = QLatin1String("statusTip");
    names[Qt::WhatsThisRole] = QLatin1String("whatsThis");
    names[Qt::FontRole] = QLatin1String("font");
    names[Qt::TextAlignmentRole] = QLatin1String("textAlignment");
    names[Qt::BackgroundRole] = QLatin1String("background");
    names[Qt::ForegroundRole] = QLatin1String("foreground");
    names[Qt::CheckStateRole] = QLatin1String("checkState");
    names[Qt::AccessibleTextRole] = QLatin1String("accessibleText");
    names[Qt::AccessibleDescriptionRole] = QLatin1String("accessibleDescription");
    names[Qt::SizeHintRole] = QLatin1String("sizeHint");
    return names;
}
} // namespace qml
} // namespace mixxx
