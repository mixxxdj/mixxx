#include "metadatafilesettings.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QFileDialog>
#include <QLineEdit>
#include <QMouseEvent>
#include <QObject>
#include <QTextCodec>

#include "moc_metadatafilesettings.cpp"

FileSettings MetadataFileSettings::s_latestSettings;

MetadataFileSettings::MetadataFileSettings(UserSettingsPointer pSettings,
        const FileWidgets& widgets,
        QWidget* dialogWidget)
        : m_pSettings(pSettings),
          m_CPSettingsChanged(kFileSettingsChanged),
          m_widgets(widgets),
          m_pDialogWidget(dialogWidget),
          m_pDelegate(new ComboboxDelegate),
          m_pNormalDelegate(new QStyledItemDelegate) {
    s_latestSettings = getPersistedSettings(pSettings);
    setupWidgets();
}

MetadataFileSettings::~MetadataFileSettings() {
    delete m_pDelegate;
    delete m_pNormalDelegate;
}

FileSettings MetadataFileSettings::getPersistedSettings(const UserSettingsPointer& pSettings) {
    FileSettings ret;
    ret.enabled =
            pSettings->getValue(kMetadataFileEnabled, defaultFileMetadataEnabled);
    ret.fileEncoding =
            pSettings->getValue(kFileEncoding, defaultEncoding.constData()).toUtf8();
    ret.fileFormatString =
            pSettings->getValue(kFileFormatString, defaultFileFormatString);
    ret.filePath =
            pSettings->getValue(kFilePath, defaultFilePath);
    return ret;
}

void MetadataFileSettings::setupWidgets() {
    m_widgets.enableCheckbox->setChecked(s_latestSettings.enabled);

    setupEncodingComboBox();

    m_widgets.formatLineEdit->setText(s_latestSettings.fileFormatString);

    m_widgets.filePathLineEdit->setText(s_latestSettings.filePath);
    m_widgets.filePathLineEdit->setStyleSheet("");
    QObject::connect(m_widgets.changeFilePathButton, SIGNAL(pressed()), this, SLOT(slotFilepathButtonClicked()));
}

FileSettings MetadataFileSettings::getLatestSettings() {
    return MetadataFileSettings::s_latestSettings;
}

void MetadataFileSettings::applySettings() {
    if (fileSettingsDifferent() && checkIfSettingsCorrect()) {
        updateLatestSettingsAndNotify();
        persistSettings();
    }
}

bool MetadataFileSettings::fileSettingsDifferent() {
    return s_latestSettings.enabled !=
            m_widgets.enableCheckbox->isChecked() ||

            s_latestSettings.fileEncoding !=
            m_widgets.encodingBox->currentText() ||

            s_latestSettings.fileFormatString !=
            m_widgets.formatLineEdit->text() ||

            s_latestSettings.filePath != m_widgets.filePathLineEdit->text();
}

bool MetadataFileSettings::checkIfSettingsCorrect() {
    QString supposedPath = m_widgets.filePathLineEdit->text();
    int lastIndex = supposedPath.lastIndexOf('/');
    if (lastIndex != -1) {
        QString supposedDir = supposedPath.left(lastIndex);
        QDir dir(supposedDir);
        bool dirExists = dir.exists();
        if (!dirExists) {
            m_widgets.filePathLineEdit->setStyleSheet("border: 1px solid red");
        } else {
            m_widgets.filePathLineEdit->setStyleSheet("");
        }
        return dirExists;
    }
    return true;
}

void MetadataFileSettings::updateLatestSettingsAndNotify() {
    FileSettings ret;
    ret.enabled = m_widgets.enableCheckbox->isChecked();
    ret.fileEncoding = m_widgets.encodingBox->currentText().toUtf8();
    ret.fileFormatString = m_widgets.formatLineEdit->text();
    ret.filePath = QDir(m_widgets.filePathLineEdit->text()).absolutePath();
    s_latestSettings = ret;
    m_CPSettingsChanged.set(true);
}

void MetadataFileSettings::persistSettings() {
    m_pSettings->setValue(kMetadataFileEnabled, s_latestSettings.enabled);
    m_pSettings->setValue(kFileEncoding, QString(s_latestSettings.fileEncoding));
    m_pSettings->setValue(kFileFormatString, s_latestSettings.fileFormatString);
    m_pSettings->setValue(kFilePath, s_latestSettings.filePath);
}

void MetadataFileSettings::setSettingsToDefault() {
    resetSettingsToDefault();
    setupWidgets();
}

void MetadataFileSettings::resetSettingsToDefault() {
    s_latestSettings.enabled = defaultFileMetadataEnabled;
    s_latestSettings.fileEncoding = defaultEncoding;
    s_latestSettings.fileFormatString = defaultFileFormatString;
    s_latestSettings.filePath = defaultFilePath;
}

void MetadataFileSettings::slotFilepathButtonClicked() {
    QString newFilePath = QFileDialog::getSaveFileName(
            m_pDialogWidget,
            "Choose new file path",
            checkIfSettingsCorrect() ? m_widgets.filePathLineEdit->text() : defaultFilePath,
            "Text files(*.txt)");
    m_widgets.filePathLineEdit->setText(newFilePath);
}

void MetadataFileSettings::cancelSettings() {
    setupWidgets();
}

void MetadataFileSettings::setupEncodingComboBox() {
    m_widgets.encodingBox->clear();
    QList<QByteArray> codecs = QTextCodec::availableCodecs();

    QList<QByteArray> preferredCodecs = {
            "latin1",
            "UTF-8"};

    for (const QByteArray& codec : preferredCodecs) {
        m_widgets.encodingBox->addItem(codec);
        codecs.removeAll(codec);
    }

    if (preferredCodecs.contains(s_latestSettings.fileEncoding)) {
        m_widgets.encodingBox->view()->setItemDelegate(m_pDelegate);
        QAbstractItemModel* comboboxModel = m_widgets.encodingBox->model();
        comboboxModel->insertRow(
                comboboxModel->rowCount());
        comboboxModel->setData(
                comboboxModel->index(comboboxModel->rowCount() - 1, 0),
                true,
                Qt::UserRole);

        connect(m_pDelegate, &ComboboxDelegate::moreButtonPressed, this, &MetadataFileSettings::slotMoreButtonComboboxPressed);

        m_remainingCodecs = codecs;
    }

    else {
        for (const QByteArray& codec : codecs) {
            m_widgets.encodingBox->addItem(codec);
        }
    }
    m_widgets.encodingBox->setCurrentText(s_latestSettings.fileEncoding);
}

void MetadataFileSettings::slotMoreButtonComboboxPressed() {
    QAbstractItemModel* model = m_widgets.encodingBox->model();
    model->removeRow(model->rowCount() - 1);
    m_widgets.encodingBox->view()->setItemDelegate(m_pNormalDelegate);
    for (const QByteArray& codec : m_remainingCodecs) {
        m_widgets.encodingBox->addItem(codec);
    }
    m_widgets.encodingBox->setEditable(true);
}

void ComboboxDelegate::paint(QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    if (index.row() == 2) {
        QStyleOptionButton buttonOption;
        buttonOption.rect = option.rect;
        buttonOption.state = QStyle::State_Raised;
        buttonOption.text = "More...";
        QApplication::style()->drawControl(QStyle::CE_PushButton,
                &buttonOption,
                painter);
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

bool ComboboxDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) {
    if (event->type() == QEvent::MouseButtonPress &&
            model->data(index, Qt::UserRole).toBool()) {
        QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
        DEBUG_ASSERT(mouseEvent);
        if (mouseEvent->button() == Qt::LeftButton) {
            emit moreButtonPressed();
            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
