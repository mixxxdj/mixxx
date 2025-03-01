#include "dlgbasesearchcrateinfo.h"

#include "library/trackset/basetracksetfeature.h"
#include "library/trackset/searchcrate/searchcratefuntions.h"
#include "moc_dlgbasesearchcrateinfo.cpp"

// const bool sDebug = false;

dlgBaseSearchCrateInfo::dlgBaseSearchCrateInfo(BaseTrackSetFeature* feature, QWidget* parent)
        : QDialog(parent),
          m_feature(feature) {
    setupUi(this);
    connect(buttonLock, &QPushButton::clicked, this, &dlgBaseSearchCrateInfo::toggleLockStatus);
    connect(m_feature,
            &BaseTrackSetFeature::updateSearchCrateData,
            this,
            &dlgBaseSearchCrateInfo::onUpdateSearchCrateData);
}

void dlgBaseSearchCrateInfo::init(const QVariantList& searchCratesData,
        const QVariantList& playlistsCratesData) {
    // Initialize tables and UI
    for (int i = 0; i < 8; ++i)
        headerTable[i] = "";
    for (int i = 0; i <= 12; ++i) {
        for (int j = 1; j <= 4; ++j) {
            conditionsTable[i][j] = "";
        }
    }
    for (int i = 0; i < 5; ++i)
        prevnextTable[i] = "";

    if (!searchCratesData.isEmpty()) {
        initHeaderTable(searchCratesData);
        initConditionsTable(searchCratesData);
        initPrevNextTable(searchCratesData);
    }
    if (!playlistsCratesData.isEmpty()) {
        initPlaylistCrateTable(playlistsCratesData);
    }
    connectConditions();
    setupConditionUI();
    populateUI();
    updateConditionState();
}

void dlgBaseSearchCrateInfo::connectConditions() {
    for (int i = 1; i <= 12; ++i) {
        auto* fieldComboBox = findChild<QComboBox*>(
                QStringLiteral("comboBoxCondition%1Field").arg(i));
        auto* operatorComboBox = findChild<QComboBox*>(
                QString("comboBoxCondition%1Operator").arg(i));
        auto* valueLineEdit = findChild<QLineEdit*>(
                QStringLiteral("lineEditCondition%1Value").arg(i));
        auto* valueComboBox = findChild<QComboBox*>(
                QStringLiteral("comboBoxCondition%1Value").arg(i));
        auto* combinerComboBox = findChild<QComboBox*>(
                QStringLiteral("comboBoxCondition%1Combiner").arg(i));

        if (fieldComboBox && operatorComboBox && valueLineEdit &&
                valueComboBox && combinerComboBox) {
            connect(fieldComboBox,
                    &QComboBox::currentTextChanged,
                    this,
                    &dlgBaseSearchCrateInfo::updateConditionState);
            connect(operatorComboBox,
                    &QComboBox::currentTextChanged,
                    this,
                    &dlgBaseSearchCrateInfo::updateConditionState);
            connect(valueLineEdit,
                    &QLineEdit::textChanged,
                    this,
                    &dlgBaseSearchCrateInfo::updateConditionState);
            connect(valueComboBox,
                    &QComboBox::currentTextChanged,
                    this,
                    &dlgBaseSearchCrateInfo::updateConditionState);
            connect(combinerComboBox,
                    &QComboBox::currentTextChanged,
                    this,
                    &dlgBaseSearchCrateInfo::updateConditionState);
        }
    }
}

void dlgBaseSearchCrateInfo::setupConditionUI() {
    for (int i = 1; i <= 12; ++i) {
        auto* fieldComboBox = findChild<QComboBox*>(
                QStringLiteral("comboBoxCondition%1Field").arg(i));
        auto* operatorComboBox = findChild<QComboBox*>(
                QString("comboBoxCondition%1Operator").arg(i));
        auto* valueComboBox = findChild<QComboBox*>(
                QStringLiteral("comboBoxCondition%1Value").arg(i));
        auto* insertConditionButton = findChild<QPushButton*>(
                QStringLiteral("buttoncondition%1_insert").arg(i));
        auto* deleteConditionButton = findChild<QPushButton*>(
                QStringLiteral("buttoncondition%1_delete").arg(i));
        auto* moveUpConditionButton =
                findChild<QPushButton*>(QStringLiteral("buttoncondition%1_up").arg(i));
        auto* moveDownConditionButton = findChild<QPushButton*>(
                QStringLiteral("buttoncondition%1_down").arg(i));

        if (fieldComboBox) {
            connect(fieldComboBox,
                    QOverload<int>::of(&QComboBox::currentIndexChanged),
                    this,
                    &dlgBaseSearchCrateInfo::onFieldComboBoxChanged);
        }
        if (operatorComboBox) {
            connect(operatorComboBox,
                    QOverload<int>::of(&QComboBox::currentIndexChanged),
                    this,
                    &dlgBaseSearchCrateInfo::onOperatorComboBoxChanged);
        }
        if (valueComboBox) {
            connect(valueComboBox,
                    QOverload<int>::of(&QComboBox::currentIndexChanged),
                    this,
                    &dlgBaseSearchCrateInfo::onValueComboBoxChanged);
        }
        if (insertConditionButton) {
            connect(insertConditionButton,
                    &QPushButton::clicked,
                    this,
                    [this, i]() { insertCondition(i); });
        }
        if (deleteConditionButton) {
            connect(deleteConditionButton,
                    &QPushButton::clicked,
                    this,
                    [this, i]() { removeCondition(i); });
        }
        if (moveUpConditionButton) {
            connect(moveUpConditionButton,
                    &QPushButton::clicked,
                    this,
                    [this, i]() { swapConditionWithAbove(i); });
        }
        if (moveDownConditionButton) {
            connect(moveDownConditionButton,
                    &QPushButton::clicked,
                    this,
                    [this, i]() { swapConditionWithBelow(i); });
        }
    }
}

void dlgBaseSearchCrateInfo::initHeaderTable(const QVariantList& searchCratesData) {
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> Initialize headerTable with searchCratesData ";
    }
    for (int i = 0; i < 8; ++i) {
        headerTable[i] = searchCratesData[i].isNull() ? "" : searchCratesData[i].toString();
    }
}

void dlgBaseSearchCrateInfo::initPrevNextTable(const QVariantList& searchCratesData) {
    // Start at position 56 for prev/next pointers
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> Initialize prevnextTable with searchCratesData";
    }
    for (int i = 0; i < 4; ++i) {
        prevnextTable[i] = searchCratesData[56 + i].isNull()
                ? ""
                : searchCratesData[56 + i].toString();
    }
}

void dlgBaseSearchCrateInfo::initConditionsTable(const QVariantList& searchCratesData) {
    int conditionStartIndex =
            4; // 8 fields in header, starting at 0, counter in for starts at 1
               // -> position where conditions start in searchCratesData
    int baseIndex = 0;
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> Initialize conditionsTable "
                    "with searchCratesData ";
    }
    for (int i = 1; i <= 12; ++i) {
        baseIndex = conditionStartIndex + (i * 4);
        for (int j = 1; j <= 4; ++j) {
            conditionsTable[i][j] = searchCratesData[baseIndex + (j - 1)].isNull()
                    ? ""
                    : searchCratesData[baseIndex + (j - 1)].toString();
        }
    }
}

void dlgBaseSearchCrateInfo::initPlaylistCrateTable(const QVariantList& playlistsCratesData) {
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> Initialize playlistTable & "
                    "historyTable & crateTable with playlistsCratesData ";
    }
    playlistTable.clear();
    playlistNameHash.clear();
    playlistIdHash.clear();
    historyTable.clear();
    historyNameHash.clear();
    historyIdHash.clear();
    crateTable.clear();
    crateNameHash.clear();
    crateIdHash.clear();

    for (const QVariant& entry : playlistsCratesData) {
        QVariantMap map = entry.toMap();
        QString type = map["type"].toString();
        QString id = map["id"].toString();
        QString name = map["name"].toString();
        if (type == "playlist") {
            playlistTable.append(qMakePair(id, name));
            playlistNameHash.insert(name, id);
            playlistIdHash.insert(id, name);
            if (sDebug) {
                qDebug() << "[SEARCHCRATES] [EDIT DLG] --> Init playlistTable "
                            "append id "
                         << id << " name " << name;
            }
        } else if (type == "history") {
            historyTable.append(qMakePair(id, name));
            historyNameHash.insert(name, id);
            historyIdHash.insert(id, name);
            if (sDebug) {
                qDebug() << "[SEARCHCRATES] [EDIT DLG] --> Init historyTable "
                            "append id "
                         << id << " name " << name;
            }
        } else if (type == "crate") {
            crateTable.append(qMakePair(id, name));
            crateNameHash.insert(name, id);
            crateIdHash.insert(id, name);
            if (sDebug) {
                qDebug() << "[SEARCHCRATES] [EDIT DLG] --> Init cratelistTable "
                            "append id "
                         << id << " name " << name;
            }
        }
    }
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> Finished playlistTable & "
                    "crateTable & historyTable with playlistsCratesData ";
    }
}

void dlgBaseSearchCrateInfo::onSetBlockerOff(const QString& blocker) {
    if (blocker == "previous") {
        //        m_sendPrevious = !m_sendPrevious; // set the onPrevious signal
        //        back to false after previousprocedure
        // qDebug() << "[SEARCHCRATES] [EDIT DLG] --> SIGNAL RCVD Received from
        // feature -> SET 'previous' off";
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> SIGNAL RCVD Received from "
                    "feature -> Change 'previous' flag to:"
                 << m_sendPrevious;
    } else if (blocker == "next") {
        //        m_sendNext = !m_sendNext; // set the onNext signal back to
        //        false after nextprocedure
        // qDebug() << "[SEARCHCRATES] [EDIT DLG] --> SIGNAL RCVD Received from
        // feature -> SET 'next' off";
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> SIGNAL RCVD Received from "
                    "feature -> Change 'next' flag to:"
                 << m_sendNext;
    } else if (blocker == "delete") {
        //        m_sendDelete = !m_sendDelete; // set the onDelete signal back
        //        to false after deleteprocedure
        // qDebug() << "[SEARCHCRATES] [EDIT DLG] --> SIGNAL RCVD Received from
        // feature -> SET 'delete' off";
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> SIGNAL RCVD Received from "
                    "feature -> Change 'delete' flag to:"
                 << m_sendDelete;
    } else if (blocker == "new") {
        //        m_sendNew = !m_sendNew; // set the onNew signal back to false
        //        after deleteprocedure
        // qDebug() << "[SEARCHCRATES] [EDIT DLG] --> SIGNAL RCVD Received from
        // feature -> SET 'new' off";
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> SIGNAL RCVD Received from "
                    "feature -> Change 'new' flag to:"
                 << m_sendNew;
    } else if (blocker == "update") {
        //        m_isUpdatingUI = !m_isUpdatingUI; // Set the flag to indicate UI is being updated
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> SIGNAL RCVD Received from "
                    "feature -> Change 'update' flag to:"
                 << m_isUpdatingUI;
    }
}

void dlgBaseSearchCrateInfo::onUpdateSearchCrateData(const QVariantList& searchCratesData) {
    m_isUpdatingUI = true; // Set the flag to indicate UI is being updated
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> SIGNAL RCVD Received Signal "
                    "from feature -> UPDATE Initializing with data:"
                 << searchCratesData;
    }
    initHeaderTable(searchCratesData);
    initConditionsTable(searchCratesData);
    initPrevNextTable(searchCratesData);
    //    m_sendDelete = false; // set the onDelete signal back to false after deleteprocedure
    //    m_sendPrevious = false; // set the onPrevious signal back to false after previousprocedure
    //    m_sendNext = false; // set the onNext signal back to false after nextprocedure
    //    m_sendNew = false; // set the onNew signal back to false after newprocedure
    populateUI();
}

QVariant dlgBaseSearchCrateInfo::getUpdatedData() const {
    // Collect and return the updated data
    return collectUIChanges();
}

void dlgBaseSearchCrateInfo::populateUI() {
    m_isUpdatingUI = true; // Set update flag to prevent emitting signals during population
    lineEditID->setText(headerTable[0]);
    lineEditID->setReadOnly(true);
    lineEditName->setText(headerTable[1]);
    spinBoxCount->setValue(headerTable[2].toInt());
    checkBoxShow->setChecked(headerTable[3] == "true");
    buttonLock->setText(headerTable[4] == "true" ? "Unlock" : "Lock");
    checkBoxAutoDJ->setChecked(headerTable[5] == "true");
    lineEditSearchInput->setText(headerTable[6]);
    lineEditSearchSQL->setReadOnly(true);
    lineEditSearchSQL->setStyleSheet("font: 10pt");
    lineEditSearchSQL->setText(headerTable[7]);
    //    lineEditSearchSQL->setToolTip(
    textEditSearchSQL->setText(headerTable[7]);
    textEditSearchSQL->setStyleSheet("font: 10pt");
    //    textEditSearchSQL->setToolTip(

    QStringList fieldOptions = {"",
            "artist",
            "title",
            "album",
            "album_artist",
            "genre",
            "comment",
            "composer",
            "filetype",
            "key",
            "year",
            "datetime_added",
            "last_played_at",
            "duration",
            "bpm",
            "played",
            "timesplayed",
            "rating",
            "track",
            "playlist",
            "crate",
            "history"};
    QStringList operatorOptions = {"",
            "contains",
            "does not contain",
            "equal to",
            "not equal to",
            "starts with",
            "ends with",
            "is not empty",
            "is empty",
            "before",
            "after",
            "last",
            "last",
            "equal to",
            "less than",
            "greater than",
            "equal to",
            "not equal to",
            "between",
            "is a member of",
            "is not a member of",
            "is",
            "is not"};
    QStringList combinerOptions = {"", ") END", "AND", "OR", ") AND (", ") OR ("};

    for (int i = 1; i <= 12; ++i) {
        // Populate fieldComboBox
        auto* fieldComboBox = findChild<QComboBox*>(
                QStringLiteral("comboBoxCondition%1Field").arg(i));
        auto* operatorComboBox = findChild<QComboBox*>(
                QStringLiteral("comboBoxCondition%1Operator").arg(i));
        auto* valueLineEdit = findChild<QLineEdit*>(
                QStringLiteral("lineEditCondition%1Value").arg(i));
        auto* valueComboBox = findChild<QComboBox*>(
                QStringLiteral("comboBoxCondition%1Value").arg(i));
        auto* combinerComboBox = findChild<QComboBox*>(
                QStringLiteral("comboBoxCondition%1Combiner").arg(i));

        if (fieldComboBox) {
            QSignalBlocker blocker(fieldComboBox);
            fieldComboBox->clear();
            fieldComboBox->addItems(fieldOptions);
            fieldComboBox->setCurrentText(conditionsTable[i][1]);
            //            fieldComboBox->setToolTip(tr("Create a new condition") + "\n\n" +
        }

        if (operatorComboBox) {
            QSignalBlocker blocker(operatorComboBox);
            operatorComboBox->clear();
            operatorComboBox->addItems(operatorOptions);
            operatorComboBox->setCurrentText(conditionsTable[i][2]);
            //            operatorComboBox->setToolTip(
        }

        if (valueLineEdit && valueComboBox) {
            QSignalBlocker blocker(valueComboBox);
            if (conditionsTable[i][1] == "playlist") {
                valueLineEdit->setVisible(false);
                valueComboBox->setVisible(true);
                valueComboBox->clear();
                for (auto it = playlistTable.constBegin(); it != playlistTable.constEnd(); ++it) {
                    valueComboBox->addItem(it->second);
                }
                if (conditionsTable[i][3].indexOf("|||", 0) > 0) {
                    int posBar = conditionsTable[i][3].indexOf("|||", 0);
                    QString playlistId = conditionsTable[i][3].mid(0, posBar);
                    QString playlistName;
                    // playlist could be renamed, id stays -> get name
                    if (playlistIdHash.contains(playlistId)) {
                        playlistName = playlistIdHash.value(playlistId);
                        if (sDebug) {
                            qDebug() << "name for playlist ID -> " << playlistId
                                     << " is " << playlistName;
                        }
                        valueComboBox->setCurrentText(playlistName);
                    } else {
                        if (sDebug) {
                            qDebug() << "Id not in playlistIdHash: " << playlistId;
                        }
                    }
                    if (sDebug) {
                        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> POPULATE "
                                    "comboBoxValue -> playlistName "
                                 << playlistName;
                    }
                }
                if (sDebug) {
                    qDebug() << "[SEARCHCRATES] [EDIT DLG] --> POPULATE "
                                "comboBoxValue -> conditionsTable[i][3] "
                             << conditionsTable[i][3];
                }

            } else if (conditionsTable[i][1] == "history") {
                valueLineEdit->setVisible(false);
                valueComboBox->setVisible(true);
                valueComboBox->clear();
                for (auto it = historyTable.constBegin(); it != historyTable.constEnd(); ++it) {
                    valueComboBox->addItem(it->second);
                }
                //  crate could be renamed, id stays -> get name
                if (conditionsTable[i][3].indexOf("|||", 0) > 0) {
                    int posBar = conditionsTable[i][3].indexOf("|||", 0);
                    QString historyId = conditionsTable[i][3].mid(0, posBar);
                    QString historyName;
                    if (historyIdHash.contains(historyId)) {
                        historyName = historyIdHash.value(historyId);
                        if (sDebug) {
                            qDebug() << "name for history ID -> " << historyId
                                     << " is " << historyName;
                        }
                        valueComboBox->setCurrentText(historyName);
                    } else {
                        if (sDebug) {
                            qDebug() << "Id not in historyIdHash: " << historyId;
                        }
                    }
                    if (sDebug) {
                        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> POPULATE "
                                    "comboBoxValue -> historyName "
                                 << historyName;
                    }
                }
                if (sDebug) {
                    qDebug() << "[SEARCHCRATES] [EDIT DLG] --> POPULATE "
                                "comboBoxValue -> conditionsTable[i][3] "
                             << conditionsTable[i][3];
                }

            } else if (conditionsTable[i][1] == "crate") {
                valueLineEdit->setVisible(false);
                valueComboBox->setVisible(true);
                valueComboBox->clear();
                for (auto it = crateTable.constBegin(); it != crateTable.constEnd(); ++it) {
                    valueComboBox->addItem(it->second);
                }
                //  crate could be renamed, id stays -> get name
                if (conditionsTable[i][3].indexOf("|||", 0) > 0) {
                    int posBar = conditionsTable[i][3].indexOf("|||", 0);
                    QString crateId = conditionsTable[i][3].mid(0, posBar);
                    QString crateName;
                    if (crateIdHash.contains(crateId)) {
                        crateName = crateIdHash.value(crateId);
                        if (sDebug) {
                            qDebug() << "name for crate ID -> " << crateId << " is " << crateName;
                        }
                        valueComboBox->setCurrentText(crateName);
                    } else {
                        if (sDebug) {
                            qDebug() << "Id not in crateIdHash: " << crateId;
                        }
                    }
                    if (sDebug) {
                        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> POPULATE "
                                    "comboBoxValue -> crateName "
                                 << crateName;
                    }
                }
                if (sDebug) {
                    qDebug() << "[SEARCHCRATES] [EDIT DLG] --> POPULATE "
                                "comboBoxValue -> conditionsTable[i][3] "
                             << conditionsTable[i][3];
                }
                // track in all crates / playlists / historylists
            } else if (conditionsTable[i][1] == "track") {
                valueLineEdit->setVisible(false);
                valueComboBox->setVisible(true);
                valueComboBox->clear();
                for (auto it = crateTable.constBegin(); it != crateTable.constEnd(); ++it) {
                    valueComboBox->addItem(it->second);
                }
                valueComboBox->addItem("all playlists");
                valueComboBox->addItem("all crates");
                valueComboBox->addItem("all historylists");
                valueComboBox->setCurrentText(conditionsTable[i][3]);
                if (sDebug) {
                    qDebug() << "[SEARCHCRATES] [EDIT DLG] --> POPULATE "
                                "comboBoxValue -> conditionsTable[i][3] "
                             << conditionsTable[i][3];
                }
            } else {
                if (sDebug) {
                    qDebug() << "[SEARCHCRATES] [EDIT DLG] --> POPULATE "
                                "comboBoxValue -> NO playlist / NO crate";
                }
                valueLineEdit->setVisible(true);
                valueLineEdit->setText(conditionsTable[i][3]);
                valueComboBox->setVisible(false);
            }
            //            valueLineEdit->setToolTip(tr("3. Enter the Condition Value") + "\n\n" +
        }

        if (combinerComboBox) {
            QSignalBlocker blocker(combinerComboBox);
            combinerComboBox->clear();
            combinerComboBox->addItems(combinerOptions);
            combinerComboBox->setCurrentText(conditionsTable[i][4]);
        }
    }

    //    connectConditions();
    buttoncondition1_up->setEnabled(false);
    buttoncondition12_down->setEnabled(false);

    // Pagebuttons
    connect(applyButton,
            &QPushButton::clicked,
            this,
            &dlgBaseSearchCrateInfo::onApplyButtonClicked);
    connect(newButton, &QPushButton::clicked, this, &dlgBaseSearchCrateInfo::onNewButtonClicked);
    connect(deleteButton,
            &QPushButton::clicked,
            this,
            &dlgBaseSearchCrateInfo::onDeleteButtonClicked);
    connect(previousButton,
            &QPushButton::clicked,
            this,
            &dlgBaseSearchCrateInfo::onPreviousButtonClicked);
    connect(nextButton, &QPushButton::clicked, this, &dlgBaseSearchCrateInfo::onNextButtonClicked);
    connect(okButton, &QPushButton::clicked, this, &dlgBaseSearchCrateInfo::onOKButtonClicked);
    connect(cancelButton,
            &QPushButton::clicked,
            this,
            &dlgBaseSearchCrateInfo::onCancelButtonClicked);
    // set
    m_isUpdatingUI = false;
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> POPULATEUI READY" << headerTable[0];
    }
}

// narrow possible operator selections based on field selection
void dlgBaseSearchCrateInfo::onFieldComboBoxChanged() {
    QStringList stringFieldOptions = {"artist",
            "title",
            "album",
            "album_artist",
            "genre",
            "comment",
            "composer",
            "filetype",
            "key"};
    QStringList dateFieldOptions = {"year", "datetime_added", "last_played_at"};
    QStringList numberFieldOptions = {"duration", "bpm", "played", "timesplayed", "rating"};
    QStringList playlistCrateFieldOptions = {"playlist", "crate", "history"};
    QStringList trackFieldOptions = {"track"};
    QStringList stringOperatorOptions = {"",
            "contains",
            "does not contain",
            "equal to",
            "not equal to",
            "starts with",
            "ends with",
            "is empty",
            "is not empty"};
    QStringList dateOperatorOptions = {"", "before", "after", "last", "equal to", "between"};
    QStringList numberOperatorOptions = {"",
            "less than",
            "greater than",
            "equal to",
            "not equal to",
            "between"};
    QStringList playlistCrateOperatorOptions = {"", "is", "is not"};
    QStringList trackOperatorOptions = {
            "",
            "is a member of",
            "is not a member of"};
    QStringList operatorOptions = {"",
            "contains",
            "does not contain",
            "equal to",
            "not equal to",
            "starts with",
            "ends with",
            "is empty",
            "is not empty",
            "before",
            "after",
            "last",
            "equal to",
            "less than",
            "greater than",
            "equal to",
            "not equal to",
            "is",
            "is not"};

    // Find the field combo box
    QComboBox* fieldComboBox = qobject_cast<QComboBox*>(sender());
    if (!fieldComboBox) {
        if (sDebug) {
            qDebug() << "Field ComboBox not found!";
        }
        return;
    }
    int conditionCounter = fieldComboBox->objectName()
                                   .replace("comboBoxCondition", "")
                                   .replace("Field", "")
                                   .toInt();
    conditionsTable[conditionCounter][1] = fieldComboBox->currentText();
    if (sDebug) {
        qDebug() << "conditionCounter field: " << conditionCounter;
    }
    auto* valueLineEdit = findChild<QLineEdit*>(
            QStringLiteral("lineEditCondition%1Value").arg(conditionCounter));
    auto* valueComboBox = findChild<QComboBox*>(
            QStringLiteral("comboBoxCondition%1Value").arg(conditionCounter));
    valueLineEdit->clearMask();
    valueLineEdit->setInputMask("");
    if (sDebug) {
        qDebug() << "conditionCounter field: fieldComboBox->currentText()"
                 << fieldComboBox->currentText();
    }

    // Determine which operator options to display based on the selected field
    if (stringFieldOptions.contains(conditionsTable[conditionCounter][1])) {
        operatorOptions = stringOperatorOptions;
        labelValidation->setText(QString("Information:"));
        textEditValidation->setStyleSheet("font: 10pt");
        textEditValidation->setText(
                QString("You have different search options for strings: "
                        "contains, does not contain, starts with, ends with "
                        "... only enter the searchstring (no wildcards)"));
    } else if (dateFieldOptions.contains(conditionsTable[conditionCounter][1])) {
        labelValidation->setText(QStringLiteral("Information:"));
        textEditValidation->setStyleSheet("font: 10pt");
        textEditValidation->setText(QStringLiteral(
                "You have different search options for 'date'-like fields:"));
        operatorOptions = dateOperatorOptions;
    } else if (numberFieldOptions.contains(conditionsTable[conditionCounter][1])) {
        labelValidation->setText(QStringLiteral("Information:"));
        textEditValidation->setStyleSheet("font: 10pt");
        textEditValidation->setText(QStringLiteral(
                "You have different search options for 'number'-like fields:"));
        operatorOptions = numberOperatorOptions;
        // track in all crates / playlists / historylists
    } else if (trackFieldOptions.contains(conditionsTable[conditionCounter][1])) {
        labelValidation->setText(QStringLiteral("Information:"));
        textEditValidation->setStyleSheet("font: 10pt");
        textEditValidation->setText(
                QStringLiteral("You have different search options for 'track':"));
        operatorOptions = trackOperatorOptions;
    } else if (playlistCrateFieldOptions.contains(conditionsTable[conditionCounter][1])) {
        labelValidation->setText(QStringLiteral("Information:"));
        textEditValidation->setStyleSheet("font: 10pt");
        textEditValidation->setText(
                QStringLiteral("You have different search options for 'playlist, crate and "
                               "history':"));
        operatorOptions = playlistCrateOperatorOptions;
    } else if (conditionsTable[conditionCounter][1] == "") {
        valueLineEdit->setVisible(true);
        valueComboBox->setVisible(false);
        labelValidation->setText(QStringLiteral("Information:"));
        textEditValidation->setStyleSheet("font: 10pt");
        textEditValidation->setText(QStringLiteral(
                "Select a field to start creating a condition':"));
    }

    QStringView objectNameView(fieldComboBox->objectName());
    QStringView prefixView(u"comboBoxCondition"); // Unicode string literal
    int index = objectNameView.mid(prefixView.length(), 1).toInt();
    QComboBox* operatorComboBox = findChild<QComboBox*>(
            QString("comboBoxCondition%1Operator").arg(index));

    if (operatorComboBox) {
        QString tempOperator = operatorComboBox->currentText();
        if (fieldComboBox->currentText() == "") {
            QString tempOperator = "";
            if (sDebug) {
                qDebug() << "tempOperator = empty " << tempOperator;
            }
        }
        operatorComboBox->clear();
        operatorComboBox->addItems(operatorOptions);
        operatorComboBox->setCurrentText(tempOperator);
    }
}

// narrow possible operator selections based on field selection
void dlgBaseSearchCrateInfo::onOperatorComboBoxChanged() {
    QStringList stringFieldOptions = {"artist",
            "title",
            "album",
            "album_artist",
            "genre",
            "comment",
            "composer",
            "filetype",
            "key"};
    QStringList numberFieldOptions = {"duration", "bpm", "played", "timesplayed", "rating"};
    QStringList playlistCrateFieldOptions = {"playlist", "crate", "history"};
    QStringList trackFieldOptions = {"track"};

    // Find the field combo box
    QComboBox* operatorComboBox = qobject_cast<QComboBox*>(sender());
    if (!operatorComboBox) {
        if (sDebug) {
            qDebug() << "Operator ComboBox not found!";
        }
        return;
    }
    int conditionCounter = operatorComboBox->objectName()
                                   .replace("comboBoxCondition", "")
                                   .replace("Operator", "")
                                   .toInt();
    conditionsTable[conditionCounter][2] = operatorComboBox->currentText();
    auto* valueLineEdit = findChild<QLineEdit*>(
            QStringLiteral("lineEditCondition%1Value").arg(conditionCounter));
    auto* valueComboBox = findChild<QComboBox*>(
            QStringLiteral("comboBoxCondition%1Value").arg(conditionCounter));

    // date or number regex for 'datetime_added' and 'last_played_at'
    QRegularExpression dateOrNum(
            "0000|(19[0-9][0-9]|20[0-9][0-9])-(0[1-9]|[1][0-2])-(0[1-9]|[12][0-"
            "9]|3[01])");
    //    QValidator *checkDateOrNum = new QRegExpValidator(dateOrNum, this);
    //    QRegExp rx("0000|(19[0-9][0-9]|20[0-9][0-9])-(0[1-9]|[1][0-2])-(0[1-9]|[12][0-9]|3[01])");
    //    QValidator* checkDateOrNum = new QRegularExpressionValidator(dateOrNum, this);

    // Determine which operator options to display based on the selected field
    if (sDebug) {
        qDebug()
                << "conditionCounter field: fieldComboBox->conditionCounter][1]"
                << conditionsTable[conditionCounter][1];
    }
    if (stringFieldOptions.contains(conditionsTable[conditionCounter][1])) {
        valueLineEdit->setVisible(true);
        valueComboBox->setVisible(false);
        valueLineEdit->setText("");
        valueLineEdit->clearMask();
        valueLineEdit->setInputMask("");
        valueLineEdit->setPlaceholderText("e.g. abc");
        labelValidation->setText(QString("Information:"));
        textEditValidation->setStyleSheet("color: rgb(0,0,0)");
        textEditValidation->setStyleSheet("font: 10pt");
        if (conditionsTable[conditionCounter][2] == "contains") {
            textEditValidation->setText(
                    QStringLiteral("'contains' -> value format: (*abc*) [abc] \n"
                                   "-> With 'contains' you can search for tracks with "
                                   "'%1' containing the 'searchtext' you enter in the "
                                   "inputbox.\n"
                                   "-> The 'searchtext' can consist of multiple "
                                   "words, but the engine will check the exact "
                                   "similarity. \n"
                                   "-> If you want to search for multiple words in "
                                   "different order you need to create multiple "
                                   "conditions. \n"
                                   "-> Only enter the 'searchtext' (no wildcards)")
                            .arg(conditionsTable[conditionCounter][1]));
        } else if (conditionsTable[conditionCounter][2] == "does not contain") {
            textEditValidation->setText(
                    QStringLiteral("'does not contain' -> value format: (*abc*) [abc] "
                                   "\n"
                                   "-> With 'does not contain' you can search for "
                                   "tracks with '%1' NOT containing the 'searchtext' "
                                   "you enter in the inputbox.\n"
                                   "-> The 'searchtext' can consist of multiple "
                                   "words, but the engine will check the exact "
                                   "similarity. \n"
                                   "-> If you want to search for multiple words in "
                                   "different order you need to create multiple "
                                   "conditions. \n"
                                   "-> Only enter the 'searchtext' (no wildcards)")
                            .arg(conditionsTable[conditionCounter][1]));
        } else if (conditionsTable[conditionCounter][2] == "equal to") {
            textEditValidation->setText(
                    QString("'equal to' -> value format: (*abc*) [abc] \n"
                            "-> With 'equal to' you can search for tracks with "
                            "'%1' equal to exact the 'searchtext' you enter in "
                            "the inputbox.\n"
                            "-> The 'searchtext' can consist of multiple "
                            "words, but the engine will check the exact "
                            "similarity, no more or less.\n"
                            "-> Only enter the 'searchtext' (no wildcards)")
                            .arg(conditionsTable[conditionCounter][1]));
        } else if (conditionsTable[conditionCounter][2] == "not equal to") {
            textEditValidation->setText(
                    QStringLiteral("'not equal to' -> value format: (*abc*) [abc] \n"
                                   "-> With 'not equal to' you can search for tracks "
                                   "with '%1' is different from the exact'searchtext' "
                                   "you enter in the inputbox.\n"
                                   "-> The 'searchtext' can consist of multiple "
                                   "words, but the engine will check the exact "
                                   "similarity, no more or less.\n"
                                   "-> Only enter the 'searchtext' (no wildcards)")
                            .arg(conditionsTable[conditionCounter][1]));
        } else if (conditionsTable[conditionCounter][2] == "starts with") {
            textEditValidation->setText(
                    QStringLiteral("'starts with' -> value format: (abc*) [abc] \n"
                                   "-> With 'starts with' you can search for tracks "
                                   "with '%1' starting with the 'searchtext' you "
                                   "enter in the inputbox.\n"
                                   "-> The 'searchtext' can consist of multiple "
                                   "words, but the engine will check the exact "
                                   "similarity.\n"
                                   "-> If you want to search for multiple words in "
                                   "different order you need to create multiple "
                                   "conditions. \n"
                                   "-> Only enter the 'searchtext' (no wildcards)")
                            .arg(conditionsTable[conditionCounter][1]));
        } else if (conditionsTable[conditionCounter][2] == "ends with") {
            textEditValidation->setText(
                    QStringLiteral("'ends with' -> value format: (*abc) [abc] \n"
                                   "-> With 'ends with' you can search for tracks "
                                   "with '%1' ending with the 'searchtext' you enter "
                                   "in the inputbox.\n"
                                   "-> The 'searchtext' can consist of multiple "
                                   "words, but the engine will check the exact "
                                   "similarity.\n"
                                   "-> If you want to search for multiple words in "
                                   "different order you need to create multiple "
                                   "conditions. \n"
                                   "-> Only enter the 'searchtext' (no wildcards)")
                            .arg(conditionsTable[conditionCounter][1]));
        } else if (conditionsTable[conditionCounter][2] == "is empty") {
            valueLineEdit->setVisible(false);
            valueLineEdit->setText("is empty");
            textEditValidation->setText(
                    QStringLiteral("'is empty' -> value format: "
                                   " [] \n"
                                   "-> With 'is empty' you can search for tracks that "
                                   "have an 'Null' value in '%1' in the database.\n"
                                   "-> You must not enter a 'searchtext', leave the "
                                   "inputbox empty.\n"));
        } else if (conditionsTable[conditionCounter][2] == "is not empty") {
            valueLineEdit->setVisible(false);
            valueLineEdit->setText("is not empty");
            textEditValidation->setText(
                    QStringLiteral("'is not empty' -> value format: "
                                   " [] \n"
                                   "-> With 'is not empty' you can search for tracks that "
                                   "don't have an 'Null' value in '%1' in the database.\n"
                                   "-> You must not enter a 'searchtext', leave the inputbox "
                                   "empty.\n"));
        }
    } else if (conditionsTable[conditionCounter][1] == "year") {
        valueLineEdit->setVisible(true);
        valueComboBox->setVisible(false);
        valueLineEdit->clearMask();
        valueLineEdit->setPlaceholderText("e.g. 2016");
        if (conditionsTable[conditionCounter][2] == "equal to") {
            valueLineEdit->setInputMask("9999"); // Year: 4-digit format
            textEditValidation->setText(
                    QStringLiteral("'equal to' -> value format: (Year) [YYYY] \n"
                                   "-> With 'equal to' you can search for tracks with "
                                   "'%1' exactly equal to the 'searchyear' you enter "
                                   "in the inputbox.\n"
                                   "-> The entered year will not be included in the "
                                   "results.\n"
                                   "-> Only enter the searchyear in the YYYY format "
                                   "(no wildcards)")
                            .arg(conditionsTable[conditionCounter][1]));
        } else if (conditionsTable[conditionCounter][2] == "before") {
            valueLineEdit->setInputMask("9999"); // Year: 4-digit format
            textEditValidation->setText(
                    QStringLiteral("'before' -> value format: (Year) [YYYY] \n"
                                   "-> With 'before' you can search for tracks with "
                                   "'%1' is before the 'searchyear' you enter in the "
                                   "inputbox.\n"
                                   "-> The entered year will not be included in the "
                                   "results.\n"
                                   "-> Only enter the searchyear in the YYYY format "
                                   "(no wildcards)")
                            .arg(conditionsTable[conditionCounter][1]));
        } else if (conditionsTable[conditionCounter][2] == "after") {
            valueLineEdit->setInputMask("9999"); // Year: 4-digit format
            textEditValidation->setText(
                    QStringLiteral("'after' -> value format: (Year) [YYYY] \n"
                                   "-> With 'after' you can search for tracks with "
                                   "'%1' is after the 'searchyear' you enter in the "
                                   "inputbox.\n"
                                   "-> The entered year will not be included in the "
                                   "results.\n"
                                   "-> Only enter the 'searchyear' in the YYYY format "
                                   "(no wildcards)")
                            .arg(conditionsTable[conditionCounter][1]));
        } else if (conditionsTable[conditionCounter][2] == "between") {
            valueLineEdit->setInputMask("9999|9999"); // Year: 4-digit format
            valueLineEdit->setPlaceholderText("e.g. 2016|2022");
            textEditValidation->setText(
                    QStringLiteral("'between' -> value format: (Year) [YYYY|YYYY] \n"
                                   "-> With 'between' you can search for tracks with "
                                   "'%1' is between the both 'searchyears' you enter "
                                   "in the inputbox.\n"
                                   "-> The entered years will not be included in the "
                                   "results.\n"
                                   "-> Only enter the 'searchyears' in the YYYY "
                                   "format (no wildcards) divided by the | symbol")
                            .arg(conditionsTable[conditionCounter][1]));
        }
    } else if (conditionsTable[conditionCounter][1] == "datetime_added" ||
            conditionsTable[conditionCounter][1] == "last_played_at") {
        //        QRegExp
        //        R_date("0000|(19[0-9][0-9]|20[0-9][0-9])-(0[1-9]|[1][0-2])-(0[1-9]|[12][0-9]|3[01])");
        valueLineEdit->setVisible(true);
        valueComboBox->setVisible(false);
        valueLineEdit->clearMask();
        if (conditionsTable[conditionCounter][2] == "equal to") {
            valueLineEdit->setPlaceholderText("e.g. 2016-02-21 or number X");
            //            valueLineEdit->setInputMask("9999-99-99"); // Date: YYYY-MM-DD format
            valueLineEdit->setValidator(new QRegularExpressionValidator(dateOrNum, this));
            textEditValidation->setText(
                    QStringLiteral("'equal to' -> value format: (date) [YYYY-MM-DD] "
                                   "or (number) [X] \n"
                                   "-> With 'equal to' you can search for tracks with "
                                   "'%1' equal to the exact date you enter in the "
                                   "inputbox or \n"
                                   "-> you can enter a number representing a number "
                                   "of days.\n"
                                   "-> That number will be subtracted from todays' "
                                   "date to define the date to compare with.\n"
                                   "-> The entered 'date' must be in the exact "
                                   "format: 'year: 4 digits - month: 2 digits - day: "
                                   "2 digits 'YYYY-MM-DD'\n"
                                   "-> or a number like 1 or 4785.\n"
                                   "-> Only enter the 'searchtext' (no wildcards)")
                            .arg(conditionsTable[conditionCounter][1]));
        } else if (conditionsTable[conditionCounter][2] == "before") {
            valueLineEdit->setPlaceholderText("e.g. 2016-02-21 or number X");
            //            valueLineEdit->setInputMask("9999-99-99"); // Date: YYYY-MM-DD format
            valueLineEdit->setValidator(new QRegularExpressionValidator(dateOrNum, this));
            textEditValidation->setText(
                    QStringLiteral("'before' -> value format: (date) [YYYY-MM-DD] or "
                                   "(number) [X] \n"
                                   "-> With 'before' you can search for tracks with "
                                   "'%1' before the exact date you enter in the "
                                   "inputbox or \n"
                                   "-> you can enter a number representing a number "
                                   "of days.\n"
                                   "-> That number will be subtracted from todays' "
                                   "date to define the date to compare with.\n"
                                   "-> The entered 'date' must be in the exact "
                                   "format: 'year: 4 digits - month: 2 digits - day: "
                                   "2 digits 'YYYY-MM-DD'\n"
                                   "-> or a number like 1 or 4785.\n"
                                   "-> The entered date or the calculated date "
                                   "(number of days subtracted from todays date) will "
                                   "not be included in the results.\n"
                                   "-> Only enter the 'searchtext' (no wildcards)")
                            .arg(conditionsTable[conditionCounter][1]));
        } else if (conditionsTable[conditionCounter][2] == "after") {
            valueLineEdit->setPlaceholderText("e.g. 2016-02-21 or number X");
            //            valueLineEdit->setInputMask("9999-99-99"); // Date: YYYY-MM-DD format
            valueLineEdit->setValidator(new QRegularExpressionValidator(dateOrNum, this));
            textEditValidation->setText(
                    QStringLiteral("'after' -> value format: (date) [YYYY-MM-DD] or "
                                   "(number) [X] \n"
                                   "-> With 'after' you can search for tracks with "
                                   "'%1' after the exact date you enter in the "
                                   "inputbox or \n"
                                   "-> you can enter a number representing a number "
                                   "of days.\n"
                                   "-> That number will be subtracted from todays' "
                                   "date to define the date to compare with.\n"
                                   "-> The entered 'date' must be in the exact "
                                   "format: 'year: 4 digits - month: 2 digits - day: "
                                   "2 digits 'YYYY-MM-DD'\n"
                                   "-> or a number like 1 or 4785.\n"
                                   "-> The entered date or the calculated date "
                                   "(number of days subtracted from todays date) will "
                                   "not be included in the results.\n"
                                   "-> Only enter the 'searchtext' (no wildcards)")
                            .arg(conditionsTable[conditionCounter][1]));
        } else if (conditionsTable[conditionCounter][2] == "last") {
            valueLineEdit->setPlaceholderText("e.g. number X");
            //            valueLineEdit->setInputMask("9999-99-99"); // Date: YYYY-MM-DD format
            textEditValidation->setText(
                    QStringLiteral("'last' -> value format: (number) [X] \n"
                                   "-> With 'last' you can search for tracks with "
                                   "'%1' in the last X days you enter in the inputbox or \n"
                                   "-> you can enter a number representing a number "
                                   "of days.\n"
                                   "-> That number will be subtracted from todays' "
                                   "date to define the date to compare with.\n"
                                   "-> The entered 'number' must be like 1 or 4785.\n"
                                   "-> The calculated date (number of days subtracted "
                                   "from todays date) will be included in the results.\n"
                                   "-> Only enter the 'searchtext' (no wildcards)")
                            .arg(conditionsTable[conditionCounter][1]));
        } else if (conditionsTable[conditionCounter][2] == "between") {
            valueLineEdit->setPlaceholderText("e.g. 2016-02-21|2022-02-22 or number X|Y : 200|50");
            //            valueLineEdit->setValidator(new
            //            QRegularExpressionValidator(dateOrNum, this));
            //            valueLineEdit->setInputMask("9999-99-99|9999-99-99");
            //            // Date: YYYY-MM-DD format
            textEditValidation->setText(
                    QStringLiteral("'between' -> value format: (date) "
                                   "[YYYY-MM-DD|YYYY-MM-DD] or (Number) [X|Y] \n"
                                   "-> With 'between' you can search for tracks with "
                                   "'%1' between both searchdates you enter in the "
                                   "inputbox or \n"
                                   "-> you can enter numbers representing numbers of "
                                   "days.\n"
                                   "-> Those numbers will be subtracted from todays' "
                                   "date to define the dates to compare with.\n"
                                   "-> The entered 'dates' must be in the exact "
                                   "format: 'year: 4 digits - month: 2 digits - day: "
                                   "2 digits 'YYYY-MM-DD'\n"
                                   "-> or numbers like 1 or 4785 divided by the | "
                                   "symbol.\n"
                                   "-> The entered date or the calculated date "
                                   "(number of days subtracted from todays date) will "
                                   "not be included in the results.\n"
                                   "-> Only enter the 'searchtext' (no wildcards)")
                            .arg(conditionsTable[conditionCounter][1]));
        }
    } else if (numberFieldOptions.contains(
                       conditionsTable[conditionCounter][1])) {
        valueLineEdit->setVisible(true);
        valueComboBox->setVisible(false);
        valueLineEdit->clearMask();
        if (conditionsTable[conditionCounter][2] == "equal to") {
            textEditValidation->setText(
                    QStringLiteral("'equal to' -> value format: (Number) [X] \n"
                                   "-> With 'equal to' you can search for tracks with "
                                   "'%1' equal to the exact number you enter in the "
                                   "inputbox.\n"
                                   "-> Only enter the number (no wildcards)")
                            .arg(conditionsTable[conditionCounter][1]));
            valueLineEdit->setPlaceholderText("e.g. 200 (X = integer)");
            valueLineEdit->setValidator(new QIntValidator(valueLineEdit)); // Integer validation
        } else if (conditionsTable[conditionCounter][2] == "less than") {
            textEditValidation->setText(
                    QStringLiteral("'less than' -> value format: (Number) [X] \n"
                                   "-> With 'less than' you can search for tracks "
                                   "with '%1' less than the exact number you enter in "
                                   "the inputbox.\n"
                                   "-> The entered number will not be included in the "
                                   "results.\n"
                                   "-> Only enter the number (no wildcards)")
                            .arg(conditionsTable[conditionCounter][1]));
            valueLineEdit->setPlaceholderText("e.g. 200 (X = integer)");
            valueLineEdit->setValidator(new QIntValidator(valueLineEdit)); // Integer validation
        } else if (conditionsTable[conditionCounter][2] == "greater than") {
            textEditValidation->setText(
                    QStringLiteral("'greater than' -> value format: (Number) [X] \n"
                                   "-> With 'greater than' you can search for tracks "
                                   "with '%1' greater than the exact number you enter "
                                   "in the inputbox.\n"
                                   "-> The entered number will not be included in the "
                                   "results.\n"
                                   "-> Only enter the number (no wildcards)")
                            .arg(conditionsTable[conditionCounter][1]));
            valueLineEdit->setPlaceholderText("e.g. 200 (X = integer)");
            valueLineEdit->setValidator(new QIntValidator(valueLineEdit)); // Integer validation
        } else if (conditionsTable[conditionCounter][2] == "between") {
            textEditValidation->setText(
                    QStringLiteral("'between' -> value format: (Number) [X|Y] \n"
                                   "-> With 'between' you can search for tracks with "
                                   "'%1' between both searchnumbers you enter in the "
                                   "inputbox.\n"
                                   "-> The numbers can be entered like 1 or 4785 "
                                   "divided by the | symbol.\n"
                                   "-> The entered numbers will not be included in "
                                   "the results.\n"
                                   "-> Only enter the numbers (no wildcards)")
                            .arg(conditionsTable[conditionCounter][1]));
            valueLineEdit->setPlaceholderText("e.g. 200|50 (X|Y = integer)");
        }
        // track in all crates / playlists / historylists
    } else if (trackFieldOptions.contains(
                       conditionsTable[conditionCounter][1])) {
        valueLineEdit->setVisible(false);
        valueComboBox->setVisible(true);
        if (conditionsTable[conditionCounter][2] == "is a member of") {
            textEditValidation->setText(
                    QStringLiteral("'is a member of' -> select the %1 in the dropdown. \n"
                                   "-> With 'is a member of' you can search for tracks that are "
                                   "member of ALL playlists, ALL crates or ALL historylist "
                                   "the type can be selected in the dropdown.\n")
                            .arg(conditionsTable[conditionCounter][1]));
            valueLineEdit->setPlaceholderText("e.g. 200 (X = integer)");
            valueLineEdit->setValidator(new QIntValidator(valueLineEdit)); // Integer validation
        } else if (conditionsTable[conditionCounter][2] == "is not a member of") {
            textEditValidation->setText(QStringLiteral(
                    "'is not a member of' -> select the %1 in the dropdown. \n"
                    "-> With 'is not a member of' you can search for tracks "
                    "that are "
                    "not a member of ALL playlists, ALL crates or ALL "
                    "historylist "
                    "the type can be selected in the dropdown.\n")
                            .arg(conditionsTable[conditionCounter][1]));
            valueLineEdit->setPlaceholderText("e.g. 200 (X = integer)");
            valueLineEdit->setValidator(new QIntValidator(valueLineEdit)); // Integer validation
        }
        if (conditionsTable[conditionCounter][1] == "track") {
            valueLineEdit->setVisible(false);
            valueComboBox->setVisible(true);
            valueComboBox->clear();

            // for (auto it = playlistTable.constBegin(); it != playlistTable.constEnd(); ++it) {
            valueComboBox->addItem("all playlists");
            valueComboBox->addItem("all crates");
            valueComboBox->addItem("all historylists");
            if (sDebug) {
                qDebug() << "[SEARCHCRATES] [EDIT DLG] --> POPULATE "
                            "comboBoxValue -> track -> all p/c/h ";
            }
        }
        valueComboBox->setCurrentText(conditionsTable[conditionCounter][3]);
    } else if (playlistCrateFieldOptions.contains(
                       conditionsTable[conditionCounter][1])) {
        valueLineEdit->setVisible(false);
        valueComboBox->setVisible(true);
        if (conditionsTable[conditionCounter][2] == "is") {
            textEditValidation->setText(
                    QStringLiteral("'is' -> select the %1 in the dropdown. \n"
                                   "-> With 'is' you can search for tracks that are "
                                   "member of the selected %1 you select in the "
                                   "dropdown.\n")
                            .arg(conditionsTable[conditionCounter][1]));
            valueLineEdit->setPlaceholderText("e.g. 200 (X = integer)");
            valueLineEdit->setValidator(new QIntValidator(valueLineEdit)); // Integer validation
        } else if (conditionsTable[conditionCounter][2] == "is not") {
            textEditValidation->setText(
                    QStringLiteral("'is not' -> select the %1 in the dropdown. \n"
                                   "-> With 'is not' you can search for tracks that "
                                   "are NO member of the selected %1 you select in "
                                   "the dropdown.\n")
                            .arg(conditionsTable[conditionCounter][1]));
            valueLineEdit->setPlaceholderText("e.g. 200 (X = integer)");
            valueLineEdit->setValidator(new QIntValidator(valueLineEdit)); // Integer validation
        }
        if (conditionsTable[conditionCounter][1] == "playlist") {
            valueLineEdit->setVisible(false);
            valueComboBox->setVisible(true);
            valueComboBox->clear();

            for (auto it = playlistTable.constBegin(); it != playlistTable.constEnd(); ++it) {
                valueComboBox->addItem(it->second);
                if (sDebug) {
                    qDebug() << "[SEARCHCRATES] [EDIT DLG] --> POPULATE "
                                "comboBoxValue -> playlist -> it->second "
                             << it->second << " it->first "
                             << it->first;
                }
            }
            valueComboBox->setCurrentText(conditionsTable[conditionCounter][3]);
        } else if (conditionsTable[conditionCounter][1] == "history") {
            valueLineEdit->setVisible(false);
            valueComboBox->setVisible(true);
            valueComboBox->clear();
            for (auto it = historyTable.constBegin(); it != historyTable.constEnd(); ++it) {
                valueComboBox->addItem(it->second);
                if (sDebug) {
                    qDebug()
                            << "[SEARCHCRATES] [EDIT DLG] --> POPULATE "
                               "comboBoxValue -> historylist -> it->second "
                            << it->second << " it->first "
                            << it->first;
                }
            }
            valueComboBox->setCurrentText(conditionsTable[conditionCounter][3]);
        } else if (conditionsTable[conditionCounter][1] == "crate") {
            valueLineEdit->setVisible(false);
            valueComboBox->setVisible(true);
            valueComboBox->clear();
            for (auto it = crateTable.constBegin(); it != crateTable.constEnd(); ++it) {
                valueComboBox->addItem(it->second);
                if (sDebug) {
                    qDebug()
                            << "[SEARCHCRATES] [EDIT DLG] --> POPULATE "
                               "comboBoxValue -> cratelist -> it->second "
                            << it->second << " it->first "
                            << it->first;
                }
            }
            valueComboBox->setCurrentText(conditionsTable[conditionCounter][3]);
        }
    }
}

void dlgBaseSearchCrateInfo::onValueComboBoxChanged() {
    // Find the value combo box
    QComboBox* valueComboBox = qobject_cast<QComboBox*>(sender());
    if (!valueComboBox) {
        if (sDebug) {
            qDebug() << "Value ComboBox not found!";
        }
        return;
    }
    int conditionCounter = valueComboBox->objectName()
                                   .replace("comboBoxCondition", "")
                                   .replace("Value", "")
                                   .toInt();
    auto* valueLineEdit = findChild<QLineEdit*>(
            QStringLiteral("lineEditCondition%1Value").arg(conditionCounter));
    conditionsTable[conditionCounter][3] = valueComboBox->currentText();
    if (sDebug) {
        qDebug() << "[onValueComboBoxChanged] valueComboBox " << valueComboBox->currentText();
        qDebug() << "[onValueComboBoxChanged] valueLineEdit " << valueLineEdit->text();
        qDebug() << "[onValueComboBoxChanged] conditionCounter " << conditionCounter;
    }
}

void dlgBaseSearchCrateInfo::storeUIIn2Table() {
    headerTable[0] = lineEditID->text();
    headerTable[1] = lineEditName->text();
    headerTable[2] = QStringLiteral("%1").arg(spinBoxCount->value());
    headerTable[3] = QStringLiteral("%1").arg(checkBoxShow->isChecked());
    headerTable[4] = QStringLiteral("%1").arg(buttonLock->text() == "Unlock");
    headerTable[5] = QStringLiteral("%1").arg(checkBoxAutoDJ->isChecked());
    headerTable[6] = lineEditSearchInput->text();
    headerTable[7] = lineEditSearchSQL->text();

    for (int i = 1; i <= 12; ++i) {
        // check if condition is valid
        auto* fieldComboBox = findChild<QComboBox*>(
                QStringLiteral("comboBoxCondition%1Field").arg(i));
        auto* operatorComboBox = findChild<QComboBox*>(
                QStringLiteral("comboBoxCondition%1Operator").arg(i));
        auto* valueLineEdit = findChild<QLineEdit*>(
                QStringLiteral("lineEditCondition%1Value").arg(i));
        auto* valueComboBox = findChild<QComboBox*>(
                QStringLiteral("comboBoxCondition%1Value").arg(i));
        auto* combinerComboBox = findChild<QComboBox*>(
                QStringLiteral("comboBoxCondition%1Combiner").arg(i));
        if (fieldComboBox) {
            conditionsTable[i][1] = fieldComboBox->currentText();
        }
        if (operatorComboBox) {
            conditionsTable[i][2] = operatorComboBox->currentText();
        }
        if ((valueLineEdit) && (valueComboBox)) {
            if (conditionsTable[i][1] == "playlist") {
                if (playlistNameHash.contains(valueComboBox->currentText())) {
                    QString id = playlistNameHash.value(valueComboBox->currentText());
                    if (sDebug) {
                        qDebug() << "ID for playlist name -> "
                                 << valueComboBox->currentText() << "is:" << id;
                    }
                    conditionsTable[i][3] = QString("%1|||%2").arg(
                            id, valueComboBox->currentText());
                } else {
                    if (sDebug) {
                        qDebug() << "Name not in playlistNameHash: "
                                 << valueComboBox->currentText();
                    }
                }
            } else if (conditionsTable[i][1] == "history") {
                if (historyNameHash.contains(valueComboBox->currentText())) {
                    QString id = historyNameHash.value(valueComboBox->currentText());
                    if (sDebug) {
                        qDebug() << "ID for history name -> "
                                 << valueComboBox->currentText() << "is:" << id;
                    }
                    conditionsTable[i][3] = QString("%1|||%2").arg(
                            id, valueComboBox->currentText());
                } else {
                    if (sDebug) {
                        qDebug() << "Name not in historyNameHash: " << valueComboBox->currentText();
                    }
                }
            } else if (conditionsTable[i][1] == "crate") {
                if (crateNameHash.contains(valueComboBox->currentText())) {
                    QString id = crateNameHash.value(valueComboBox->currentText());
                    if (sDebug) {
                        qDebug() << "ID for crate name -> "
                                 << valueComboBox->currentText() << "is:" << id;
                    }
                    conditionsTable[i][3] = QString("%1|||%2").arg(
                            id, valueComboBox->currentText());
                } else {
                    if (sDebug) {
                        qDebug() << "Name not in crateNameHash: " << valueComboBox->currentText();
                    }
                }
            } else if (conditionsTable[i][1] == "track") {
                // track in all crates / playlists / historylists
                QStringList trackOperatorOptions = {
                        "",
                        "is a member of",
                        "is not a member of"};
                if (trackOperatorOptions.contains(operatorComboBox->currentText())) {
                    conditionsTable[i][3] = valueComboBox->currentText();
                    if (sDebug) {
                        qDebug() << "Track -> "
                                 << operatorComboBox->currentText()
                                 << " "
                                 << valueComboBox->currentText();
                    }
                } else {
                    if (sDebug) {
                        qDebug() << "Track function not found ";
                    }
                }
            } else {
                conditionsTable[i][3] = valueLineEdit->text();
            }
        }
        if (combinerComboBox) {
            conditionsTable[i][4] = combinerComboBox->currentText();
        }
    }
}

// validatity check
bool dlgBaseSearchCrateInfo::validationCheck() {
    storeUIIn2Table();
    textEditValidation->setText(QString(""));
    QStringList stringFieldOptions = {"artist",
            "title",
            "album",
            "album_artist",
            "genre",
            "comment",
            "composer",
            "filetype",
            "key"};
    QStringList dateFieldOptions = {"year", "datetime_added", "last_played_at"};
    QStringList numberFieldOptions = {"duration", "bpm", "played", "timesplayed", "rating"};
    QStringList trackFieldOptions = {"track"};
    QStringList playlistCrateFieldOptions = {"", "playlist", "crate", "history"};
    QStringList stringOperatorOptions = {"contains",
            "does not contain",
            "equal to",
            "not equal to",
            "starts with",
            "ends with",
            "is not empty",
            "is empty"};
    QStringList dateOperatorOptions = {"before", "after", "last", "equal to", "between"};
    QStringList numberOperatorOptions = {
            "less than", "greater than", "equal to", "not equal to", "between"};
    QStringList playlistCrateOperatorOptions = {"", "is", "is not"};
    QStringList trackOperatorOptions = {"", "is a member of", "is not a member of"};

    bool conditionsAreValid = false;
    int endCounter = 0;
    bool endPlacedCorrect = false;
    bool latestCombinerChecked = false;
    bool foundMatchError = false;
    bool checkFieldMatchesOperator = true;
    bool checkMatchBetweenFieldAndOperator = false;
    labelValidation->setText(QStringLiteral("Validation:"));

    for (int i = 12; i >= 1; --i) {
        // check if condition is valid
        // check if field and operator are matching
        if (!foundMatchError) {
            if (stringFieldOptions.contains(conditionsTable[i][1])) {
                if (!stringOperatorOptions.contains(conditionsTable[i][2])) {
                    checkFieldMatchesOperator = false;
                    foundMatchError = true;
                    if (sDebug) {
                        qDebug() << "[SEARCHCRATES] [EDIT DLG] [VALIDATION] --> "
                                    "Matcherror String in condition "
                                 << i;
                    }
                    textEditValidation->setStyleSheet("color: rgb(255,0,0)");
                    textEditValidation->setText(
                            QStringLiteral("Your conditions contain errors: the "
                                           "chosen field and operator don't match in "
                                           "condition %1 \n. SearchCrates is NOT saved.")
                                    .arg(i));
                }
            } else if (dateFieldOptions.contains(conditionsTable[i][1])) {
                if (!dateOperatorOptions.contains(conditionsTable[i][2])) {
                    checkFieldMatchesOperator = false;
                    foundMatchError = true;
                    if (sDebug) {
                        qDebug() << "[SEARCHCRATES] [EDIT DLG] [VALIDATION] --> "
                                    "Matcherror Date in condition "
                                 << i;
                    }
                    textEditValidation->setStyleSheet("color: rgb(255,0,0)");
                    textEditValidation->setText(
                            QStringLiteral("Your conditions contain errors: the "
                                           "chosen field and operator don't match in "
                                           "condition %1 \n. SearchCrates is NOT saved.")
                                    .arg(i));
                }
            } else if (numberFieldOptions.contains(conditionsTable[i][1])) {
                if (!numberOperatorOptions.contains(conditionsTable[i][2])) {
                    checkFieldMatchesOperator = false;
                    foundMatchError = true;
                    if (sDebug) {
                        qDebug() << "[SEARCHCRATES] [EDIT DLG] [VALIDATION] --> "
                                    "Matcherror Number in condition "
                                 << i;
                    }
                    textEditValidation->setStyleSheet("color: rgb(255,0,0)");
                    textEditValidation->setText(
                            QStringLiteral("Your conditions contain errors: the "
                                           "chosen field and operator don't match in "
                                           "condition %1 \n. SearchCrates is NOT saved.")
                                    .arg(i));
                }
                // track in all crates / playlists / historylists
            } else if (trackFieldOptions.contains(conditionsTable[i][1])) {
                if (!trackOperatorOptions.contains(conditionsTable[i][2])) {
                    checkFieldMatchesOperator = false;
                    foundMatchError = true;
                    if (sDebug) {
                        qDebug() << "[SEARCHCRATES] [EDIT DLG] [VALIDATION] --> "
                                    "Matcherror Track in condition "
                                 << i;
                    }
                    textEditValidation->setStyleSheet("color: rgb(255,0,0)");
                    textEditValidation->setText(
                            QStringLiteral("Your conditions contain errors: the "
                                           "chosen field and operator don't match in "
                                           "condition %1 \n. SearchCrates is NOT saved.")
                                    .arg(i));
                }
            } else if (playlistCrateFieldOptions.contains(conditionsTable[i][1])) {
                if (!playlistCrateOperatorOptions.contains(conditionsTable[i][2])) {
                    checkFieldMatchesOperator = false;
                    foundMatchError = true;
                    if (sDebug) {
                        qDebug() << "[SEARCHCRATES] [EDIT DLG] [VALIDATION] --> "
                                    "Matcherror Number in condition "
                                 << i;
                    }
                    textEditValidation->setStyleSheet("color: rgb(255,0,0)");
                    textEditValidation->setText(
                            QStringLiteral("Your conditions contain errors: the "
                                           "chosen field and operator don't match in "
                                           "condition %1 \n. SearchCrates is NOT saved.")
                                    .arg(i));
                }
            }
            // checks if operators match input values
            if (conditionsTable[i][2] == "between") {
                if (!conditionsTable[i][3].contains("|")) {
                    if (sDebug) {
                        qDebug() << "[SEARCHCRATES] [EDIT DLG] [VALIDATION] --> "
                                    "Matcherror between Operator and input value in condition "
                                 << i << "no '|' in 'between'-condition.";
                    }
                    textEditValidation->setStyleSheet("color: rgb(255,0,0)");
                    textEditValidation->setText(
                            QStringLiteral("Your conditions contain errors: the "
                                           "chosen operator and input don't match in "
                                           "condition %1 \n."
                                           "When you use the 'between'-operator, "
                                           "the input values need to be separated by a '|'. \n"
                                           "e.g. 2|107 \n"
                                           "SearchCrates is NOT saved.")
                                    .arg(i));
                    checkFieldMatchesOperator = false;
                    // checkOperatorMatchesValue = false;
                    foundMatchError = true;
                } else if ((conditionsTable[i][3].length() < 3) ||
                        (conditionsTable[i][3].indexOf("|", 0) < 1) ||
                        (conditionsTable[i][3].indexOf("|", 0) ==
                                conditionsTable[i][3].length() - 1)) {
                    if (sDebug) {
                        qDebug() << "[SEARCHCRATES] [EDIT DLG] [VALIDATION] --> "
                                    "Matcherror between Operator and input value in condition "
                                 << i << "missing value in 'between'-condition.";
                    }
                    textEditValidation->setStyleSheet("color: rgb(255,0,0)");
                    textEditValidation->setText(
                            QStringLiteral("Your conditions contain errors: the "
                                           "chosen operator and input don't match in "
                                           "condition %1.\n"
                                           "When you use the 'between'-operator, "
                                           "the input values need to be numbers or "
                                           "dates separated by a '|'. \n"
                                           "e.g. 2|107 or 1988-02-29|2015-12-02 \n"
                                           "SearchCrates is NOT saved.")
                                    .arg(i));
                    checkFieldMatchesOperator = false;
                    foundMatchError = true;
                } else if (conditionsTable[i][3].indexOf("|", 0) > 0) {
                    if (conditionsTable[i][3].contains("-")) {
                        if (sDebug) {
                            qDebug() << "pos | " << conditionsTable[i][3].indexOf("|", 0);
                            qDebug() << "length " << conditionsTable[i][3].length();
                        }
                        if ((conditionsTable[i][3].indexOf("|", 0) == 10) &&
                                (conditionsTable[i][3].length() == 21)) {
                            std::chrono::year_month_day fromDate(
                                    std::chrono::year(conditionsTable[i][3].mid(0, 4).toInt()),
                                    std::chrono::month(conditionsTable[i][3].mid(5, 2).toInt()),
                                    std::chrono::day(conditionsTable[i][3].mid(8, 2).toInt()));
                            QDate From(fromDate.year().operator int(),
                                    unsigned(fromDate.month()),
                                    unsigned(fromDate.day()));
                            std::chrono::year_month_day toDate(
                                    std::chrono::year(conditionsTable[i][3].mid(11, 4).toInt()),
                                    std::chrono::month(conditionsTable[i][3].mid(16, 2).toInt()),
                                    std::chrono::day(conditionsTable[i][3].mid(19, 2).toInt()));
                            QDate To(toDate.year().operator int(),
                                    unsigned(toDate.month()),
                                    unsigned(toDate.day()));

                            bool FromValid = From.isValid();
                            bool ToValid = To.isValid();
                            if (sDebug) {
                                qDebug() << "[SEARCHCRATES] [EDIT DLG] "
                                            "[VALIDATION] --> From: "
                                         << From << " Valid = " << FromValid;
                                qDebug() << "[SEARCHCRATES] [EDIT DLG] "
                                            "[VALIDATION] --> To: "
                                         << To << " Valid = " << ToValid;
                            }
                            if ((!FromValid) || (!ToValid)) {
                                // one / both of the dates are INvalid
                                if (sDebug) {
                                    qDebug() << "[SEARCHCRATES] [EDIT DLG] "
                                                "[VALIDATION] --> "
                                             << "Matcherror between Operator "
                                                "and input value in condition "
                                             << i << "dates are not valid.";
                                }
                                textEditValidation->setStyleSheet("color: rgb(255,0,0)");
                                textEditValidation->setText(QStringLiteral(
                                        "Your conditions contain errors: the "
                                        "chosen operator and input don't match "
                                        "in condition %1.\n"
                                        "At least one of the dates you entered "
                                        "is invalid.\n"
                                        "When you use the 'between'-operator, "
                                        "the input values need to be numbers "
                                        "or dates separated by a '|'. \n"
                                        "e.g. 2|107 or 1988-02-29|2015-12-02 \n"
                                        "SearchCrates is NOT saved.")
                                                .arg(i));
                                checkFieldMatchesOperator = false;
                                foundMatchError = true;
                            }
                        } else {
                            // one of the dates is not correct mask YYYY-MM-DD
                            if (sDebug) {
                                qDebug() << "[SEARCHCRATES] [EDIT DLG] "
                                            "[VALIDATION] --> "
                                         << "Matcherror between Operator and "
                                            "input value in condition "
                                         << i
                                         << "dates are not entered in correct "
                                            "format.";
                            }
                            textEditValidation->setStyleSheet("color: rgb(255,0,0)");
                            textEditValidation->setText(QStringLiteral(
                                    "Your conditions contain errors: the "
                                    "chosen operator and input don't match in "
                                    "condition %1.\n"
                                    "When you use the 'between'-operator, "
                                    "the input values need to be numbers or "
                                    "dates separated by a '|'. \n"
                                    "e.g. 2|107 or 1988-02-29|2015-12-02 \n"
                                    "SearchCrates is NOT saved.")
                                            .arg(i));
                            checkFieldMatchesOperator = false;
                            foundMatchError = true;
                        }

                    } else {
                        int posBar = conditionsTable[i][3].indexOf("|", 0);
                        QString From = conditionsTable[i][3].mid(0, posBar);
                        QString To = conditionsTable[i][3].mid(posBar + 1,
                                conditionsTable[i][3].length() - posBar + 1);
                        if ((From.toInt() > 36500) || (To.toInt() > 36500)) {
                            if (sDebug) {
                                qDebug() << "[SEARCHCRATES] [EDIT DLG] "
                                            "[VALIDATION] --> "
                                            "Matcherror between Operator and "
                                            "input value in condition "
                                         << i
                                         << "values are > 100 years in "
                                            "'between'-condition.";
                            }
                            textEditValidation->setStyleSheet("color: rgb(255,0,0)");
                            textEditValidation->setText(QStringLiteral(
                                    "Your conditions contain errors: the "
                                    "chosen operator and input don't match in "
                                    "condition %1.\n  "
                                    "The value in the input represents more "
                                    "than 100 years. \n"
                                    "When you use the 'between'-operator, the "
                                    "input values need to be numbers or dates "
                                    "separated by a '|'. \n"
                                    "e.g. 2|107 or 1988-02-29|2015-12-02 \n"
                                    "SearchCrates is NOT saved.")
                                            .arg(i));
                            checkFieldMatchesOperator = false;
                            foundMatchError = true;
                        } else if ((From.toInt() / From.toInt() != 1) ||
                                (To.toInt() / To.toInt() != 1)) {
                            if (sDebug) {
                                qDebug() << "[SEARCHCRATES] [EDIT DLG] "
                                            "[VALIDATION] --> "
                                            "Matcherror between Operator and "
                                            "input value in condition "
                                         << i
                                         << "values are not numbers in "
                                            "'between'-condition.";
                            }
                            textEditValidation->setStyleSheet("color: rgb(255,0,0)");
                            textEditValidation->setText(QStringLiteral(
                                    "Your conditions contain errors: the "
                                    "chosen operator and input don't match in "
                                    "condition %1. \n"
                                    "When you use the 'between'-operator, "
                                    "the input values need to be numbers or "
                                    "dates separated by a '|'. \n"
                                    "e.g. 2|107 or 1988-02-29|2015-12-02 \n"
                                    "SearchCrates is NOT saved.")
                                            .arg(i));
                            checkFieldMatchesOperator = false;
                            foundMatchError = true;
                        }
                    }
                }
            }
        }

        // check if latest condition has end-combiner
        if (latestCombinerChecked == false) {
            if (conditionsTable[i][1] != "") {
                if (conditionsTable[i][4] == ") END") {
                    endPlacedCorrect = true;
                }
                latestCombinerChecked = true;
            }
        }
        // count end-combiners
        if (conditionsTable[i][4] == ") END") {
            endCounter = endCounter + 1;
        }
    }

    // no conditions created -> std conditions
    if (conditionsTable[1][1] == "") {
        endPlacedCorrect = true;
    }

    // if end-combiner is in last condition
    if (endPlacedCorrect) {
        // check end-combiner occurrences
        if (endCounter < 1) {
            if ((conditionsTable[1][1] != "")) {
                textEditValidation->setStyleSheet("color: rgb(255,0,0)");
                textEditValidation->setText(QStringLiteral(
                        "Your conditions contain errors: You didn't place an ') "
                        "END' in the last combiner. SearchCrates is NOT saved."));
                conditionsAreValid = false;
            } else {
                // no conditions created -> std conditions
                checkMatchBetweenFieldAndOperator = true;
            }
        } else if (endCounter == 1) {
            checkMatchBetweenFieldAndOperator = true;
        } else if (endCounter > 1) {
            textEditValidation->setStyleSheet("color: rgb(255,0,0)");
            textEditValidation->setText(
                    QStringLiteral("Your conditions contain errors: ') END' can only "
                                   "be used at the end of your conditions, you have "
                                   "%1 occurrences. SearchCrates is NOT saved.")
                            .arg(endCounter));
            conditionsAreValid = false;
        }
    } else {
        textEditValidation->setStyleSheet("color: rgb(255,0,0)");
        textEditValidation->setText(QStringLiteral(
                "Your conditions contain errors: You didn't place an ') END' "
                "in the last combiner. SearchCrates is NOT saved."));
        conditionsAreValid = false;
    }
    //
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [EDIT DLG] [VALIDATION] --> "
                    "checkMatchBetweenFieldAndOperator "
                 << checkMatchBetweenFieldAndOperator;
    }
    if (checkMatchBetweenFieldAndOperator) {
        if (sDebug) {
            qDebug() << "[SEARCHCRATES] [EDIT DLG] [VALIDATION] --> "
                        "checkMatchBetweenFieldAndOperator "
                     << checkMatchBetweenFieldAndOperator
                     << " checkFieldMatchesOperator "
                     << checkFieldMatchesOperator << " foundMatchError ";
        }
        if (foundMatchError) {
            if (sDebug) {
                qDebug() << "[SEARCHCRATES] [EDIT DLG] [VALIDATION] --> ERROR -> NO SAVE ";
            }
            conditionsAreValid = false;
        } else {
            if (sDebug) {
                qDebug() << "[SEARCHCRATES] [EDIT DLG] [VALIDATION] --> OK -> SAVE ";
            }
            textEditValidation->setStyleSheet("color: rgb(155,0,0)");
            textEditValidation->setStyleSheet("background-color: rgb(0,155,0)");
            textEditValidation->setText(QStringLiteral(
                    "Your conditions are valid, the searchCrates will be saved"));
            conditionsAreValid = true;
        }
    }
    return conditionsAreValid;
}

// Changes the lock-button states in the UI
void dlgBaseSearchCrateInfo::toggleLockStatus() {
    if (buttonLock->text() == "Lock") {
        buttonLock->setText("Unlock");
    } else {
        buttonLock->setText("Lock");
    }
}

QVariantList dlgBaseSearchCrateInfo::collectUIChanges() const {
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> CollectUIChanges Started!";
    }
    QVariantList updatedData;
    updatedData.append(lineEditID->text());
    updatedData.append(headerTable[1]);
    updatedData.append(spinBoxCount->value());
    updatedData.append(checkBoxShow->isChecked());
    updatedData.append(buttonLock->text() == "Unlock");
    updatedData.append(checkBoxAutoDJ->isChecked());
    updatedData.append(lineEditSearchInput->text());
    updatedData.append(lineEditSearchSQL->text());

    for (int i = 1; i <= 12; ++i) {
        updatedData.append(conditionsTable[i][1]);
        updatedData.append(conditionsTable[i][2]);
        updatedData.append(conditionsTable[i][3]);
        updatedData.append(conditionsTable[i][4]);
        //        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> Collecting Condition " << i << ":"
        //                 << "Field:" << conditionsTable[i][1]
        //                 << "Operator:" << conditionsTable[i][2]
        //                 << "Value:" << conditionsTable[i][3]
        //                 << "Combiner:" << conditionsTable[i][4];
    }

    // add prev/next pointers
    for (int i = 0; i < 4; ++i) {
        updatedData.append(prevnextTable[i]);
    }

    //    if (sDebug) {
    qDebug() << "[SEARCHCRATES] [EDIT DLG] --> CollectUIChanges Finished!";
    qDebug() << "[SEARCHCRATES] [EDIT DLG] --> Collected data:" << updatedData;
    //    }
    return updatedData;
}

void dlgBaseSearchCrateInfo::onApplyButtonClicked() {
    textEditValidation->setText(QStringLiteral(" "));
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> Apply button clicked!";
    }
    storeUIIn2Table();
    QVariantList editedData = collectUIChanges();
    if (validationCheck()) {
        if (sDebug) {
            qDebug() << "[SEARCHCRATES] [EDIT DLG] --> Validation check " << validationCheck();
            qDebug() << "[SEARCHCRATES] [EDIT DLG] --> Validation check OK";
            qDebug() << "[SEARCHCRATES] [EDIT DLG] --> Data collected for Apply:" << editedData;
        }
        emit dataUpdated(editedData); // Emit signal with updated data if needed
        if (sDebug) {
            qDebug() << "[SEARCHCRATES] [EDIT DLG] --> SIGNAL SND -> Data applied "
                        "without closing the dialog";
        }
        textEditValidation->setStyleSheet("color: rgb(155,0,0)");
        //            textEditValidation->setStyleSheet("textcolor: rgb(255,255,255)");
        textEditValidation->setStyleSheet("background-color: rgb(0,155,0)");
        textEditValidation->setText(QStringLiteral(
                "Validation check OK, The searchCrates is saved"));
        headerTable[7] = buildWhereClause();
        populateUI();
        //    accept();
    } else {
        if (sDebug) {
            qDebug() << "[SEARCHCRATES] [EDIT DLG] --> Validation check NIET OK";
        }
        populateUI();
        return;
    }
}

void dlgBaseSearchCrateInfo::onNewButtonClicked() {
    if (!m_isUpdatingUI) { // Only emit if not in update mode
        if (sDebug) {
            qDebug() << "[SEARCHCRATES] [EDIT DLG] --> SIGNAL SND -> New button "
                        "clicked, emitted requestNewwSearchCrates signal, current "
                        "searchCratesID="
                     << headerTable[0];
        }
        emit requestNewSearchCrate();
        m_isUpdatingUI = true;
    } else {
        if (sDebug) {
            qDebug() << "[SEARCHCRATES] [EDIT DLG] --> SIGNAL SND -> New button "
                        "clicked, loop signal blocked, current searchCratesID="
                     << headerTable[0];
        }
        const QSignalBlocker signalBlocker(this);
        m_isUpdatingUI = false;
    }
}

void dlgBaseSearchCrateInfo::onDeleteButtonClicked() {
    if (!m_isUpdatingUI) { // Only emit if not in update mode
        if (sDebug) {
            qDebug() << "[SEARCHCRATES] [EDIT DLG] --> SIGNAL SND -> Delete button "
                        "clicked, emitted requestDeleteSearchCrates signal, "
                        "current searchCratesID="
                     << headerTable[0];
        }
        emit requestDeleteSearchCrate();
        m_isUpdatingUI = true;
    } else {
        if (sDebug) {
            qDebug() << "[SEARCHCRATES] [EDIT DLG] --> SIGNAL SND -> Delete button "
                        "clicked, loop signal blocked, current searchCratesID="
                     << headerTable[0];
        }
        const QSignalBlocker signalBlocker(this);
        m_isUpdatingUI = false;
    }
}

void dlgBaseSearchCrateInfo::onPreviousButtonClicked() {
    if (!m_isUpdatingUI) { // Only emit if not in update mode
        if (sDebug) {
            qDebug() << "[SEARCHCRATES] [EDIT DLG] --> SIGNAL SND -> Previous "
                        "button clicked, emitted requestPreviousSearchCrates "
                        "signal, current searchCratesID="
                     << headerTable[0];
        }
        emit requestPreviousSearchCrate();
        m_isUpdatingUI = true;
    } else {
        if (sDebug) {
            qDebug() << "[SEARCHCRATES] [EDIT DLG] --> SIGNAL SND -> Previous "
                        "button clicked, loop signal blocked, current "
                        "searchCratesID="
                     << headerTable[0];
        }
        const QSignalBlocker signalBlocker(this);
        m_isUpdatingUI = false;
    }
}

void dlgBaseSearchCrateInfo::onNextButtonClicked() {
    if (!m_isUpdatingUI) { // Only emit if not in update mode
        if (sDebug) {
            qDebug() << "[SEARCHCRATES] [EDIT DLG] --> SIGNAL SND -> Next button "
                        "clicked, emitted requestNextSearchCrates signal, current "
                        "searchCratesID="
                     << headerTable[0];
        }
        emit requestNextSearchCrate();
        m_isUpdatingUI = true;
    } else {
        const QSignalBlocker signalBlocker(this);
        if (sDebug) {
            qDebug() << "[SEARCHCRATES] [EDIT DLG] --> SIGNAL SND -> Next button "
                        "clicked, loop signal blocked, current searchCratesID="
                     << headerTable[0];
        }
        m_isUpdatingUI = false;
    }
}

void dlgBaseSearchCrateInfo::onOKButtonClicked() {
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> OK button clicked!";
    }
    QVariantList editedData = collectUIChanges();

    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> Data collected for Apply:" << editedData;
    }
    emit dataUpdated(editedData); // Emit signal with updated data if needed

    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> SIGNAL SND -> Data saved and dialog closed";
    }
    accept();
}

void dlgBaseSearchCrateInfo::onCancelButtonClicked() {
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> Cancel button clicked!";
    }
    accept();
}

// begin signal grey out / show next condition
// check initial state on startup -> call updateConditionState once
void dlgBaseSearchCrateInfo::updateConditionState() {
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> updateConditionState started / "
                    "enable/disable condition fields";
    }
    // check -> enable the next field?
    bool enableNextField = true;

    for (int i = 1; i <= 12; ++i) {
        auto* fieldComboBox = findChild<QComboBox*>(
                QStringLiteral("comboBoxCondition%1Field").arg(i));
        auto* operatorComboBox = findChild<QComboBox*>(
                QStringLiteral("comboBoxCondition%1Operator").arg(i));
        auto* valueLineEdit = findChild<QLineEdit*>(
                QStringLiteral("lineEditCondition%1Value").arg(i));
        auto* valueComboBox = findChild<QComboBox*>(
                QStringLiteral("comboBoxCondition%1Value").arg(i));
        auto* combinerComboBox = findChild<QComboBox*>(
                QStringLiteral("comboBoxCondition%1Combiner").arg(i));
        auto* insertButton = findChild<QPushButton*>(
                QStringLiteral("buttoncondition%1_insert").arg(i));
        auto* moveUpButton = findChild<QPushButton*>(QStringLiteral("buttoncondition%1_up").arg(i));
        auto* moveDownButton = findChild<QPushButton*>(
                QStringLiteral("buttoncondition%1_down").arg(i));
        auto* deleteButton = findChild<QPushButton*>(
                QStringLiteral("buttoncondition%1_delete").arg(i));

        // enable next field only if previous is selected -> narrowing operator
        // + check to have everything filled in
        if (fieldComboBox && operatorComboBox && valueLineEdit &&
                valueComboBox && combinerComboBox) {
            bool fieldSelected = !fieldComboBox->currentText().isEmpty();
            bool operatorSelected = !operatorComboBox->currentText().isEmpty();
            bool valuePresent = !valueLineEdit->text().isEmpty();
            bool valueSelected = !valueComboBox->currentText().isEmpty();
            bool combinerSelected = !combinerComboBox->currentText().isEmpty();

            // fieldComboBox enabled if line above is complete or if loaded value from db
            fieldComboBox->setEnabled(fieldSelected || enableNextField);

            // enable operatorcombobox if field selected or if loaded value from db
            operatorComboBox->setEnabled(fieldSelected || operatorSelected);

            // enabling valueinput + combiner
            // special cases: crate/playlist -> activate valuecombobox
            // special cases: is empty / is not empty -> no valuelineedit or valuecombobox
            if (operatorSelected) {
                if ((fieldComboBox->currentText() == "playlist") ||
                        (fieldComboBox->currentText() == "crate") ||
                        (fieldComboBox->currentText() == "history") ||
                        (fieldComboBox->currentText() == "track")) {
                    valueComboBox->setEnabled(operatorSelected || valueSelected);
                    combinerComboBox->setEnabled(operatorSelected || combinerSelected);
                } else if ((operatorComboBox->currentText() == "is empty") ||
                        (operatorComboBox->currentText() == "is not empty")) {
                    combinerComboBox->setEnabled((operatorSelected) || combinerSelected);
                } else {
                    combinerComboBox->setEnabled((valuePresent) || combinerSelected);
                    valueLineEdit->setEnabled(operatorSelected || valuePresent);
                }
            } else {
                valueLineEdit->setEnabled(valuePresent);
                combinerComboBox->setEnabled(valuePresent || combinerSelected);
            }

            // enable condition insert / delete / move up / move down
            // if no conditions (field 1 empty), no condition can be deleted,
            // check if #1 has selected field
            auto* fieldComboBox1 = findChild<QComboBox*>(QStringLiteral("comboBoxCondition1Field"));
            if (fieldComboBox1->currentText().isEmpty()) {
                deleteButton->setEnabled(false);
            } else {
                deleteButton->setEnabled(fieldSelected && operatorSelected && combinerSelected);
            }
            // if already 12 conditions, no extra condition can be inserted,
            // check if #12 has selected field
            auto* fieldComboBox12 = findChild<QComboBox*>(
                    QStringLiteral("comboBoxCondition12Field"));
            if (fieldComboBox12->currentText().isEmpty()) {
                insertButton->setEnabled(fieldSelected && operatorSelected && combinerSelected);
            } else {
                insertButton->setEnabled(false);
            }
            moveUpButton->setEnabled(fieldSelected && operatorSelected && combinerSelected);
            moveDownButton->setEnabled(fieldSelected && operatorSelected && combinerSelected);
            deleteButton->setEnabled(fieldSelected && operatorSelected && combinerSelected);
            // Only enable the next field line if this line has values or has been started
            enableNextField = fieldSelected;
        }
    }
    for (int i = 12; i >= 1; --i) {
        if (sDebug) {
            qDebug() << "[SEARCHCRATES] [EDIT DLG] --> updateConditionState "
                        "enable/disable insert/delete/move up/move down";
        }
        auto* fieldComboBox = findChild<QComboBox*>(
                QStringLiteral("comboBoxCondition%1Field").arg(i));

        if (fieldComboBox->currentText().isEmpty()) {
            auto* insertButton = findChild<QPushButton*>(
                    QStringLiteral("buttoncondition%1_insert").arg(i));
            auto* moveUpButton = findChild<QPushButton*>(
                    QStringLiteral("buttoncondition%1_up").arg(i));
            auto* moveDownButton = findChild<QPushButton*>(
                    QStringLiteral("buttoncondition%1_down").arg(i));
            auto* deleteButton = findChild<QPushButton*>(
                    QStringLiteral("buttoncondition%1_delete").arg(i));

            insertButton->setEnabled(false);
            moveUpButton->setEnabled(false);
            moveDownButton->setEnabled(false);
            deleteButton->setEnabled(false);
        } else {
            // return;
        }
    }
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> updateConditionState finished";
    }
}
//  end signal grey out / show next condition

// conditions: insert / delete / up / down
void dlgBaseSearchCrateInfo::insertCondition(int index) {
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> Insert button clicked! on condition #" << index;
    }
    if (index < 1 || index > 11) {
        return;
    }
    //    for (int i = 1; i <= 12; ++i) {
    //        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> BEFORE INSERT COPY " << i << ":"
    //                 << "Field:" << conditionsTable[i][1]
    //                 << "Operator:" << conditionsTable[i][2]
    //                 << "Value:" << conditionsTable[i][3]
    //                 << "Combiner:" << conditionsTable[i][4];
    //    }

    // Shift all conditions after this index down
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> Insert button clicked on "
                    "condition #"
                 << index << " copy from 11 to " << index;
    }
    for (int i = 11; i >= index; --i) {
        copyCondition(i, i + 1);
    }
    //    for (int i = 1; i <= 12; ++i) {
    //        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> AFTER INSERT COPY
    //        BEFORE CLEAR " << i << ":"
    //                 << "Field:" << conditionsTable[i][1]
    //                 << "Operator:" << conditionsTable[i][2]
    //                 << "Value:" << conditionsTable[i][3]
    //                 << "Combiner:" << conditionsTable[i][4];
    //    }

    // Insert an empty condition at the selected index
    clearCondition(index);
    populateUI();
}

void dlgBaseSearchCrateInfo::removeCondition(int index) {
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> Remove button clicked! on condition #" << index;
    }
    if (index < 1 || index > 12) {
        return;
    }

    // Shift all conditions after this index up
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> Remove condition #" << index
                 << " copy from " << index << " < 12";
    }
    for (int i = index; i < 12; ++i) {
        copyCondition(i + 1, i);
    }
    // Clear the last condition
    clearCondition(12);
    populateUI();
}

void dlgBaseSearchCrateInfo::swapConditionWithAbove(int index) {
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> Swap with above button clicked! "
                    "on condition #"
                 << index;
    }
    if (index < 2 || index > 12) {
        return;
    }
    // Swap the current condition with the one above
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> Swap with above condition # : "
                 << index << " with " << index - 1;
    }
    swapConditions(index, index - 1);
    populateUI();
}

void dlgBaseSearchCrateInfo::swapConditionWithBelow(int index) {
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> Swap with below button clicked! "
                    "on condition #"
                 << index;
    }
    if (index < 1 || index > 11) {
        return;
    }
    // Swap the current condition with the one below
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> Swap with below condition # : "
                 << index << " with " << index + 1;
    }
    swapConditions(index, index + 1);
    populateUI();
}

void dlgBaseSearchCrateInfo::copyCondition(int fromIndex, int toIndex) {
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> copy condition from "
                 << fromIndex << " to " << toIndex;
    }
    if (fromIndex < 1 || fromIndex > 12 || toIndex < 1 || toIndex > 12) {
        return;
    }
    // Copy the condition from fromIndex to toIndex
    for (int j = 1; j <= 4; ++j) {
        conditionsTable[toIndex][j] = conditionsTable[fromIndex][j];
    }
}

void dlgBaseSearchCrateInfo::clearCondition(int index) {
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> clear condition #" << index;
    }
    // Clear the condition on the index row
    for (int j = 1; j <= 4; ++j) {
        conditionsTable[index][j] = "";
    }
}

void dlgBaseSearchCrateInfo::swapConditions(int index1, int index2) {
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [EDIT DLG] --> swap conditions " << index1 << " and " << index2;
    }
    // Swap the condition between rows index1 & index2 -> field
    QString tempField = conditionsTable[index1][1];
    conditionsTable[index1][1] = conditionsTable[index2][1];
    conditionsTable[index2][1] = tempField;

    // Swap the condition between rows index1 & index2 -> operator
    QString tempOperator = conditionsTable[index1][2];
    conditionsTable[index1][2] = conditionsTable[index2][2];
    conditionsTable[index2][2] = tempOperator;

    // Swap the condition between rows index1 & index2 -> value
    QString tempValue = conditionsTable[index1][3];
    conditionsTable[index1][3] = conditionsTable[index2][3];
    conditionsTable[index2][3] = tempValue;

    // Swap the condition between rows index1 & index2 -> combiner
    QString tempCombiner = conditionsTable[index1][4];
    conditionsTable[index1][4] = conditionsTable[index2][4];
    conditionsTable[index2][4] = tempCombiner;
}

QString dlgBaseSearchCrateInfo::buildWhereClause() {
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [EDIT DLG] [GETWHERECLAUSEFORSEARCHCRATES] "
                    "[CONSTRUCT SQL] --> start constructing buildWhereClause";
    }
    QString whereClause = "(";
    bool hasConditions = false;
    QStringList combinerOptions = {") END", "AND", "OR", ") AND (", ") OR ("};
    for (int i = 1; i <= 12; ++i) {
        const QString& field = conditionsTable[i][1];
        const QString& op = conditionsTable[i][2];
        const QString& value = conditionsTable[i][3];

        //  begin build condition
        //  function moved to searchCratesfunctions.h to share it with
        //  dlgsearchCratesinfo to create preview
        const QString& condition = buildCondition(field, op, value);

        //  end build condition
        if (condition != "") {
            hasConditions = true;
            whereClause += condition;
            // Add combiner if not the last condition
            // Adding combiner + spaces around the combiner
            if (i < 12 && combinerOptions.contains(conditionsTable[i][4])) {
                if (conditionsTable[i][4] == ") END") {
                    whereClause += ")";
                } else if ((conditionsTable[i][4] == "AND") || (conditionsTable[i][4] == "OR")) {
                    whereClause += " " + conditionsTable[i][4] + " ";
                } else {
                    whereClause += conditionsTable[i][4] + " ";
                }
            }
        }
    }

    if (!hasConditions) {
        whereClause += QStringLiteral(
                "library.artist LIKE '%%1%' OR "
                "library.title LIKE '%%1%' OR "
                "library.album LIKE '%%1%' OR "
                "library.album_artist LIKE '%%1%' OR "
                "library.composer LIKE '%%1%' OR "
                "library.genre LIKE '%%1%' OR "
                "library.comment LIKE '%%1%')")
                               .arg(headerTable[6]);
    }

    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [EDIT DLG] [GETWHERECLAUSEFORSEARCHCRATES] "
                    "[CONSTRUCT SQL] -> Constructed WHERE clause:"
                 << whereClause;
    }
    return whereClause;
}
