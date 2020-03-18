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

typedef struct {
    int id;
    TrackId trackId;
    mixxx::RgbColor color;
} CueDatabaseRow;

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
}

void DlgReplaceCueColor::slotApply() {
    setApplyButtonEnabled(false);

    // Get values for SELECT query
    QProgressDialog progress("Selecting database rows...", "Cancel", 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setAutoReset(false);
    progress.setValue(0);

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
        setApplyButtonEnabled(true);
        return;
    }

    mixxx::RgbColor::optional_t newColor = mixxx::RgbColor::fromQString(pushButtonNewColor->text());
    VERIFY_OR_DEBUG_ASSERT(newColor) {
        setApplyButtonEnabled(true);
        return;
    }

    int hotcueIndex = spinBoxHotcueIndex->value() - 1;

    // The pooler limits the lifetime of all thread-local connections. It will
    // be closed by its destructor when it goes out of scope
    const mixxx::DbConnectionPooler dbConnectionPooler(m_pDbConnectionPool);
    QSqlDatabase database = mixxx::DbConnectionPooled(m_pDbConnectionPool);

    // Open the database connection in this thread.
    VERIFY_OR_DEBUG_ASSERT(database.isOpen()) {
        qWarning() << "Failed to open database for cue color replace dialog."
                   << database.lastError();
        setApplyButtonEnabled(true);
        return;
    }

    // Build SELECT query string
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

    QString queryString = QStringLiteral("SELECT id, track_id, color FROM " CUE_TABLE);
    if (!queryStringConditions.isEmpty()) {
        queryString += QStringLiteral(" WHERE ") + queryStringConditions.join(QStringLiteral(" AND "));
    }

    // Select affected queues
    QSqlQuery selectQuery(database);
    selectQuery.prepare(queryString);
    for (auto i = queryValues.constBegin(); i != queryValues.constEnd(); i++) {
        selectQuery.bindValue(i.key(), i.value());
    }

    if (!selectQuery.exec()) {
        LOG_FAILED_QUERY(selectQuery);
        return;
    }
    int idColumn = selectQuery.record().indexOf("id");
    int trackIdColumn = selectQuery.record().indexOf("track_id");
    int colorColumn = selectQuery.record().indexOf("color");

    QList<CueDatabaseRow> rows;
    QSet<TrackId> trackIds;
    while (selectQuery.next()) {
        QCoreApplication::processEvents();
        if (progress.wasCanceled()) {
            setApplyButtonEnabled(true);
            return;
        }
        mixxx::RgbColor::optional_t color = mixxx::RgbColor::fromQVariant(selectQuery.value(colorColumn));
        VERIFY_OR_DEBUG_ASSERT(color) {
            continue;
        }
        CueDatabaseRow row = {
                .id = selectQuery.value(idColumn).toInt(),
                .trackId = TrackId(selectQuery.value(trackIdColumn).toInt()),
                .color = *color,
        };
        rows << row;
        trackIds << row.trackId;
    }

    // Cue Selection finished
    progress.reset();

    if (rows.size() == 0) {
        QMessageBox::warning(this, tr("No colors changed!"), tr("No cues matched the specified criteria."));
        setApplyButtonEnabled(true);
        return;
    }

    if (QMessageBox::question(
                this,
                tr("Really replace colors?"),
                tr("Really replace the colors of %1 cues in %2 tracks? This change cannot be undone!").arg(QString::number(rows.size()), QString::number(trackIds.size()))) == QMessageBox::No) {
        setApplyButtonEnabled(true);
        return;
    }

    // Update Cues
    progress.setLabelText("Updating cues...");
    progress.setValue(0);

    ScopedTransaction transaction(database);
    QSqlQuery query(database);
    query.prepare("UPDATE " CUE_TABLE " SET color=:new_color WHERE id=:id AND track_id=:track_id AND color=:current_color");
    query.bindValue(":new_color", mixxx::RgbColor::toQVariant(newColor));

    bool canceled = false;
    trackIds.clear();
    for (const auto& row : rows) {
        QCoreApplication::processEvents();
        if (progress.wasCanceled()) {
            canceled = true;
            break;
        }
        query.bindValue(":id", row.id);
        query.bindValue(":track_id", row.trackId.value());
        query.bindValue(":current_color", mixxx::RgbColor::toQVariant(row.color));
        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
            canceled = true;
            break;
        }

        if (query.numRowsAffected() > 0) {
            trackIds << row.trackId;
        }
    }

    if (canceled) {
        transaction.rollback();
    } else {
        transaction.commit();
        emit databaseTracksChanged(trackIds);
    }

    progress.reset();
    setApplyButtonEnabled(true);
}

void DlgReplaceCueColor::setApplyButtonEnabled(bool enabled) {
    QPushButton* button = buttonBox->button(QDialogButtonBox::Apply);
    if (button) {
        button->setEnabled(enabled);
    }
}
