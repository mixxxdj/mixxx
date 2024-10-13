#pragma once

#include <QAction>

class ActionUtils {
  public:
    static QString buildMultiShortcutActionText(
            const QString& baseText, const QList<QKeySequence>& shortcuts) {
        QString textAndAccel = baseText;
        bool first = true;
        for (const auto& shortcut : shortcuts) {
            if (!shortcut.isEmpty()) {
                if (first) {
                    textAndAccel += u'\t';
                    first = false;
                } else {
                    textAndAccel += QStringLiteral(" | ");
                }
                textAndAccel += shortcut.toString(QKeySequence::NativeText);
            }
        }
        return textAndAccel;
    }

    static void updateMultiShortcutActionText(QAction* action) {
        action->setText(buildMultiShortcutActionText(action->text(), action->shortcuts()));
    }
};
