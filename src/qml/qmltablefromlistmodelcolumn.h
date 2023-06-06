#pragma once

#include <QAbstractListModel>
#include <QAbstractTableModel>
#include <QJSValue>
#include <QQmlListProperty>
#include <QQmlParserStatus>
#include <QtQml>

namespace mixxx {
namespace qml {

class QmlTableFromListModelColumn : public QObject {
    Q_OBJECT
    Q_PROPERTY(QJSValue display READ display WRITE setDisplay NOTIFY displayChanged FINAL)
    Q_PROPERTY(QJSValue setDisplay READ getSetDisplay WRITE setSetDisplay NOTIFY setDisplayChanged)
    Q_PROPERTY(QJSValue decoration READ decoration WRITE setDecoration NOTIFY
                    decorationChanged FINAL)
    Q_PROPERTY(QJSValue setDecoration READ getSetDecoration WRITE
                    setSetDecoration NOTIFY setDecorationChanged FINAL)
    Q_PROPERTY(QJSValue edit READ edit WRITE setEdit NOTIFY editChanged FINAL)
    Q_PROPERTY(QJSValue setEdit READ getSetEdit WRITE setSetEdit NOTIFY setEditChanged FINAL)
    Q_PROPERTY(QJSValue toolTip READ toolTip WRITE setToolTip NOTIFY toolTipChanged FINAL)
    Q_PROPERTY(QJSValue setToolTip READ getSetToolTip WRITE setSetToolTip NOTIFY
                    setToolTipChanged FINAL)
    Q_PROPERTY(QJSValue statusTip READ statusTip WRITE setStatusTip NOTIFY statusTipChanged FINAL)
    Q_PROPERTY(QJSValue setStatusTip READ getSetStatusTip WRITE setSetStatusTip
                    NOTIFY setStatusTipChanged FINAL)
    Q_PROPERTY(QJSValue whatsThis READ whatsThis WRITE setWhatsThis NOTIFY whatsThisChanged FINAL)
    Q_PROPERTY(QJSValue setWhatsThis READ getSetWhatsThis WRITE setSetWhatsThis
                    NOTIFY setWhatsThisChanged FINAL)
    Q_PROPERTY(QJSValue font READ font WRITE setFont NOTIFY fontChanged FINAL)
    Q_PROPERTY(QJSValue setFont READ getSetFont WRITE setSetFont NOTIFY setFontChanged FINAL)
    Q_PROPERTY(QJSValue textAlignment READ textAlignment WRITE setTextAlignment
                    NOTIFY textAlignmentChanged FINAL)
    Q_PROPERTY(QJSValue setTextAlignment READ getSetTextAlignment WRITE
                    setSetTextAlignment NOTIFY setTextAlignmentChanged FINAL)
    Q_PROPERTY(QJSValue background READ background WRITE setBackground NOTIFY
                    backgroundChanged FINAL)
    Q_PROPERTY(QJSValue setBackground READ getSetBackground WRITE
                    setSetBackground NOTIFY setBackgroundChanged FINAL)
    Q_PROPERTY(QJSValue foreground READ foreground WRITE setForeground NOTIFY
                    foregroundChanged FINAL)
    Q_PROPERTY(QJSValue setForeground READ getSetForeground WRITE
                    setSetForeground NOTIFY setForegroundChanged FINAL)
    Q_PROPERTY(QJSValue checkState READ checkState WRITE setCheckState NOTIFY
                    checkStateChanged FINAL)
    Q_PROPERTY(QJSValue setCheckState READ getSetCheckState WRITE
                    setSetCheckState NOTIFY setCheckStateChanged FINAL)
    Q_PROPERTY(QJSValue accessibleText READ accessibleText WRITE
                    setAccessibleText NOTIFY accessibleTextChanged FINAL)
    Q_PROPERTY(QJSValue setAccessibleText READ getSetAccessibleText WRITE
                    setSetAccessibleText NOTIFY setAccessibleTextChanged FINAL)
    Q_PROPERTY(QJSValue accessibleDescription READ accessibleDescription
                    WRITE setAccessibleDescription NOTIFY accessibleDescriptionChanged FINAL)
    Q_PROPERTY(QJSValue setAccessibleDescription READ getSetAccessibleDescription
                    WRITE setSetAccessibleDescription NOTIFY setAccessibleDescriptionChanged FINAL)
    Q_PROPERTY(QJSValue sizeHint READ sizeHint WRITE setSizeHint NOTIFY sizeHintChanged FINAL)
    Q_PROPERTY(QJSValue setSizeHint READ getSetSizeHint WRITE setSetSizeHint
                    NOTIFY setSizeHintChanged FINAL)
    QML_NAMED_ELEMENT(TableFromListModelColumn)
  public:
    QmlTableFromListModelColumn(QObject* parent = nullptr);
    ~QmlTableFromListModelColumn() override;
    QJSValue display() const;
    void setDisplay(const QJSValue& stringOrFunction);
    QJSValue getSetDisplay() const;
    void setSetDisplay(const QJSValue& function);
    QJSValue decoration() const;
    void setDecoration(const QJSValue& stringOrFunction);
    QJSValue getSetDecoration() const;
    void setSetDecoration(const QJSValue& function);
    QJSValue edit() const;
    void setEdit(const QJSValue& stringOrFunction);
    QJSValue getSetEdit() const;
    void setSetEdit(const QJSValue& function);
    QJSValue toolTip() const;
    void setToolTip(const QJSValue& stringOrFunction);
    QJSValue getSetToolTip() const;
    void setSetToolTip(const QJSValue& function);
    QJSValue statusTip() const;
    void setStatusTip(const QJSValue& stringOrFunction);
    QJSValue getSetStatusTip() const;
    void setSetStatusTip(const QJSValue& function);
    QJSValue whatsThis() const;
    void setWhatsThis(const QJSValue& stringOrFunction);
    QJSValue getSetWhatsThis() const;
    void setSetWhatsThis(const QJSValue& function);
    QJSValue font() const;
    void setFont(const QJSValue& stringOrFunction);
    QJSValue getSetFont() const;
    void setSetFont(const QJSValue& function);
    QJSValue textAlignment() const;
    void setTextAlignment(const QJSValue& stringOrFunction);
    QJSValue getSetTextAlignment() const;
    void setSetTextAlignment(const QJSValue& function);
    QJSValue background() const;
    void setBackground(const QJSValue& stringOrFunction);
    QJSValue getSetBackground() const;
    void setSetBackground(const QJSValue& function);
    QJSValue foreground() const;
    void setForeground(const QJSValue& stringOrFunction);
    QJSValue getSetForeground() const;
    void setSetForeground(const QJSValue& function);
    QJSValue checkState() const;
    void setCheckState(const QJSValue& stringOrFunction);
    QJSValue getSetCheckState() const;
    void setSetCheckState(const QJSValue& function);
    QJSValue accessibleText() const;
    void setAccessibleText(const QJSValue& stringOrFunction);
    QJSValue getSetAccessibleText() const;
    void setSetAccessibleText(const QJSValue& function);
    QJSValue accessibleDescription() const;
    void setAccessibleDescription(const QJSValue& stringOrFunction);
    QJSValue getSetAccessibleDescription() const;
    void setSetAccessibleDescription(const QJSValue& function);
    QJSValue sizeHint() const;
    void setSizeHint(const QJSValue& stringOrFunction);
    QJSValue getSetSizeHint() const;
    void setSetSizeHint(const QJSValue& function);
    QJSValue getterAtRole(const QString& roleName);
    QJSValue setterAtRole(const QString& roleName);
    const QHash<QString, QJSValue> getters() const;
    static const QHash<int, QString> supportedRoleNames();
  Q_SIGNALS:
    void indexChanged();
    void displayChanged();
    void setDisplayChanged();
    void decorationChanged();
    void setDecorationChanged();
    void editChanged();
    void setEditChanged();
    void toolTipChanged();
    void setToolTipChanged();
    void statusTipChanged();
    void setStatusTipChanged();
    void whatsThisChanged();
    void setWhatsThisChanged();
    void fontChanged();
    void setFontChanged();
    void textAlignmentChanged();
    void setTextAlignmentChanged();
    void backgroundChanged();
    void setBackgroundChanged();
    void foregroundChanged();
    void setForegroundChanged();
    void checkStateChanged();
    void setCheckStateChanged();
    void accessibleTextChanged();
    void setAccessibleTextChanged();
    void accessibleDescriptionChanged();
    void setAccessibleDescriptionChanged();
    void sizeHintChanged();
    void setSizeHintChanged();

  private:
    // We store these in hashes because QQuickTableModel needs string-based lookup
    // in certain situations.
    QHash<QString, QJSValue> m_getters;
    QHash<QString, QJSValue> m_setters;
};

} // namespace qml
} // namespace mixxx
