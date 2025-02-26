#include "wfastsearch.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMessageBox>
#include <QVBoxLayout>

#include "moc_wfastsearch.cpp"

WFastSearch::WFastSearch(QWidget* parent)
        : QDialog(parent) {
    setWindowTitle("Fast Search");
    resize(600, 300);

    // Create input fields
    m_trackArtist = new QLineEdit(this);
    m_trackTitle = new QLineEdit(this);
    m_albumArtist = new QLineEdit(this);
    m_album = new QLineEdit(this);
    m_year = new QLineEdit(this);
    m_composer = new QLineEdit(this);
    m_key = new QLineEdit(this);
    m_bpm = new QLineEdit(this);

    // Create buttons
    m_searchButton = new QPushButton("Search", this);
    m_search2CrateButton = new QPushButton("Search to Crate", this);

    // Layout
    QFormLayout* formLayout = new QFormLayout;
    formLayout->addRow("Track Artist:", m_trackArtist);
    formLayout->addRow("Track Title:", m_trackTitle);
    formLayout->addRow("Album Artist:", m_albumArtist);
    formLayout->addRow("Album:", m_album);
    formLayout->addRow("Year:", m_year);
    formLayout->addRow("Composer:", m_composer);
    formLayout->addRow("Key:", m_key);
    formLayout->addRow("BPM:", m_bpm);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(m_searchButton);
    buttonLayout->addWidget(m_search2CrateButton);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);

    // Connect buttons to slots
    connect(m_searchButton, &QPushButton::clicked, this, &WFastSearch::onSearchClicked);
    connect(m_search2CrateButton, &QPushButton::clicked, this, &WFastSearch::onSearch2CrateClicked);
}

WFastSearch::~WFastSearch() {
}

void WFastSearch::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        close();
    } else {
        QDialog::keyPressEvent(event);
    }
}

void WFastSearch::onSearchClicked() {
    QString query = generateQuery();
    if (query.isEmpty()) {
        QMessageBox::warning(this,
                "Error",
                "Please enter at least one search criterion or abort with "
                "ESC.");
        return;
    }

    emit searchRequest(query);
    close();
}

void WFastSearch::onSearch2CrateClicked() {
    QString query = generateQuery();
    if (query.isEmpty()) {
        QMessageBox::warning(this,
                "Error",
                "Please enter at least one search criterion or abort with "
                "ESC.");
        return;
    }

    emit search2CrateRequest(query);
    close();
}

QString WFastSearch::generateQuery() const {
    QMap<QString, QStringList> fieldValues;
    QStringList userInputList;

    // Het field values
    auto addField = [&fieldValues, &userInputList](const QString& field, const QString& value) {
        if (!value.isEmpty()) {
            // userInputList.append(field + ":" + value);
            QStringList values = value.split("|", Qt::SkipEmptyParts);
            for (QString& val : values) {
                val = val.trimmed();
                //    userInputList.append(field + ":" + val + "|");

                // if (val.contains("*")) {
                //     values.append(val.split("*", Qt::SkipEmptyParts));
                //     values.removeOne(val);
                // } else {
                if (val.contains("*")) {
                    QStringList splitValues = val.split("*");
                    splitValues.removeAll("");
                    values.append(splitValues);
                    for (const QString& splitVal : std::as_const(splitValues)) {
                        userInputList.append(field + ":" + splitVal);
                    }
                    values.removeOne(val);
                } else {
                    userInputList.append(field + ":" + val);
                }
            }
            fieldValues[field] = values;
        }
    };

    addField("artist", m_trackArtist->text());
    addField("title", m_trackTitle->text());
    addField("album_artist", m_albumArtist->text());
    addField("album", m_album->text());
    addField("year", m_year->text());
    addField("composer", m_composer->text());
    addField("key", m_key->text());
    addField("bpm", m_bpm->text());

    QString userInput = userInputList.join(" ");

    QStringList fieldsWithPipes;
    QStringList fieldsWithoutPipes;

    // Separate fields with and without | (pipe)
    for (auto it = fieldValues.begin(); it != fieldValues.end(); ++it) {
        if (it.value().size() > 1) {
            fieldsWithPipes.append(it.key());
        } else {
            fieldsWithoutPipes.append(it.key());
        }
    }

    QStringList queryCombinations;

    if (!fieldsWithPipes.isEmpty()) {
        // Generate combinations if | (pipe) exist
        QStringList pipeCombinations;

        std::function<void(const QStringList&, int, const QString&)> generateCombinations;
        generateCombinations = [&fieldValues,
                                       &pipeCombinations,
                                       &generateCombinations](
                                       const QStringList& fields,
                                       int index,
                                       QString currentCombination) {
            if (index >= fields.size()) {
                pipeCombinations.append(currentCombination.trimmed());
                return;
            }
            QString field = fields[index];
            for (const QString& value : std::as_const(fieldValues[field])) {
                generateCombinations(fields,
                        index + 1,
                        currentCombination +
                                (currentCombination.isEmpty() ? "" : " ") +
                                field + ":" + value);
            }
        };

        generateCombinations(fieldsWithPipes, 0, "");

        // Combine with fields without | (pipes)
        QString staticFields;
        for (const QString& field : fieldsWithoutPipes) {
            if (!fieldValues[field].isEmpty()) {
                staticFields += (staticFields.isEmpty() ? "" : " ") + field +
                        ":" + fieldValues[field].join(" ");
            }
        }

        for (const QString& combination : pipeCombinations) {
            queryCombinations.append(staticFields.isEmpty()
                            ? combination
                            : staticFields + " " + combination);
        }
    } else {
        // No | (pipe) -> simple concatenation
        QStringList conditions;
        for (auto it = fieldValues.begin(); it != fieldValues.end(); ++it) {
            if (!it.value().isEmpty()) {
                conditions.append(it.key() + ":" + it.value().join(" "));
            }
        }
        queryCombinations.append(conditions.join(" "));
    }

    // return queryCombinations.join(" | ");
    qDebug() << "[FASTSEARCH] -> userinput: " << userInput;
    qDebug() << "[FASTSEARCH] -> queryCombinations: " << queryCombinations.join(" | ");

    return "userinput: " + userInput + "\nquery: " + queryCombinations.join(" | ");
}
