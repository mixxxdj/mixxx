#include "wsearchpopup.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QCheckBox>
#include <QDate>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMap>
#include <QMessageBox>
#include <QProxyStyle>
#include <QRegularExpression>
#include <QShowEvent>
#include <QStyleFactory>
#include <QStyleOption>
#include <QVBoxLayout>

class ComboBoxStyle : public QProxyStyle {
  public:
    int styleHint(StyleHint hint,
            const QStyleOption* option = nullptr,
            const QWidget* widget = nullptr,
            QStyleHintReturn* returnData = nullptr) const override {
        if (hint == SH_ComboBox_Popup) {
            return 0;
        }
        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
};

#include "moc_wsearchpopup.cpp"
#include "preferences/configobject.h"

namespace {
const QString kSavedQueriesConfigGroup = QStringLiteral("[SearchQueries]");
const QString kFastSearchConfigGroup = QStringLiteral("[SearchPopup]");
constexpr int kMaxSearchEntries = 30;
constexpr int kMinFontSize = 6;
constexpr int kMaxFontSize = 30;

constexpr int kDefaultWidth = 800;
constexpr int kDefaultHeight = 420;

struct ParsedTerm {
    QString field;
    QString value;
};

QList<ParsedTerm> parseSearchTerms(const QString& text) {
    QString cleanedText = text;
    cleanedText.replace("?", "");
    cleanedText.replace(",", "");
    cleanedText.replace(";", ":");
    cleanedText.replace("::", ":");
    cleanedText.replace(":::", ":");
    cleanedText.replace("~", "");
    cleanedText.replace("=", "");
    cleanedText.replace("|", " ");

    const QStringList patternFields = {"artist:",
            "album_artist:",
            "album:",
            "title:",
            "genre:",
            "composer:",
            "grouping:",
            "comment:",
            "location:",
            "filetype:",
            "played:",
            "rating:",
            "year:",
            "key:",
            "bpm:",
            "duration:",
            "datetime_added:"};

    const QString patternFieldsJoined = patternFields.join("|");
    const QString pattern = QString(R"(\b(%1)\s*([^:]+?)(?=\s*\b(?:%1|$)))")
                                    .arg(patternFieldsJoined);

    static QRegularExpression termRegex(
            pattern, QRegularExpression::CaseInsensitiveOption);

    QList<ParsedTerm> result;
    auto it = termRegex.globalMatch(cleanedText);
    while (it.hasNext()) {
        const auto match = it.next();
        const QString term = match.captured(1).split(":")[0].trimmed();
        const QString value = match.captured(2).trimmed();
        if (!term.isEmpty() && !value.isEmpty()) {
            result.append({term, value});
        }
    }
    return result;
}
} // namespace

WSearchPopup::WSearchPopup(UserSettingsPointer pConfig, QWidget* parent)
        : QDialog(parent),
          m_pConfig(pConfig) {
    setWindowTitle(tr("Fast Search"));

    QStyle* fusion = QStyleFactory::create(QStringLiteral("Fusion"));
    if (fusion) {
        setStyle(fusion);
    }

    QPalette pal;
    pal.setColor(QPalette::Window, QColor(40, 40, 40));
    pal.setColor(QPalette::WindowText, QColor(220, 220, 220));
    pal.setColor(QPalette::Base, QColor(30, 30, 30));
    pal.setColor(QPalette::AlternateBase, QColor(40, 40, 40));
    pal.setColor(QPalette::Text, QColor(220, 220, 220));
    pal.setColor(QPalette::Button, QColor(60, 60, 60));
    pal.setColor(QPalette::ButtonText, QColor(220, 220, 220));
    pal.setColor(QPalette::Highlight, QColor(80, 120, 200));
    pal.setColor(QPalette::HighlightedText, Qt::white);
    pal.setColor(QPalette::ToolTipBase, QColor(40, 40, 40));
    pal.setColor(QPalette::ToolTipText, QColor(220, 220, 220));
    pal.setColor(QPalette::PlaceholderText, QColor(150, 150, 150));
    pal.setColor(QPalette::Light, QColor(40, 40, 40).lighter(160));
    pal.setColor(QPalette::Midlight, QColor(40, 40, 40).lighter(130));
    pal.setColor(QPalette::Mid, QColor(40, 40, 40).darker(120));
    pal.setColor(QPalette::Dark, QColor(40, 40, 40).darker(150));
    pal.setColor(QPalette::Shadow, QColor(0, 0, 0));
    pal.setColor(QPalette::Disabled, QPalette::Text, QColor(120, 120, 120));
    pal.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(120, 120, 120));

    setPalette(pal);
    setAutoFillBackground(true);

    m_historyCombo = new QComboBox(this);
    m_historyCombo->setEditable(false);
    m_historyCombo->setInsertPolicy(QComboBox::NoInsert);
    m_historyCombo->setSizeAdjustPolicy(
            QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_historyCombo->setMinimumContentsLength(30);

    m_historyCombo->setStyle(new ComboBoxStyle);
    m_historyCombo->setMaxVisibleItems(8);
    (void)m_historyCombo->view();
    connect(m_historyCombo,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &WSearchPopup::slotHistoryActivated);

    m_trackArtist = new QLineEdit(this);
    m_trackTitle = new QLineEdit(this);
    m_albumArtist = new QLineEdit(this);
    m_album = new QLineEdit(this);
    m_year = new QLineEdit(this);
    m_composer = new QLineEdit(this);
    m_key = new QLineEdit(this);
    m_bpm = new QLineEdit(this);
    m_genre = new QLineEdit(this);
    m_grouping = new QLineEdit(this);
    m_location = new QLineEdit(this);
    m_dateAdded = new QLineEdit(this);
    m_dateAdded->setToolTip(tr(
            "Number of days (e.g. 10 = last 10 days)\n"
            "or absolute date YYYY-MM-DD or YYYY-MM-DD-YYYY-MM-DD"));

    m_searchButton = new QPushButton(tr("Search"), this);
    m_clearButton = new QPushButton(tr("Clear"), this);
    m_closeOnSearchCheckBox = new QCheckBox(tr("Close on search"), this);
    m_closeOnSearchCheckBox->setChecked(
            m_pConfig
                    ? m_pConfig->getValue<bool>(
                              ConfigKey(kFastSearchConfigGroup,
                                      QStringLiteral("CloseOnSearch")),
                              true)
                    : true);

    m_fontSmallerButton = new QToolButton(this);
    m_fontSmallerButton->setText(QStringLiteral("A-"));
    m_fontSmallerButton->setToolTip(tr("Decrease font size"));

    m_fontLargerButton = new QToolButton(this);
    m_fontLargerButton->setText(QStringLiteral("A+"));
    m_fontLargerButton->setToolTip(tr("Increase font size"));

    connect(m_fontSmallerButton,
            &QToolButton::clicked,
            this,
            &WSearchPopup::slotFontSmaller);
    connect(m_fontLargerButton,
            &QToolButton::clicked,
            this,
            &WSearchPopup::slotFontLarger);

    QFormLayout* formLayout = new QFormLayout;
    formLayout->addRow(tr("Track Artist:"), m_trackArtist);
    formLayout->addRow(tr("Track Title:"), m_trackTitle);
    formLayout->addRow(tr("Album Artist:"), m_albumArtist);
    formLayout->addRow(tr("Album:"), m_album);
    formLayout->addRow(tr("Year:"), m_year);
    formLayout->addRow(tr("Composer:"), m_composer);
    formLayout->addRow(tr("Key:"), m_key);
    formLayout->addRow(tr("BPM:"), m_bpm);
    formLayout->addRow(tr("Genre:"), m_genre);
    formLayout->addRow(tr("Grouping:"), m_grouping);
    formLayout->addRow(tr("Location:"), m_location);
    formLayout->addRow(tr("Date Added:"), m_dateAdded);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(m_searchButton);
    buttonLayout->addWidget(m_clearButton);
    buttonLayout->addWidget(m_closeOnSearchCheckBox);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_fontSmallerButton);
    buttonLayout->addWidget(m_fontLargerButton);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(new QLabel(tr("History:"), this));
    mainLayout->addWidget(m_historyCombo);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    setStyleSheet(QStringLiteral(R"(
    QDialog { background-color: #282828; }
    QLabel { color: #dcdcdc; }
    QLineEdit, QComboBox {
        background-color: #1e1e1e;
        color: #dcdcdc;
        border: 1px solid #3a3a3a;
        padding: 2px 4px;
        selection-background-color: #5078c8;
        selection-color: #ffffff;
    }
    QLineEdit:focus, QComboBox:focus { border: 1px solid #5078c8; }
    QComboBox QAbstractItemView {
        background-color: #1e1e1e;
        color: #dcdcdc;
        border: 1px solid #3a3a3a;
        selection-background-color: #5078c8;
        selection-color: #ffffff;
    }
    QPushButton, QToolButton {
        background-color: #3c3c3c;
        color: #dcdcdc;
        border: 1px solid #505050;
        padding: 4px 10px;
    }
    QPushButton:hover, QToolButton:hover { background-color: #4a4a4a; }
    QPushButton:pressed, QToolButton:pressed { background-color: #2e2e2e; }
    QCheckBox { color: #dcdcdc; }
    QCheckBox::indicator {
        width: 14px;
        height: 14px;
        background-color: #1e1e1e;
        border: 1px solid #505050;
    }
    QCheckBox::indicator:checked {
        background-color: #5078c8;
        border: 1px solid #5078c8;
    }
)"));

    connect(m_searchButton,
            &QPushButton::clicked,
            this,
            &WSearchPopup::onSearchClicked);
    connect(m_clearButton,
            &QPushButton::clicked,
            this,
            &WSearchPopup::slotClearFields);
    connect(m_closeOnSearchCheckBox,
            &QCheckBox::toggled,
            this,
            &WSearchPopup::slotCloseOnSearchToggled);

    QSize savedSize(kDefaultWidth, kDefaultHeight);
    if (m_pConfig) {
        m_fontSize = m_pConfig->getValue<int>(
                ConfigKey(kFastSearchConfigGroup, QStringLiteral("FontSize")),
                0);
        const int w = m_pConfig->getValue<int>(
                ConfigKey(kFastSearchConfigGroup, QStringLiteral("Width")),
                kDefaultWidth);
        const int h = m_pConfig->getValue<int>(
                ConfigKey(kFastSearchConfigGroup, QStringLiteral("Height")),
                kDefaultHeight);
        if (w > 0 && h > 0) {
            savedSize = QSize(w, h);
        }
    }
    resize(savedSize);

    applyDarkPalette();
    applyFontSize();
    loadHistory();
}

WSearchPopup::~WSearchPopup() {
    if (m_pConfig) {
        m_pConfig->setValue(
                ConfigKey(kFastSearchConfigGroup, QStringLiteral("Width")),
                width());
        m_pConfig->setValue(
                ConfigKey(kFastSearchConfigGroup, QStringLiteral("Height")),
                height());
    }
    saveHistory();
}

void WSearchPopup::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    applyDarkPalette();
}

void WSearchPopup::applyDarkPalette() {
    if (m_historyCombo && m_historyCombo->view()) {
        m_historyCombo->view()->setPalette(palette());
        m_historyCombo->view()->viewport()->setPalette(palette());
    }
}

void WSearchPopup::applyFontSize() {
    if (m_fontSize <= 0) {
        m_fontSize = font().pointSize();
        if (m_fontSize <= 0) {
            m_fontSize = 10;
        }
    }
    QFont f = font();
    f.setPointSize(m_fontSize);
    setFont(f);

    const QList<QWidget*> children = findChildren<QWidget*>();
    for (auto* w : children) {
        w->setFont(f);
        w->updateGeometry();
    }

    if (m_historyCombo && m_historyCombo->view()) {
        m_historyCombo->view()->setFont(f);
    }

    if (layout()) {
        layout()->invalidate();
        layout()->activate();
    }
    QApplication::processEvents();
    if (layout()) {
        const QSize hint = layout()->sizeHint();
        const int newWidth = qMax(width(), qMax(hint.width(), kDefaultWidth));
        const int newHeight = qMax(hint.height(), kDefaultHeight);
        resize(newWidth, newHeight);
    }
}

void WSearchPopup::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        close();
    } else {
        QDialog::keyPressEvent(event);
    }
}

void WSearchPopup::slotFontSmaller() {
    applyFontSize();
    if (m_fontSize > kMinFontSize) {
        --m_fontSize;
        applyFontSize();
        if (m_pConfig) {
            m_pConfig->setValue(
                    ConfigKey(kFastSearchConfigGroup, QStringLiteral("FontSize")),
                    m_fontSize);
        }
    }
}

void WSearchPopup::slotFontLarger() {
    applyFontSize();
    if (m_fontSize < kMaxFontSize) {
        ++m_fontSize;
        applyFontSize();
        if (m_pConfig) {
            m_pConfig->setValue(
                    ConfigKey(kFastSearchConfigGroup, QStringLiteral("FontSize")),
                    m_fontSize);
        }
    }
}

void WSearchPopup::slotCloseOnSearchToggled(bool checked) {
    if (m_pConfig) {
        m_pConfig->setValue(
                ConfigKey(kFastSearchConfigGroup, QStringLiteral("CloseOnSearch")),
                checked);
    }
}

void WSearchPopup::loadHistory() {
    if (!m_pConfig) {
        return;
    }
    m_historyCombo->clear();
    const QList<ConfigKey> queryKeys =
            m_pConfig->getKeysWithGroup(kSavedQueriesConfigGroup);
    QList<ConfigKey> sortedKeys = queryKeys;
    std::sort(sortedKeys.begin(),
            sortedKeys.end(),
            [](const ConfigKey& a, const ConfigKey& b) {
                return a.item.toInt() < b.item.toInt();
            });
    for (const auto& key : sortedKeys) {
        const QString text = m_pConfig->getValueString(key).trimmed();
        if (!text.isEmpty()) {
            m_historyCombo->addItem(text);
        }
    }
    m_historyCombo->setCurrentIndex(-1);
}

void WSearchPopup::saveHistory() {
    if (!m_pConfig) {
        return;
    }
    const QList<ConfigKey> queryKeys =
            m_pConfig->getKeysWithGroup(kSavedQueriesConfigGroup);
    for (const auto& key : queryKeys) {
        m_pConfig->remove(key);
    }
    for (int i = 0; i < m_historyCombo->count(); ++i) {
        m_pConfig->setValue(
                ConfigKey(kSavedQueriesConfigGroup, QString::number(i)),
                m_historyCombo->itemText(i).trimmed());
    }
}

void WSearchPopup::addToHistory(const QString& entry) {
    if (entry.isEmpty()) {
        return;
    }
    QString toStore = entry;
    const QStringList lines = entry.split('\n');
    for (const QString& part : lines) {
        if (part.startsWith(QStringLiteral("query: "))) {
            toStore = part.mid(7).trimmed();
            break;
        }
    }
    const int existing = m_historyCombo->findText(toStore);
    if (existing >= 0) {
        m_historyCombo->removeItem(existing);
    }
    m_historyCombo->insertItem(0, toStore);
    while (m_historyCombo->count() > kMaxSearchEntries) {
        m_historyCombo->removeItem(m_historyCombo->count() - 1);
    }
    m_historyCombo->setCurrentIndex(-1);
}

void WSearchPopup::slotHistoryActivated(int index) {
    if (index < 0) {
        return;
    }
    restoreFromHistory(m_historyCombo->itemText(index));
}

void WSearchPopup::restoreFromHistory(const QString& entry) {
    QString source;
    const QStringList lines = entry.split('\n');
    bool foundStructured = false;
    for (const QString& part : lines) {
        if (part.startsWith(QStringLiteral("query: "))) {
            source = part.mid(7).trimmed();
            foundStructured = true;
            break;
        }
    }
    if (!foundStructured) {
        for (const QString& part : lines) {
            if (part.startsWith(QStringLiteral("userinput: "))) {
                source = part.mid(11).trimmed();
                foundStructured = true;
                break;
            }
        }
    }
    if (!foundStructured) {
        source = entry.trimmed();
    }
    if (source.isEmpty()) {
        return;
    }

    m_trackArtist->clear();
    m_trackTitle->clear();
    m_albumArtist->clear();
    m_album->clear();
    m_year->clear();
    m_composer->clear();
    m_key->clear();
    m_bpm->clear();
    m_genre->clear();
    m_grouping->clear();
    m_location->clear();
    m_dateAdded->clear();
    m_bareTerm.clear();

    QMap<QString, QStringList> byField;
    const QList<ParsedTerm> terms = parseSearchTerms(source);
    for (const ParsedTerm& t : terms) {
        auto& list = byField[t.field.toLower()];
        if (!list.contains(t.value)) {
            list.append(t.value);
        }
    }

    auto setField = [](QLineEdit* le, const QStringList& values) {
        if (!values.isEmpty()) {
            le->setText(values.join(QStringLiteral("|")));
        }
    };

    setField(m_trackArtist, byField.value(QStringLiteral("artist")));
    setField(m_trackTitle, byField.value(QStringLiteral("title")));
    setField(m_albumArtist, byField.value(QStringLiteral("album_artist")));
    setField(m_album, byField.value(QStringLiteral("album")));
    setField(m_year, byField.value(QStringLiteral("year")));
    setField(m_composer, byField.value(QStringLiteral("composer")));
    setField(m_key, byField.value(QStringLiteral("key")));
    setField(m_bpm, byField.value(QStringLiteral("bpm")));
    setField(m_genre, byField.value(QStringLiteral("genre")));
    setField(m_grouping, byField.value(QStringLiteral("grouping")));
    setField(m_location, byField.value(QStringLiteral("location")));
    setField(m_dateAdded, byField.value(QStringLiteral("datetime_added")));

    if (terms.isEmpty()) {
        m_trackArtist->setText(source);
        m_trackTitle->setText(source);
        m_albumArtist->setText(source);
        m_album->setText(source);
        m_composer->setText(source);
        m_bareTerm = source;
    }
}

void WSearchPopup::onSearchClicked() {
    const QString query = generateQuery();
    if (query.isEmpty()) {
        QMessageBox::warning(this,
                tr("Error"),
                tr("Please enter at least one search criterion or abort with "
                   "ESC."));
        return;
    }
    addToHistory(query);
    saveHistory();
    emit searchRequest(query);
    if (m_closeOnSearchCheckBox && m_closeOnSearchCheckBox->isChecked()) {
        close();
    }
}
void WSearchPopup::slotClearFields() {
    m_trackArtist->clear();
    m_trackTitle->clear();
    m_albumArtist->clear();
    m_album->clear();
    m_year->clear();
    m_composer->clear();
    m_key->clear();
    m_bpm->clear();
    m_genre->clear();
    m_grouping->clear();
    m_location->clear();
    m_dateAdded->clear();
    m_bareTerm.clear();
    m_historyCombo->setCurrentIndex(-1);
    m_trackArtist->setFocus();
}

QString WSearchPopup::generateQuery() const {
    if (!m_bareTerm.isEmpty()) {
        const QStringList values = {
                m_trackArtist->text(),
                m_trackTitle->text(),
                m_albumArtist->text(),
                m_album->text(),
                m_composer->text()};
        bool allSame = true;
        for (const QString& v : values) {
            if (v != m_bareTerm) {
                allSame = false;
                break;
            }
        }
        if (allSame) {
            return "userinput: " + m_bareTerm + "\nquery: " + m_bareTerm;
        }
    }

    QMap<QString, QStringList> fieldValues;
    QStringList userInputList;

    auto addField = [&fieldValues, &userInputList](
                            const QString& field, const QString& value) {
        if (!value.isEmpty()) {
            QStringList values = value.split("|", Qt::SkipEmptyParts);
            for (QString& val : values) {
                val = val.trimmed();
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
    addField("genre", m_genre->text());
    addField("grouping", m_grouping->text());
    addField("location", m_location->text());

    QString dateAddedArg = m_dateAdded->text().trimmed();
    if (!dateAddedArg.isEmpty()) {
        bool isNumber = false;
        const int days = dateAddedArg.toInt(&isNumber);
        if (isNumber && days > 0) {
            const QDate today = QDate::currentDate();
            const QDate from = today.addDays(-days);
            dateAddedArg = QStringLiteral(">") + from.toString(QStringLiteral("yyyy-MM-dd"));
        }
    }
    addField("datetime_added", dateAddedArg);

    QString userInput = userInputList.join("|");

    QStringList fieldsWithPipes;
    QStringList fieldsWithoutPipes;

    for (auto it = fieldValues.begin(); it != fieldValues.end(); ++it) {
        if (it.value().size() > 1) {
            fieldsWithPipes.append(it.key());
        } else {
            fieldsWithoutPipes.append(it.key());
        }
    }

    QStringList queryCombinations;

    if (!fieldsWithPipes.isEmpty()) {
        QStringList pipeCombinations;

        std::function<void(const QStringList&, int, const QString&)> generateCombinations;
        generateCombinations = [&fieldValues,
                                       &pipeCombinations,
                                       &generateCombinations](
                                       const QStringList& fields,
                                       int index,
                                       const QString& currentCombination) {
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
        QStringList conditions;
        for (auto it = fieldValues.begin(); it != fieldValues.end(); ++it) {
            if (!it.value().isEmpty()) {
                conditions.append(it.key() + ":" + it.value().join(" "));
            }
        }
        queryCombinations.append(conditions.join(" "));
    }

    return "userinput: " + userInput + "\nquery: " + queryCombinations.join(" | ");
}
