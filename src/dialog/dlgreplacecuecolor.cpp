#include "dialog/dlgreplacecuecolor.h"

#include <QAbstractButton>
#include <QColor>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QResizeEvent>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStyleFactory>

#include "library/dao/cuedao.h"
#include "library/queryutil.h"
#include "moc_dlgreplacecuecolor.cpp"
#include "preferences/colorpalettesettings.h"
#include "track/track.h"
#include "util/color/color.h"
#include "util/color/predefinedcolorpalettes.h"
#include "util/defs.h"

namespace {

const QString kColorButtonStyleSheetLight = QStringLiteral(
        "QPushButton { background-color: %1; color: black; }");
const QString kColorButtonStyleSheetDark = QStringLiteral(
        "QPushButton { background-color: %1; color: white; }");

void setButtonColor(QPushButton* button, const QColor& color) {
    button->setText(color.name());
    button->setStyleSheet(
            (Color::isDimColor(color)
                            ? kColorButtonStyleSheetDark
                            : kColorButtonStyleSheetLight)
                    .arg(color.name()));
}

typedef struct {
    DbId id;
    TrackId trackId;
    mixxx::RgbColor color;
} CueDatabaseRow;

} // namespace

DlgReplaceCueColor::DlgReplaceCueColor(
        UserSettingsPointer pConfig,
        mixxx::DbConnectionPoolPtr dbConnectionPool,
        TrackCollectionManager* pTrackCollectionManager,
        QWidget* pParent)
        : QDialog(pParent),
          m_pConfig(pConfig),
          m_pDbConnectionPool(dbConnectionPool),
          m_pTrackCollectionManager(pTrackCollectionManager),
          m_bDatabaseChangeInProgress(false),
          m_pNewColorMenu(new QMenu(this)),
          m_pCurrentColorMenu(new QMenu(this)),
          m_lastAutoSetNewColor(std::nullopt),
          m_lastAutoSetCurrentColor(std::nullopt),
          m_pStyle(QStyleFactory::create(QStringLiteral("fusion"))) {
    setupUi(this);
    setWindowModality(Qt::ApplicationModal);

    spinBoxHotcueIndex->setMaximum(kMaxNumberOfHotcues);

    QIcon icon = QIcon::fromTheme("dialog-warning");
    if (!icon.isNull()) {
        labelReplaceAllColorsIcon->setPixmap(icon.pixmap(QSize(32, 32)));
    } else {
        labelReplaceAllColorsIcon->hide();
    }

    // Unfortunately, not all styles supported by Qt support setting a
    // background color for QPushButtons (see
    // https://bugreports.qt.io/browse/QTBUG-11089). For example, when using
    // the gtk2 style all color buttons would be just grey. It's possible to
    // work around this by modifying the button border with a QSS stylesheet,
    // so that the QStyle will be overwritten, but as a sane default for skins
    // without styles for WColorPicker, we're setting the platform-independent
    // "Fusion" style here. This will make the buttons look slightly different
    // from the rest of the application (when not styled via QSS), but that's
    // better than having buttons without any colors (which would make the
    // color picker unusable).
    pushButtonNewColor->setStyle(m_pStyle.get());
    pushButtonCurrentColor->setStyle(m_pStyle.get());

    // Set up new color button
    ColorPaletteSettings colorPaletteSettings(pConfig);
    ColorPalette hotcuePalette = colorPaletteSettings.getHotcueColorPalette();
    mixxx::RgbColor firstColor = mixxx::PredefinedColorPalettes::kDefaultCueColor;
    DEBUG_ASSERT(hotcuePalette.size() > 0);
    if (hotcuePalette.size() > 0) { // Should always be true
        firstColor = hotcuePalette.at(0);
    }
    setButtonColor(pushButtonNewColor, mixxx::RgbColor::toQColor(firstColor));

    // Add menu for 'New color' button
    m_pNewColorPickerAction = make_parented<WColorPickerAction>(
            WColorPicker::Option::AllowCustomColor |
                    // TODO(xxx) remove this once the preferences are themed via QSS
                    WColorPicker::Option::NoExtStyleSheet,
            colorPaletteSettings.getHotcueColorPalette(),
            this);
    m_pNewColorPickerAction->setObjectName("HotcueColorPickerAction");
    m_pNewColorPickerAction->setSelectedColor(firstColor);
    connect(m_pNewColorPickerAction,
            &WColorPickerAction::colorPicked,
            this,
            [this](mixxx::RgbColor::optional_t color) {
                if (color) {
                    setButtonColor(pushButtonNewColor, mixxx::RgbColor::toQColor(*color));
                    slotUpdateWidgets();
                }
                m_pNewColorMenu->hide();
            });
    m_pNewColorMenu->addAction(m_pNewColorPickerAction);
    pushButtonNewColor->setMenu(m_pNewColorMenu);

    // Set up 'Current color' button
    setButtonColor(pushButtonCurrentColor,
            mixxx::RgbColor::toQColor(
                    mixxx::PredefinedColorPalettes::kDefaultCueColor));

    // Update apply button when the current color comparison combobox is
    // modified
    connect(comboBoxCurrentColorCompare,
            &QComboBox::currentTextChanged,
            this,
            &DlgReplaceCueColor::slotUpdateWidgets);

    // Add menu for 'Current color' button
    m_pCurrentColorPickerAction = make_parented<WColorPickerAction>(
            WColorPicker::Option::AllowCustomColor |
                    // TODO(xxx) remove this once the preferences are themed via QSS
                    WColorPicker::Option::NoExtStyleSheet,
            colorPaletteSettings.getHotcueColorPalette(),
            this);
    m_pCurrentColorPickerAction->setObjectName("HotcueColorPickerAction");
    m_pCurrentColorPickerAction->setSelectedColor(
            mixxx::PredefinedColorPalettes::kDefaultCueColor);
    connect(m_pCurrentColorPickerAction,
            &WColorPickerAction::colorPicked,
            this,
            [this](mixxx::RgbColor::optional_t color) {
                if (color) {
                    setButtonColor(pushButtonCurrentColor,
                            mixxx::RgbColor::toQColor(*color));
                    slotUpdateWidgets();
                }
                m_pCurrentColorMenu->hide();
            });
    m_pCurrentColorMenu->addAction(m_pCurrentColorPickerAction);
    pushButtonCurrentColor->setMenu(m_pCurrentColorMenu);

    // Update dialog widgets when conditions checkboxes are (un)checked
    connect(checkBoxCurrentColorCondition,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this,
            &DlgReplaceCueColor::slotUpdateWidgets);
    connect(checkBoxHotcueIndexCondition,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this,
            &DlgReplaceCueColor::slotUpdateWidgets);

    connect(buttonBox,
            &QDialogButtonBox::clicked,
            this,
            [this](QAbstractButton* button) {
                switch (buttonBox->buttonRole(button)) {
                case QDialogButtonBox::RejectRole:
                    reject();
                    break;
                case QDialogButtonBox::ApplyRole:
                    slotApply();
                    break;
                default:
                    break;
                };
            });

    slotUpdateWidgets();
}

void DlgReplaceCueColor::setColorPalette(const ColorPalette& palette) {
    m_pNewColorPickerAction->setColorPalette(palette);
    QResizeEvent resizeNewColorMenuEvent(QSize(), m_pNewColorMenu->size());
    qApp->sendEvent(m_pNewColorMenu, &resizeNewColorMenuEvent);
    m_pCurrentColorPickerAction->setColorPalette(palette);
    QResizeEvent resizeCurrentColorMenuEvent(QSize(), m_pCurrentColorMenu->size());
    qApp->sendEvent(m_pCurrentColorMenu, &resizeCurrentColorMenuEvent);
}

void DlgReplaceCueColor::setNewColor(mixxx::RgbColor color) {
    mixxx::RgbColor::optional_t buttonColor = mixxx::RgbColor::fromQString(
            pushButtonNewColor->text());

    // Make sure we don't overwrite colors selected by the user
    if (!m_lastAutoSetNewColor || m_lastAutoSetNewColor == buttonColor) {
        setButtonColor(pushButtonNewColor, mixxx::RgbColor::toQColor(color));
        m_pNewColorPickerAction->setSelectedColor(color);
        slotUpdateWidgets();
    }

    m_lastAutoSetNewColor = color;
}

void DlgReplaceCueColor::setCurrentColor(mixxx::RgbColor color) {
    mixxx::RgbColor::optional_t buttonColor = mixxx::RgbColor::fromQString(
            pushButtonCurrentColor->text());

    // Make sure we don't overwrite colors selected by the user
    if (!m_lastAutoSetCurrentColor || m_lastAutoSetCurrentColor == buttonColor) {
        setButtonColor(pushButtonCurrentColor, mixxx::RgbColor::toQColor(color));
        m_pCurrentColorPickerAction->setSelectedColor(color);
        slotUpdateWidgets();
    }

    m_lastAutoSetCurrentColor = color;
}

void DlgReplaceCueColor::slotApply() {
    m_bDatabaseChangeInProgress = false;
    slotUpdateWidgets();

    // Get values for SELECT query
    QProgressDialog progress(this);
    progress.setWindowModality(Qt::ApplicationModal);
    progress.setAutoReset(false);
    progress.setLabelText(tr("Selecting database rows..."));
    progress.setMaximum(0);
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

    mixxx::RgbColor::optional_t currentColor =
            mixxx::RgbColor::fromQString(pushButtonCurrentColor->text());
    VERIFY_OR_DEBUG_ASSERT(currentColor) {
        m_bDatabaseChangeInProgress = false;
        slotUpdateWidgets();
        return;
    }

    mixxx::RgbColor::optional_t newColor =
            mixxx::RgbColor::fromQString(pushButtonNewColor->text());
    VERIFY_OR_DEBUG_ASSERT(newColor) {
        m_bDatabaseChangeInProgress = false;
        slotUpdateWidgets();
        return;
    }

    int hotcueIndex = spinBoxHotcueIndex->value() - 1;

    QSqlDatabase database = mixxx::DbConnectionPooled(m_pDbConnectionPool);

    // Open the database connection in this thread.
    VERIFY_OR_DEBUG_ASSERT(database.isOpen()) {
        qWarning() << "Failed to open database for cue color replace dialog."
                   << database.lastError();
        m_bDatabaseChangeInProgress = false;
        slotUpdateWidgets();
        return;
    }

    // Build SELECT query string
    QMap<QString, QVariant> queryValues;
    QStringList queryStringConditions;
    if (conditions.testFlag(ConditionFlag::CurrentColorCheck)) {
        queryStringConditions << QString(QStringLiteral("color") +
                (conditions.testFlag(ConditionFlag::CurrentColorNotEqual)
                                ? QStringLiteral("!=")
                                : QStringLiteral("=")) +
                QStringLiteral(":current_color"));
        queryValues.insert(QStringLiteral(":current_color"),
                mixxx::RgbColor::toQVariant(currentColor));
    }
    if (conditions.testFlag(ConditionFlag::HotcueIndexCheck)) {
        queryStringConditions << QString(QStringLiteral("hotcue") +
                (conditions.testFlag(ConditionFlag::HotcueIndexNotEqual)
                                ? QStringLiteral("!=")
                                : QStringLiteral("=")) +
                QStringLiteral(":hotcue"));
        queryValues.insert(QStringLiteral(":hotcue"), QVariant(hotcueIndex));
    }

    QString queryString =
            QStringLiteral("SELECT id, track_id, color FROM " CUE_TABLE
                           " WHERE color != :new_color");
    queryValues.insert(QStringLiteral(":new_color"),
            mixxx::RgbColor::toQVariant(newColor));
    if (!queryStringConditions.isEmpty()) {
        queryString += QStringLiteral(" AND ") +
                queryStringConditions.join(QStringLiteral(" AND "));
    }

    // Select affected queues
    QSqlQuery selectQuery(database);
    selectQuery.prepare(queryString);
    for (auto i = queryValues.constBegin(); i != queryValues.constEnd(); i++) {
        selectQuery.bindValue(i.key(), i.value());
    }

    // Flush cached tracks to database
    const QSet<TrackId> cachedTrackIds = GlobalTrackCacheLocker().getCachedTrackIds();
    for (const TrackId& trackId : cachedTrackIds) {
        TrackPointer pTrack = GlobalTrackCacheLocker().lookupTrackById(trackId);
        if (pTrack) {
            m_pTrackCollectionManager->saveTrack(pTrack);
        }
    }

    if (!selectQuery.exec()) {
        LOG_FAILED_QUERY(selectQuery);
        m_bDatabaseChangeInProgress = false;
        slotUpdateWidgets();
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
            m_bDatabaseChangeInProgress = false;
            slotUpdateWidgets();
            return;
        }
        mixxx::RgbColor::optional_t color =
                mixxx::RgbColor::fromQVariant(selectQuery.value(colorColumn));
        VERIFY_OR_DEBUG_ASSERT(color) {
            continue;
        }
        CueDatabaseRow row = {DbId(selectQuery.value(idColumn)),
                TrackId(selectQuery.value(trackIdColumn)),
                *color};
        rows << row;
        trackIds << row.trackId;
    }

    // Cue Selection finished
    progress.reset();

    if (rows.size() == 0) {
        QMessageBox::warning(this,
                tr("No colors changed!"),
                tr("No cues matched the specified criteria."));
        m_bDatabaseChangeInProgress = false;
        slotUpdateWidgets();
        return;
    }

    if (QMessageBox::question(this,
                tr("Confirm Color Replacement"),
                tr("The colors of %1 cues in %2 tracks will be replaced. This "
                   "change cannot be undone! Are you sure?")
                        .arg(QString::number(rows.size()),
                                QString::number(trackIds.size()))) ==
            QMessageBox::No) {
        m_bDatabaseChangeInProgress = false;
        slotUpdateWidgets();
        return;
    }

    // Update Cues
    progress.setLabelText("Updating cues...");
    progress.setValue(0);

    ScopedTransaction transaction(database);
    QSqlQuery query(database);
    query.prepare("UPDATE " CUE_TABLE
                  " SET color=:new_color WHERE id=:id AND track_id=:track_id "
                  "AND color=:current_color");
    query.bindValue(":new_color", mixxx::RgbColor::toQVariant(newColor));

    bool canceled = false;

    QMultiMap<TrackPointer, DbId> cues;
    for (const auto& row : std::as_const(rows)) {
        QCoreApplication::processEvents();
        if (progress.wasCanceled()) {
            canceled = true;
            break;
        }
        query.bindValue(":id", row.id.toVariant());
        query.bindValue(":track_id", row.trackId.toVariant());
        query.bindValue(":current_color", mixxx::RgbColor::toQVariant(row.color));
        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
            canceled = true;
            break;
        }

        if (query.numRowsAffected() > 0) {
            // If the track that these cues belong to is cached, store a
            // reference to them so that we can update the in-memory objects
            // after committing the database changes
            TrackPointer pTrack = GlobalTrackCacheLocker().lookupTrackById(row.trackId);
            if (pTrack) {
                cues.insert(pTrack, row.id);
            }
        }
    }

    if (canceled) {
        transaction.rollback();
    } else {
        transaction.commit();
        trackIds.clear();

        // Update the cue colors for in-memory track objects
        for (auto it = cues.constBegin(); it != cues.constEnd(); it++) {
            TrackPointer pTrack = it.key();
            VERIFY_OR_DEBUG_ASSERT(pTrack) {
                continue;
            }
            CuePointer pCue = pTrack->findCueById(it.value());
            if (pCue) {
                pCue->setColor(*newColor);
                trackIds << pTrack->getId();
            }
        }
        emit databaseTracksChanged(trackIds);
    }

    progress.reset();
    m_bDatabaseChangeInProgress = false;
    slotUpdateWidgets();
    accept();
}

void DlgReplaceCueColor::slotUpdateWidgets() {
    bool bEnabled = !m_bDatabaseChangeInProgress;
    if (bEnabled &&
            checkBoxCurrentColorCondition->isChecked() &&
            comboBoxCurrentColorCompare->currentText() == "is") {
        mixxx::RgbColor::optional_t currentColor = mixxx::RgbColor::fromQString(
                pushButtonCurrentColor->text());
        mixxx::RgbColor::optional_t newColor = mixxx::RgbColor::fromQString(
                pushButtonNewColor->text());
        if (currentColor == newColor) {
            bEnabled = false;
        }
    }

    if (checkBoxCurrentColorCondition->isChecked() || checkBoxHotcueIndexCondition->isChecked()) {
        frameReplaceAllColors->hide();
    } else {
        frameReplaceAllColors->show();
    }

    QPushButton* button = buttonBox->button(QDialogButtonBox::Apply);
    if (button) {
        button->setEnabled(bEnabled);
    }
}
