#pragma once

#include <QStyledItemDelegate>

class ControlPickerMenu;

class ControlDelegate : public QStyledItemDelegate {
    Q_OBJECT
  public:
    ControlDelegate(QObject* pParent, ControlPickerMenu* pControlPickerMenu);
    virtual ~ControlDelegate();

    inline void setMidiOptionsColumn(int column) {
        m_iMidiOptionsColumn = column;
    }

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const;

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const;

    QString displayText(const QVariant& value, const QLocale& locale) const;

    void setEditorData(QWidget* editor, const QModelIndex& index) const;

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const;

    QString translateConfigKeyGroup(const QString& group) const;

  private:
    ControlPickerMenu* m_pPicker;
    int m_iMidiOptionsColumn;
    // HACK(rryan): Does the last painted index have a script
    // MidiOption. displayText does not give us the current QModelIndex so we
    // can't check there.
    mutable bool m_bIsIndexScript;
    QMap<QString, QString> m_numGroupsTrMap;
    QMap<QString, QString> m_otherGroupsTrMap;
};
