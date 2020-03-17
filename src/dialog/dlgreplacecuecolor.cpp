#include "dialog/dlgreplacecuecolor.h"

#include <QAbstractButton>
#include <QColor>
#include <QColorDialog>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QtConcurrent>

#include "engine/controls/cuecontrol.h"
#include "library/dao/cuedao.h"
#include "library/queryutil.h"
#include "preferences/colorpalettesettings.h"

namespace {

enum class ReplaceColorConditionFlag {
    NoConditions = 0,
    CurrentColorCheck = 1,
    CurrentColorNotEqual = 1 << 1,
    HotcueIndexCheck = 1 << 2,
    HotcueIndexNotEqual = 1 << 3,
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
        int hotcueIndex,
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

    // Build query string
    QMap<QString, QVariant> queryValues;
    QStringList queryStringConditions;
    if (conditions.testFlag(ReplaceColorConditionFlag::CurrentColorCheck)) {
        queryStringConditions << QString(
                QStringLiteral("color") +
                (conditions.testFlag(ReplaceColorConditionFlag::CurrentColorNotEqual) ? QStringLiteral("!=") : QStringLiteral("=")) +
                QStringLiteral(":current_color"));
        queryValues.insert(QStringLiteral(":current_color"), mixxx::RgbColor::toQVariant(currentColor));
    }
    if (conditions.testFlag(ReplaceColorConditionFlag::HotcueIndexCheck)) {
        queryStringConditions << QString(
                QStringLiteral("hotcue") +
                (conditions.testFlag(ReplaceColorConditionFlag::HotcueIndexNotEqual) ? QStringLiteral("!=") : QStringLiteral("=")) +
                QStringLiteral(":hotcue"));
        queryValues.insert(QStringLiteral(":hotcue"), QVariant(hotcueIndex));
    }

    QString queryString = QStringLiteral("UPDATE " CUE_TABLE " SET color=:new_color");
    if (!queryStringConditions.isEmpty()) {
        queryString += QStringLiteral(" WHERE ") + queryStringConditions.join(QStringLiteral(" AND "));
    }

    // Execute query
    ScopedTransaction transaction(database);
    QSqlQuery query(database);
    query.prepare(queryString);
    query.bindValue(QStringLiteral(":new_color"), mixxx::RgbColor::toQVariant(newColor));
    for (auto i = queryValues.constBegin(); i != queryValues.constEnd(); i++) {
        query.bindValue(i.key(), i.value());
    }

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return -1;
    }
    transaction.commit();

    return query.numRowsAffected();
}

} // namespace

DlgReplaceCueColor::DlgReplaceCueColor(
        UserSettingsPointer pConfig,
        mixxx::DbConnectionPoolPtr dbConnectionPool,
        QWidget* pParent)
        : QDialog(pParent),
          m_pConfig(pConfig),
          m_pDbConnectionPool(dbConnectionPool),
          m_pNewColorMenu(new QMenu(this)),
          m_pCurrentColorMenu(new QMenu(this)) {
    setupUi(this);

    spinBoxHotcueIndex->setMaximum(NUM_HOT_CUES);

    // Set up new color button
    ColorPaletteSettings colorPaletteSettings(pConfig);
    mixxx::RgbColor firstColor = colorPaletteSettings.getHotcueColorPalette().at(0);
    setButtonColor(pushButtonNewColor, mixxx::RgbColor::toQColor(firstColor));

    // Add menu for new color button
    m_pNewColorPickerAction = new WColorPickerAction(WColorPicker::ColorOption::DenyNoColor, colorPaletteSettings.getHotcueColorPalette(), this);
    m_pNewColorPickerAction->setObjectName("HotcueColorPickerAction");
    m_pNewColorPickerAction->setSelectedColor(firstColor);
    connect(m_pNewColorPickerAction,
            &WColorPickerAction::colorPicked,
            [this](mixxx::RgbColor::optional_t color) {
                if (color) {
                    setButtonColor(pushButtonNewColor, mixxx::RgbColor::toQColor(*color));
                }
            });
    m_pNewColorMenu->addAction(m_pNewColorPickerAction);
    m_pNewColorMenu->addSeparator();
    m_pNewColorMenu->addAction("Other Color...", [this] {
        QColor initialColor = QColor(pushButtonNewColor->text());
        QColor color = QColorDialog::getColor(initialColor, this);
        setButtonColor(pushButtonNewColor, color);
        m_pNewColorPickerAction->setSelectedColor(mixxx::RgbColor::fromQColor(color));
    });
    pushButtonNewColor->setMenu(m_pNewColorMenu);

    // Set up current color button
    setButtonColor(pushButtonCurrentColor, mixxx::RgbColor::toQColor(ColorPalette::kDefaultCueColor));

    // Add menu for current color button
    m_pCurrentColorPickerAction = new WColorPickerAction(WColorPicker::ColorOption::DenyNoColor, colorPaletteSettings.getHotcueColorPalette(), this);
    m_pCurrentColorPickerAction->setObjectName("HotcueColorPickerAction");
    m_pNewColorPickerAction->setSelectedColor(ColorPalette::kDefaultCueColor);
    connect(m_pCurrentColorPickerAction,
            &WColorPickerAction::colorPicked,
            [this](mixxx::RgbColor::optional_t color) {
                if (color) {
                    setButtonColor(pushButtonCurrentColor, mixxx::RgbColor::toQColor(*color));
                }
            });
    m_pCurrentColorMenu->addAction(m_pCurrentColorPickerAction);
    m_pCurrentColorMenu->addSeparator();
    m_pCurrentColorMenu->addAction("Other Color...", [this] {
        QColor initialColor = QColor(pushButtonCurrentColor->text());
        QColor color = QColorDialog::getColor(initialColor, this);
        setButtonColor(pushButtonCurrentColor, color);
        m_pCurrentColorPickerAction->setSelectedColor(mixxx::RgbColor::fromQColor(color));
    });
    pushButtonCurrentColor->setMenu(m_pCurrentColorMenu);

    connect(&m_dbFutureWatcher,
            &QFutureWatcher<int>::finished,
            this,
            &DlgReplaceCueColor::slotTransactionFinished);
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

void DlgReplaceCueColor::slotApply() {
    ReplaceColorConditions conditions = ReplaceColorConditionFlag::NoConditions;
    if (checkBoxCurrentColorCondition->isChecked()) {
        conditions |= ReplaceColorConditionFlag::CurrentColorCheck;
    }
    if (comboBoxCurrentColorCompare->currentText() == "is not") {
        conditions |= ReplaceColorConditionFlag::CurrentColorNotEqual;
    }

    if (checkBoxHotcueIndexCondition->isChecked()) {
        conditions |= ReplaceColorConditionFlag::HotcueIndexCheck;
    }
    if (comboBoxHotcueIndexCompare->currentText() == "is not") {
        conditions |= ReplaceColorConditionFlag::HotcueIndexNotEqual;
    }

    mixxx::RgbColor::optional_t newColor = mixxx::RgbColor::fromQString(pushButtonNewColor->text());
    mixxx::RgbColor::optional_t currentColor = mixxx::RgbColor::fromQString(pushButtonCurrentColor->text());
    int hotcueIndex = spinBoxHotcueIndex->value() - 1;

    m_dbFuture = QtConcurrent::run(
            updateCueColors,
            m_pDbConnectionPool,
            newColor,
            currentColor,
            hotcueIndex,
            conditions);
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
