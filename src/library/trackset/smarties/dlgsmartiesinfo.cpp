
#include "library/trackset/smarties/dlgsmartiesinfo.h"
// #include "library/trackset/smarties/ui_dlgsmartiesinfo.h"
#include <QDebug>

#include "library/trackset/smarties/smartiesfeature.h"
#include "moc_dlgsmartiesinfo.cpp"

dlgSmartiesInfo::dlgSmartiesInfo(
        //        QWidget* parent,
        SmartiesFeature* feature,
        QWidget* parent)
        : QDialog(parent),
          m_feature(feature),
          m_isUpdatingUI(false) {
    setupUi(this);

    connect(m_feature,
            &SmartiesFeature::updateSmartiesData,
            this,
            &dlgSmartiesInfo::onUpdateSmartiesData);

    // Initialize the lock-button states on UI load
    connect(buttonLock,
            &QPushButton::clicked,
            this,
            &dlgSmartiesInfo::toggleLockStatus);

    initializeConditionState(); // Initialize the condition states on UI load

    // Connect signals to dynamically adjust condition state when fields change
    for (int i = 0; i < 12; ++i) {
        auto* fieldComboBox = findChild<QComboBox*>(QString("comboBoxCondition%1Field").arg(i + 1));
        auto* operatorComboBox = findChild<QComboBox*>(
                QString("comboBoxCondition%1Operator").arg(i + 1));
        auto* valueLineEdit = findChild<QLineEdit*>(QString("lineEditCondition%1Value").arg(i + 1));
        auto* combinerComboBox = findChild<QComboBox*>(
                QString("comboBoxCondition%1Combiner").arg(i + 1));

        if (fieldComboBox && operatorComboBox && valueLineEdit && combinerComboBox) {
            connect(fieldComboBox,
                    &QComboBox::currentTextChanged,
                    this,
                    &dlgSmartiesInfo::updateConditionState);
            connect(operatorComboBox,
                    &QComboBox::currentTextChanged,
                    this,
                    &dlgSmartiesInfo::updateConditionState);
            connect(valueLineEdit,
                    &QLineEdit::textChanged,
                    this,
                    &dlgSmartiesInfo::updateConditionState);
            connect(combinerComboBox,
                    &QComboBox::currentTextChanged,
                    this,
                    &dlgSmartiesInfo::updateConditionState);
        }
    }
}

void dlgSmartiesInfo::init(const QVariantList& smartiesData) {
    qDebug() << "[SMARTIES] [EDIT DLG] ---> Initializing with data:" << smartiesData;
    populateUI(smartiesData);
}

void dlgSmartiesInfo::onUpdateSmartiesData(const QVariantList& smartiesData) {
    m_isUpdatingUI = true; // Set the flag to indicate UI is being updated
    qDebug() << "[SMARTIES] [EDIT DLG] ---> SIGNAL RCVD Received Signal from "
                "feature -> Initializing with data:"
             << smartiesData;
    populateUI(smartiesData);
}

QVariant dlgSmartiesInfo::getUpdatedData() const {
    // Collect and return the updated data
    return collectUIChanges();
    //    return 1;
}

void dlgSmartiesInfo::populateUI(const QVariantList& smartiesData) {
    m_isUpdatingUI = true; // Set update flag to prevent emitting signals during population

    if (smartiesData.isEmpty()) {
        return; // No data found for this ID
    }

    if (!smartiesData.isEmpty()) {
        lineEditID->setText(smartiesData[0].toString());
        lineEditID->setReadOnly(true);
        lineEditName->setText(smartiesData[1].toString());
        spinBoxCount->setValue(smartiesData[2].toInt());
        checkBoxShow->setChecked(smartiesData[3].toBool());
        buttonLock->setText(smartiesData[4].toBool() ? "Unlock" : "Lock");
        checkBoxAutoDJ->setChecked(smartiesData[5].toBool());
        lineEditSearchInput->setText(smartiesData[6].toString());
        lineEditSearchSQL->setReadOnly(true);
        lineEditSearchSQL->setText(smartiesData[7].toString());
        lineEditSearchSQL->setToolTip(
                tr("This field can be seen as 'information', it is the result "
                   "of the conditions in this window.") +
                "\n" +
                tr("This field gets calculated when you press Apply / ok, you "
                   "can have an idea of the result.") +
                "\n");
        textEditSearchSQL->setText(smartiesData[7].toString());
        textEditSearchSQL->setToolTip(
                tr("This field (the same as the one above can be seen as "
                   "'information', it is the result of the conditions in this "
                   "window.") +
                "\n" +
                tr("This view exists only to let you see the vomplete sql "
                   "statement in a scrollable form.") +
                "\n");

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
                "rating"};
        QStringList stringFieldOptions = {"",
                "artist",
                "title",
                "album",
                "album_artist",
                "genre",
                "comment",
                "composer",
                "filetype",
                "key"};
        QStringList dateFieldOptions = {"",
                "year",
                "datetime_added",
                "last_played_at"};
        QStringList numberFieldOptions = {"",
                "duration",
                "bpm",
                "played",
                "timesplayed",
                "rating"};
        QStringList stringOperatorOptions = {"",
                "contains",
                "does not contain",
                "is",
                "is not",
                "starts with",
                "ends with",
                "is not empty",
                "is empty"};
        QStringList dateOperatorOptions = {"",
                "before",
                "after",
                "is"};
        QStringList numberOperatorOptions = {"",
                "smaller than",
                "bigger than",
                "is",
                "is not"};

        QStringList combinerOptions = {"", ") END", "AND", "OR", ") AND (", ") OR ("};

        int conditionStartIndex = 8; // Adjust based on smartiesData format
        for (int i = 0; i < 12; ++i) {
            int baseIndex = conditionStartIndex + i * 4;

            // Populate fieldComboBox
            auto* fieldComboBox = findChild<QComboBox*>(
                    QString("comboBoxCondition%1Field").arg(i + 1));
            if (fieldComboBox) {
                //                fieldComboBox->setPlaceholderText(tr("Choose Field"));
                fieldComboBox->clear();
                fieldComboBox->addItems(fieldOptions);
                fieldComboBox->setToolTip(tr("Create a new condition") +
                        "\n\n" +
                        tr("You can create a condition by following 4 steps:") +
                        "\n" + tr("1. Select the field") + "\n" +
                        tr("2. Select the operator") + "\n" +
                        tr("3. Enter the value to compare with") + "\n" +
                        tr("4. Select the condition combiner") + "\n\n" +
                        tr("In fact you're creating the whereclause in a sql "
                           "query:") +
                        "\n" +
                        tr("SELECT * FROM LIBRARY WHERE ( ---The condition "
                           "you're creating here---'") +
                        "\n\n" + tr("1. Select the Condition Field") + "\n\n" +
                        tr("All fields that can be used in the condition") +
                        "\n" + tr("are listed in the selection box.") + "\n" +
                        tr("The fields are divided in groups:") + "\n" +
                        tr("Fields which contain text ") + "\n" +
                        tr("    -> artist, title, album, album_artist, ") +
                        "\n" +
                        tr("       genre, comment, composer, filetype, key") +
                        "\n" + tr("Fields which contain 'a kind of date'") +
                        "\n" +
                        tr("    -> year, datetime_added, last_played_at") +
                        "\n" + tr("Fields which contain 'a kind of number'") +
                        "\n" +
                        tr("    -> duration, bpm, played, timesplayed, "
                           "rating") +
                        "\n");

                // Get the field value from smartiesData
                QString fieldText = smartiesData[baseIndex].isNull()
                        ? ""
                        : smartiesData[baseIndex].toString();
                int fieldIndex = fieldComboBox->findText(fieldText);

                if (fieldIndex != -1) {
                    fieldComboBox->setCurrentIndex(fieldIndex);
                } else if (!fieldText.isEmpty()) {
                    fieldComboBox->insertItem(0, fieldText);
                    fieldComboBox->setCurrentIndex(0);
                }

                // Connect field change to update operator combo box
                connect(fieldComboBox,
                        &QComboBox::currentIndexChanged,
                        this,
                        [this,
                                fieldComboBox,
                                i,
                                stringFieldOptions,
                                dateFieldOptions,
                                numberFieldOptions]() {
                            updateOperatorComboBox(fieldComboBox,
                                    i,
                                    stringFieldOptions,
                                    dateFieldOptions,
                                    numberFieldOptions);
                        });
            } // Populate operatorComboBox based on field type
            auto* operatorComboBox = findChild<QComboBox*>(
                    QString("comboBoxCondition%1Operator").arg(i + 1));
            if (operatorComboBox) {
                operatorComboBox->clear();

                // Determine which operator options to display based on the selected field
                QString selectedField = fieldComboBox ? fieldComboBox->currentText() : QString();
                if (stringFieldOptions.contains(selectedField)) {
                    operatorComboBox->addItems(stringOperatorOptions);
                } else if (dateFieldOptions.contains(selectedField)) {
                    operatorComboBox->addItems(dateOperatorOptions);
                } else if (numberFieldOptions.contains(selectedField)) {
                    operatorComboBox->addItems(numberOperatorOptions);
                }

                // Get the operator value from smartiesData
                QString operatorText = smartiesData[baseIndex + 1].isNull()
                        ? ""
                        : smartiesData[baseIndex + 1].toString();

                int operatorIndex = operatorComboBox->findText(operatorText);
                if (operatorIndex != -1) {
                    operatorComboBox->setCurrentIndex(operatorIndex);
                } else if (!operatorText.isEmpty()) {
                    operatorComboBox->insertItem(0, operatorText);
                    operatorComboBox->setCurrentIndex(0);
                }
                operatorComboBox->setToolTip(
                        tr("2. Select the Condition Operator") + "\n\n" +
                        tr("The choice between operators depends on the chosen "
                           "field:") +
                        "\n\n" + tr("Fields which contain text ") + "\n" +
                        tr("    -> artist, title, album, album_artist, ") +
                        "\n" +
                        tr("       genre, comment, composer, filetype, key") +
                        "\n" + tr("can only have a text-operator") + "\n" +
                        tr("    -> contains, does not contain, is, is not,") +
                        "\n" +
                        tr("       starts with, ends with, is not empty, is "
                           "empty") +
                        "\n" +
                        tr("       contain will return more results than "
                           "is...") +
                        "\n\n" + tr("Fields which contain 'a kind of date'") +
                        "\n" +
                        tr("    -> year, datetime_added, last_played_at") +
                        "\n" + tr("can only have a date-operator") + "\n" +
                        tr("    -> before, after, is, is not") + "\n\n" +
                        tr("       before and after will have more results "
                           "than is,") +
                        "\n" +
                        tr("       you can create 2 conditions to narrow your "
                           "result;") +
                        "\n" +
                        tr("       year > 1685 AND year < 1990 will result in "
                           "the tracks in the") +
                        "\n" +
                        tr("       2nd part of the eighties (if you filled in "
                           "the year-trackfield") +
                        "\n\n" + tr("Fields which contain 'a kind of number'") +
                        "\n" +
                        tr("    -> duration, bpm, played, timesplayed, "
                           "rating") +
                        "\n" + tr("can only have a number-operator") + "\n" +
                        tr("    -> smaller than, bigger than, is, is not") +
                        "\n");
            }

            auto* valueLineEdit = findChild<QLineEdit*>(
                    QString("lineEditCondition%1Value").arg(i + 1));
            if (valueLineEdit) {
                valueLineEdit->setText(smartiesData[baseIndex + 2].isNull()
                                ? ""
                                : smartiesData[baseIndex + 2].toString());
                valueLineEdit->setToolTip(tr("3. Enter the Condition Value") +
                        "\n\n" +
                        tr("The Condition Value is the value rhe condition is "
                           "comparing with.") +
                        "\n" +
                        tr("Depending on the field and the operator different "
                           "kinds of values are possible.") +
                        "\n" +
                        tr("If you selected a field which contains text ") +
                        "\n" +
                        tr("    -> artist, title, album, album_artist, ") +
                        "\n" +
                        tr("       genre, comment, composer, filetype, key") +
                        "\n" + tr("and a text-operator") + "\n" +
                        tr("    -> contains, does not contain, is, is not,") +
                        "\n" +
                        tr("       starts with, ends with, is not empty, is "
                           "empty") +
                        "\n\n" +
                        tr("You can enter a string value: a part of / complete "
                           "name of an artist, album...") +
                        "\n" +
                        tr("If you selected a field which contains 'a kind of "
                           "date'") +
                        "\n\n" +
                        tr("    -> year, datetime_added, last_played_at") +
                        "\n" + tr("and a a date-operator") + "\n" +
                        tr("    -> before, after, is, is not") + "\n" +
                        tr("       You can enter for conditions with year a "
                           "year as YYYY (eg 1985 or 2002)") +
                        "\n" +
                        tr("       You can enter for conditions with "
                           "datetime_added the date as YYYY-MM-DD ") +
                        "\n" + tr("       (1985-10-21 or 2002-01-01)") + "\n" +
                        tr("       You can enter for conditions with "
                           "last_played_at you can enter the number of days, "
                           "(last_played_at < 30 ") +
                        "\n" +
                        tr("       will return all played tracks in the last "
                           "month,") +
                        "\n" +
                        tr("       you can combine conditions to narrow the "
                           "search result.") +
                        "\n\n" +
                        tr("If you selected a field which contains 'a kind of "
                           "number'") +
                        "\n" +
                        tr("    -> duration, bpm, played, timesplayed, "
                           "rating") +
                        "\n" + tr("and a number-operator") + "\n" +
                        tr("    -> smaller than, bigger than, is, is not") +
                        "\n" +
                        tr("       You can enter an number to compare with.") +
                        "\n");
            }

            auto* combinerComboBox = findChild<QComboBox*>(
                    QString("comboBoxCondition%1Combiner").arg(i + 1));
            if (combinerComboBox) {
                combinerComboBox->clear();
                combinerComboBox->addItems(combinerOptions);
                QString combinerText = smartiesData[baseIndex + 3].isNull()
                        ? ""
                        : smartiesData[baseIndex + 3].toString();
                int index = combinerComboBox->findText(combinerText);
                if (index != -1) {
                    combinerComboBox->setCurrentIndex(index);
                } else if (!combinerText.isEmpty()) {
                    combinerComboBox->insertItem(0, combinerText);
                    combinerComboBox->setCurrentIndex(0);
                }
                combinerComboBox->setToolTip(
                        tr("4. Select the Condition Combiner") + "\n\n" +
                        tr("Every condition has to end with ') END'") + "\n" +
                        tr("because they all start with a '('") + "\n" +
                        tr("as you could see in the field-tooltip.") + "\n\n" +
                        tr("Between the different conditions are 'combiners "
                           "used") +
                        "\n\n" +
                        tr("You have the choice between 'AND' & 'OR'") +
                        "\n\n" +
                        tr("with 'AND' you can make clear that both Conditions "
                           "need to be forfilled ") +
                        "\n\n" +
                        tr("eg: (Artist contains Prince AND title contains "
                           "rain) will return in the track Purple rain,") +
                        "\n" +
                        tr("    or Prince, if you have that track in your "
                           "collection.") +
                        "\n" +
                        tr("    (Artist contains Prince OR title contains "
                           "rain) will return in all tracks of Prince ") +
                        "\n" +
                        tr("    AND all tracks with 'rain' in the title.") +
                        "\n\n" +
                        tr("The possibility to use parenthesis is to make "
                           "conditions clear and more extensive") +
                        "\n\n" +
                        tr("eg: (Artist contains Prince AND title contains "
                           "rain) OR (Artist contains lovesymbol) OR") +
                        "\n" +
                        tr("    (Artist contains TAFNAP) OR (Artist contains "
                           "time) OR (Artist contains NPG)") +
                        "\n" +
                        tr("    Will return all 'purple rain' - tracks from "
                           "'prince' + all tracks from his ") +
                        "\n" + tr("    'lovesymbol'-name") + "\n" +
                        tr("    + all tracks from his TAFNAP-time and the "
                           "songs he created with 'the time',") +
                        "\n" +
                        tr("    the new power generation ...if you have those "
                           "tracks.") +
                        "\n\n" +
                        tr("Parenthesis and the right USE of 'AND' and 'OR' "
                           "can guve your condition results or not.") +
                        "\n");
            }
        }
    }
    // Connect the combo box and line edit signals for validation
    connectConditions();

    connect(
            applyButton,
            &QPushButton::clicked,
            this,
            &dlgSmartiesInfo::onApplyButtonClicked);
    connect(
            newButton,
            &QPushButton::clicked,
            this,
            &dlgSmartiesInfo::onNewButtonClicked);
    connect(
            previousButton,
            &QPushButton::clicked,
            this,
            &dlgSmartiesInfo::onPreviousButtonClicked);
    connect(
            nextButton,
            &QPushButton::clicked,
            this,
            &dlgSmartiesInfo::onNextButtonClicked);
    connect(
            okButton,
            &QPushButton::clicked,
            this,
            &dlgSmartiesInfo::onOKButtonClicked);
    connect(
            cancelButton,
            &QPushButton::clicked,
            this,
            &dlgSmartiesInfo::onCancelButtonClicked);
    // set
    m_isUpdatingUI = false;
}

void dlgSmartiesInfo::updateOperatorComboBox(
        QComboBox* fieldComboBox,
        int conditionIndex,
        const QStringList& stringFieldOptions,
        const QStringList& dateFieldOptions,
        const QStringList& numberFieldOptions) {
    // Operator combo box must be appropriate to the selected field combo box
    auto* operatorComboBox = findChild<QComboBox*>(
            QString("comboBoxCondition%1Operator").arg(conditionIndex + 1));
    if (!operatorComboBox) {
        return;
    }
    //    operatorComboBox->setPlaceholderText(tr("Choose Operator (choice
    //    restricted by type of field)"));
    operatorComboBox->clear();

    // Get the selected field from the fieldComboBox
    QString selectedField = fieldComboBox->currentText();

    // Define operator lists based on field types
    if (stringFieldOptions.contains(selectedField)) {
        operatorComboBox->addItems({"",
                "contains",
                "does not contain",
                "is",
                "is not",
                "starts with",
                "ends with",
                "is not empty",
                "is empty"});
    } else if (dateFieldOptions.contains(selectedField)) {
        operatorComboBox->addItems({"", "before", "after", "is"});
    } else if (numberFieldOptions.contains(selectedField)) {
        operatorComboBox->addItems({"", "smaller than", "bigger than", "is", "is not"});
    }

    // Ensure default operator is set when switching fields
    if (!m_isUpdatingUI) {
        operatorComboBox->setCurrentIndex(0);
    }
}

void dlgSmartiesInfo::connectConditions() {
    for (int i = 1; i <= 12; ++i) {
        auto* fieldComboBox = findChild<QComboBox*>(QString("comboBoxCondition%1Field").arg(i));
        auto* operatorComboBox = findChild<QComboBox*>(
                QString("comboBoxCondition%1Operator").arg(i));
        auto* valueLineEdit = findChild<QLineEdit*>(QString("lineEditCondition%1Value").arg(i));
        auto* combinerComboBox = findChild<QComboBox*>(
                QString("comboBoxCondition%1Combiner").arg(i));

        if (fieldComboBox && operatorComboBox && valueLineEdit && combinerComboBox) {
            connect(fieldComboBox,
                    &QComboBox::currentTextChanged,
                    this,
                    &dlgSmartiesInfo::updateConditionState);
            connect(operatorComboBox,
                    &QComboBox::currentTextChanged,
                    this,
                    &dlgSmartiesInfo::updateConditionState);
            connect(combinerComboBox,
                    &QComboBox::currentTextChanged,
                    this,
                    &dlgSmartiesInfo::updateConditionState);
            connect(valueLineEdit,
                    &QLineEdit::textChanged,
                    this,
                    &dlgSmartiesInfo::updateConditionState);
        }
    }
}

// Changes the lock-button states in the UI
void dlgSmartiesInfo::toggleLockStatus() {
    if (buttonLock->text() == "Lock") {
        buttonLock->setText("Unlock");
    } else {
        buttonLock->setText("Lock");
    }
}

QVariantList dlgSmartiesInfo::collectUIChanges() const {
    qDebug() << "[SMARTIES] [EDIT DLG] ---> CollectUIChanges Started!";
    QVariantList updatedData;
    updatedData.append(lineEditID->text());
    updatedData.append(lineEditName->text());
    updatedData.append(spinBoxCount->value());
    updatedData.append(checkBoxShow->isChecked());
    updatedData.append(buttonLock->text() == "Unlock");
    updatedData.append(checkBoxAutoDJ->isChecked());
    updatedData.append(lineEditSearchInput->text());
    updatedData.append(lineEditSearchSQL->text());

    for (int i = 1; i <= 12; ++i) {
        QString field = findChild<QComboBox*>(
                QString("comboBoxCondition%1Field").arg(i))
                                ->currentText();
        QString op = findChild<QComboBox*>(
                QString("comboBoxCondition%1Operator").arg(i))
                             ->currentText();
        QString value = findChild<QLineEdit*>(QString("lineEditCondition%1Value").arg(i))->text();
        QString combiner = findChild<QComboBox*>(
                QString("comboBoxCondition%1Combiner").arg(i))
                                   ->currentText();

        qDebug() << "[SMARTIES] [EDIT DLG]---> Collecting Condition "
                 << i << ":"
                 << "Field:" << field
                 << "Operator:" << op
                 << "Value:" << value
                 << "Combiner:" << combiner;

        updatedData.append(field);
        updatedData.append(op);
        updatedData.append(value);
        updatedData.append(combiner);
    }
    qDebug() << "[SMARTIES] [EDIT DLG] ---> CollectUIChanges Finished!";
    qDebug() << "[SMARTIES] [EDIT DLG] ---> Collected data:" << updatedData;
    return updatedData;
}

void dlgSmartiesInfo::onApplyButtonClicked() {
    qDebug() << "[SMARTIES] [EDIT DLG] ---> Apply button clicked!";
    QVariantList editedData = collectUIChanges();
    qDebug() << "[SMARTIES] [EDIT DLG] ---> Data collected for Apply:" << editedData;
    emit dataUpdated(editedData); // Emit signal with updated data if needed
    qDebug() << "[SMARTIES] [EDIT DLG] ---> SIGNAL SND -> Data applied without closing the dialog";
    //    accept();
}

void dlgSmartiesInfo::onNewButtonClicked() {
    // Handle creating a new Smarties entry
    emit requestNewSmarties();
    qDebug() << "[SMARTIES] [EDIT DLG] ---> SIGNAL SND -> New button "
                "clicked, emitted requestNewwSmarties signal";
}

void dlgSmartiesInfo::onPreviousButtonClicked() {
    //    if (!m_isUpdatingUI) { // Only emit if not in update mode
    emit requestPreviousSmarties();
    qDebug() << "[SMARTIES] [EDIT DLG] ---> SIGNAL SND -> Previous button "
                "clicked, emitted requestPreviousSmarties signal";
    //    }
}

void dlgSmartiesInfo::onNextButtonClicked() {
    if (!m_isUpdatingUI) { // Only emit if not in update mode
        emit requestNextSmarties();
        qDebug() << "[SMARTIES] [EDIT DLG] ---> SIGNAL SND -> Next button "
                    "clicked, emitted requestNextSmarties signal";
    }
}

void dlgSmartiesInfo::onOKButtonClicked() {
    qDebug() << "[SMARTIES] [EDIT DLG] ---> OK button clicked!";
    QVariantList editedData = collectUIChanges();
    qDebug() << "[SMARTIES] [EDIT DLG] ---> Data collected for Apply:" << editedData;
    emit dataUpdated(editedData); // Emit signal with updated data if needed
    qDebug() << "[SMARTIES] [EDIT DLG] ---> SIGNAL SND -> Data saved and dialog closed";
    accept();
}

void dlgSmartiesInfo::onCancelButtonClicked() {
    qDebug() << "[SMARTIES] [EDIT DLG] ---> Cancel button clicked!";
    accept();
}

//  begin sifnal grey out / show next condition

void dlgSmartiesInfo::updateConditionState() {
    // Call this function on any change to re-evaluate enable states based on user interaction
    initializeConditionState();
}

// Ensure initial state on startup by calling updateConditionState once
void dlgSmartiesInfo::initializeConditionState() {
    bool enableNextField = true; // Control to enable the next field in sequence

    for (int i = 0; i < 12; ++i) {
        auto* fieldComboBox = findChild<QComboBox*>(QString("comboBoxCondition%1Field").arg(i + 1));
        auto* operatorComboBox = findChild<QComboBox*>(
                QString("comboBoxCondition%1Operator").arg(i + 1));
        auto* valueLineEdit = findChild<QLineEdit*>(QString("lineEditCondition%1Value").arg(i + 1));
        auto* combinerComboBox = findChild<QComboBox*>(
                QString("comboBoxCondition%1Combiner").arg(i + 1));

        if (fieldComboBox && operatorComboBox && valueLineEdit && combinerComboBox) {
            bool fieldSelected = !fieldComboBox->currentText().isEmpty();
            bool operatorSelected = !operatorComboBox->currentText().isEmpty();
            bool valuePresent = !valueLineEdit->text().isEmpty();
            bool combinerSelected = !combinerComboBox->currentText().isEmpty();

            // Set fieldComboBox enabled if either pre-set or we should enable the next empty one
            //            qDebug() << "fieldComboBox Value i: " << i;
            //            qDebug() << "fieldComboBox Value setEnabled: " << fieldSelected;
            fieldComboBox->setEnabled(fieldSelected || enableNextField);

            // Enable other fields based on field selection and presence of pre-existing values
            operatorComboBox->setEnabled(fieldSelected || operatorSelected);
            valueLineEdit->setEnabled((fieldSelected && operatorSelected) || valuePresent);
            combinerComboBox->setEnabled((fieldSelected && operatorSelected) || combinerSelected);

            // Only enable the next field line if this line has values or has been started
            enableNextField = fieldSelected;
        }
    }
}

//  end sifnal grey out / show next condition
