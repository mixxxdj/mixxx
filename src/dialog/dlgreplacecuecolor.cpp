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
                m_pNewColorMenu->hide();
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
                m_pCurrentColorMenu->hide();
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

    connect(&m_dbSelectFutureWatcher,
            &QFutureWatcher<int>::finished,
            this,
            &DlgReplaceCueColor::slotDatabaseIdsSelected);
    connect(&m_dbUpdateFutureWatcher,
            &QFutureWatcher<int>::finished,
            this,
            &DlgReplaceCueColor::slotDatabaseUpdated);
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
    m_dbSelectFuture.waitForFinished();
    m_dbUpdateFuture.waitForFinished();
}

void DlgReplaceCueColor::slotApply() {
    Conditions conditions = ConditionFlag::NoConditions;
    if (checkBoxCurrentColorCondition->isChecked()) {
        conditions |= ConditionFlag::CurrentColorCheck;
    }
    if (comboBoxCurrentColorCompare->currentText() == "is not") {
        conditions |= ConditionFlag::CurrentColorNotEqual;
    }

    if (checkBoxHotcueIndexCondition->isChecked()) {
        conditions |= ConditionFlag::HotcueIndexCheck;
    }
    if (comboBoxHotcueIndexCompare->currentText() == "is not") {
        conditions |= ConditionFlag::HotcueIndexNotEqual;
    }

    mixxx::RgbColor::optional_t currentColor = mixxx::RgbColor::fromQString(pushButtonCurrentColor->text());
    VERIFY_OR_DEBUG_ASSERT(currentColor) {
        return;
    }

    int hotcueIndex = spinBoxHotcueIndex->value() - 1;

    setApplyButtonEnabled(false);
    m_dbSelectFuture = QtConcurrent::run(
            this,
            &DlgReplaceCueColor::selectCues,
            *currentColor,
            hotcueIndex,
            conditions);
    m_dbSelectFutureWatcher.setFuture(m_dbSelectFuture);
}

void DlgReplaceCueColor::setApplyButtonEnabled(bool enabled) {
    QPushButton* button = buttonBox->button(QDialogButtonBox::Apply);
    if (button) {
        button->setEnabled(enabled);
    }
}

QMap<int, int> DlgReplaceCueColor::selectCues(
        mixxx::RgbColor::optional_t currentColor,
        int hotcueIndex,
        Conditions conditions) {
    // The pooler limits the lifetime of all thread-local connections. It will
    // be closed by its destructor when it goes out of scope
    const mixxx::DbConnectionPooler dbConnectionPooler(m_pDbConnectionPool);
    QSqlDatabase database = mixxx::DbConnectionPooled(m_pDbConnectionPool);

    //Open the database connection in this thread.
    VERIFY_OR_DEBUG_ASSERT(database.isOpen()) {
        qWarning() << "Failed to open database for cue color replace dialog."
                   << database.lastError();
        return {};
    }

    // Give thread a low priority
    QThread* thisThread = QThread::currentThread();
    thisThread->setPriority(QThread::LowPriority);

    // Build query string
    QMap<QString, QVariant> queryValues;
    QStringList queryStringConditions;
    if (conditions.testFlag(ConditionFlag::CurrentColorCheck)) {
        queryStringConditions << QString(
                QStringLiteral("color") +
                (conditions.testFlag(ConditionFlag::CurrentColorNotEqual) ? QStringLiteral("!=") : QStringLiteral("=")) +
                QStringLiteral(":current_color"));
        queryValues.insert(QStringLiteral(":current_color"), mixxx::RgbColor::toQVariant(currentColor));
    }
    if (conditions.testFlag(ConditionFlag::HotcueIndexCheck)) {
        queryStringConditions << QString(
                QStringLiteral("hotcue") +
                (conditions.testFlag(ConditionFlag::HotcueIndexNotEqual) ? QStringLiteral("!=") : QStringLiteral("=")) +
                QStringLiteral(":hotcue"));
        queryValues.insert(QStringLiteral(":hotcue"), QVariant(hotcueIndex));
    }

    QString queryString = QStringLiteral("SELECT id, track_id FROM " CUE_TABLE);
    if (!queryStringConditions.isEmpty()) {
        queryString += QStringLiteral(" WHERE ") + queryStringConditions.join(QStringLiteral(" AND "));
    }

    // Select affected queues
    QSqlQuery query(database);
    query.prepare(queryString);
    for (auto i = queryValues.constBegin(); i != queryValues.constEnd(); i++) {
        query.bindValue(i.key(), i.value());
    }

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return {};
    }
    QMap<int, int> ids;
    int idColumn = query.record().indexOf("id");
    int trackIdColumn = query.record().indexOf("track_id");
    while (query.next()) {
        ids.insert(query.value(idColumn).toInt(), query.value(trackIdColumn).toInt());
    }

    return ids;
}

void DlgReplaceCueColor::slotDatabaseIdsSelected() {
    QMap<int, int> ids = m_dbSelectFuture.result();

    if (ids.size() == 0) {
        QMessageBox::warning(this, tr("No colors changed!"), tr("No cues matched the specified criteria."));
        setApplyButtonEnabled(true);
        return;
    }

    QSet<int> cueIds;
    QSet<TrackId> trackIds;
    for (auto i = ids.constBegin(); i != ids.constEnd(); i++) {
        cueIds << i.key();
        trackIds << TrackId(i.value());
    }

    if (QMessageBox::question(
                this,
                tr("Really replace colors?"),
                tr("Really replace the colors of %1 cues in %2 tracks? This change cannot be undone!").arg(QString::number(cueIds.size()), QString::number(trackIds.size()))) == QMessageBox::No) {
        setApplyButtonEnabled(true);
        return;
    }

    mixxx::RgbColor::optional_t newColor = mixxx::RgbColor::fromQString(pushButtonNewColor->text());
    VERIFY_OR_DEBUG_ASSERT(newColor) {
        setApplyButtonEnabled(true);
        return;
    }

    m_dbUpdateFuture = QtConcurrent::run(
            this,
            &DlgReplaceCueColor::updateCues,
            cueIds,
            trackIds,
            *newColor);
    m_dbUpdateFutureWatcher.setFuture(m_dbUpdateFuture);
}

void DlgReplaceCueColor::updateCues(QSet<int> cueIds, QSet<TrackId> trackIds, mixxx::RgbColor newColor) {
    // The pooler limits the lifetime of all thread-local connections. It will
    // be closed by its destructor when it goes out of scope
    const mixxx::DbConnectionPooler dbConnectionPooler(m_pDbConnectionPool);
    QSqlDatabase database = mixxx::DbConnectionPooled(m_pDbConnectionPool);

    //Open the database connection in this thread.
    VERIFY_OR_DEBUG_ASSERT(database.isOpen()) {
        qWarning() << "Failed to open database for cue color replace dialog."
                   << database.lastError();
        return;
    }

    //Give thread a low priority
    QThread* thisThread = QThread::currentThread();
    thisThread->setPriority(QThread::LowPriority);

    ScopedTransaction transaction(database);
    QSqlQuery query(database);
    query.prepare("UPDATE " CUE_TABLE " SET color=:color WHERE id=:id");
    query.bindValue(":color", mixxx::RgbColor::toQVariant(newColor));

    bool queryFailed = false;
    for (const auto& id : cueIds) {
        query.bindValue(":id", id);
        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
            queryFailed = true;
            break;
        }
    }

    if (queryFailed) {
        transaction.rollback();
        return;
    }

    transaction.commit();
    emit databaseTracksChanged(trackIds);
}

void DlgReplaceCueColor::slotDatabaseUpdated() {
    setApplyButtonEnabled(true);
}
