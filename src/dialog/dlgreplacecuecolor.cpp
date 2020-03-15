#include "dialog/dlgreplacecuecolor.h"

#include <QAbstractButton>
#include <QColor>
#include <QColorDialog>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QtConcurrent>

#include "library/dao/cuedao.h"
#include "library/queryutil.h"
#include "util/color/rgbcolor.h"

namespace {

enum ReplaceColorConditionFlag {
    NoConditions = 0,
    CurrentColorCheck = 1,
    CurrentColorNotEqual = 2,
};
Q_DECLARE_FLAGS(ReplaceColorConditions, ReplaceColorConditionFlag);
Q_DECLARE_OPERATORS_FOR_FLAGS(ReplaceColorConditions);

constexpr int kColorButtonLightnessThreshold = 0x80;
const QString kColorButtonStyleSheetLight = QStringLiteral(
        "QPushButton { background-color: %1; }");
const QString kColorButtonStyleSheetDark = QStringLiteral(
        "QPushButton { background-color: %1; color: white; }");

void setButtonColor(QPushButton* button, const QColor& color) {
    button->setText(color.name());
    button->setStyleSheet((
            (color.lightness() >= kColorButtonLightnessThreshold)
                    ? kColorButtonStyleSheetLight
                    : kColorButtonStyleSheetDark)
                                  .arg(color.name()));
}

int updateCueColors(
        mixxx::DbConnectionPoolPtr dbConnectionPool,
        mixxx::RgbColor::optional_t newColor,
        mixxx::RgbColor::optional_t currentColor,
        ReplaceColorConditions conditions) {
    // The pooler limits the lifetime all thread-local connections,
    // that should be closed immediately before exiting this function.
    const mixxx::DbConnectionPooler dbConnectionPooler(dbConnectionPool);
    QSqlDatabase database = mixxx::DbConnectionPooled(dbConnectionPool);

    //Open the database connection in this thread.
    VERIFY_OR_DEBUG_ASSERT(database.isOpen()) {
        qWarning() << "Failed to open database for Serato parser."
                   << database.lastError();
        return -1;
    }

    //Give thread a low priority
    QThread* thisThread = QThread::currentThread();
    thisThread->setPriority(QThread::LowPriority);

    ScopedTransaction transaction(database);
    QSqlQuery query(database);

    if (conditions.testFlag(ReplaceColorConditionFlag::CurrentColorCheck)) {
        query.prepare(
                QStringLiteral("UPDATE " CUE_TABLE " SET color=:new_color WHERE color") +
                (conditions.testFlag(ReplaceColorConditionFlag::CurrentColorNotEqual) ? QStringLiteral("!=") : QStringLiteral("=")) +
                QStringLiteral(":current_color"));
        query.bindValue(":current_color", mixxx::RgbColor::toQVariant(currentColor));
    } else {
        query.prepare(
                QStringLiteral("UPDATE " CUE_TABLE " SET =:new_color"));
    }
    query.bindValue(":new_color", mixxx::RgbColor::toQVariant(newColor));

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return -1;
    }
    transaction.commit();

    return query.numRowsAffected();
}

} // namespace

DlgReplaceCueColor::DlgReplaceCueColor(
        mixxx::DbConnectionPoolPtr dbConnectionPool,
        QWidget* pParent)
        : QDialog(pParent),
          m_pDbConnectionPool(dbConnectionPool) {
    setupUi(this);

    setButtonColor(pushButtonNewColor, QColor(0, 0, 0));
    setButtonColor(pushButtonCurrentColor, QColor(0, 0, 0));

    connect(&m_dbFutureWatcher,
            &QFutureWatcher<int>::finished,
            this,
            &DlgReplaceCueColor::slotTransactionFinished);
    connect(pushButtonNewColor,
            &QPushButton::clicked,
            [this] {
                slotSelectColor(pushButtonNewColor);
            });
    connect(pushButtonCurrentColor,
            &QPushButton::clicked,
            [this] {
                slotSelectColor(pushButtonCurrentColor);
            });
    connect(buttonBox,
            &QDialogButtonBox::clicked,
            [this](QAbstractButton* button) {
                switch (buttonBox->buttonRole(button)) {
                case QDialogButtonBox::RejectRole:
                    hide();
                    break;
                case QDialogButtonBox::ApplyRole:
                    slotApply();
                    break;
                default:
                    break;
                };
            });
}

DlgReplaceCueColor::~DlgReplaceCueColor() {
    m_dbFuture.waitForFinished();
}

void DlgReplaceCueColor::slotSelectColor(QPushButton* button) {
    QColor initialColor = QColor(button->text());
    QColor color = QColorDialog::getColor(initialColor, this);
    setButtonColor(button, color);
}

void DlgReplaceCueColor::slotApply() {
    ReplaceColorConditions conditions = ReplaceColorConditionFlag::NoConditions;
    if (checkBoxCurrentColorCondition->isChecked()) {
        conditions |= ReplaceColorConditionFlag::CurrentColorCheck;
    }
    if (comboBoxCurrentColorCompare->currentText() == "is not") {
        conditions |= ReplaceColorConditionFlag::CurrentColorNotEqual;
    }

    mixxx::RgbColor::optional_t newColor = mixxx::RgbColor::fromQString(pushButtonNewColor->text());
    mixxx::RgbColor::optional_t currentColor = mixxx::RgbColor::fromQString(pushButtonCurrentColor->text());

    m_dbFuture = QtConcurrent::run(updateCueColors, m_pDbConnectionPool, newColor, currentColor, conditions);
    m_dbFutureWatcher.setFuture(m_dbFuture);
}

void DlgReplaceCueColor::slotTransactionFinished() {
    int numAffectedRows = m_dbFuture.result();
    if (numAffectedRows < 0) {
        QMessageBox::critical(this, tr("Error occured!"), tr("Database update failed. Please check the logs."));
    } else if (numAffectedRows == 0) {
        QMessageBox::warning(this, tr("No colors changed!"), tr("No database rows matched the specified criteria."));
    } else {
        QMessageBox::information(this, tr("Colors Replaced!"), tr("Done! %1 rows were affected.").arg(numAffectedRows));
    }
}
