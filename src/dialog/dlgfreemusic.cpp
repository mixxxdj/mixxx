#include "dialog/dlgfreemusic.h"

#include <iterator>

#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>

#include "moc_dlgfreemusic.cpp"
#include "util/desktophelper.h"

namespace {

struct FreeMusicSource {
    const char* name;
    const char* description;
    const char* url;
    /// Search URL template with a %s placeholder for the percent-encoded
    /// query. Empty if the source does not support direct search.
    const char* searchUrl;
    /// Note shown when the source streams tracks for free in the browser
    /// without downloading. Empty if it does not stream.
    const char* streamingNote;
};

constexpr FreeMusicSource kFreeMusicSources[] = {
        {QT_TRANSLATE_NOOP("DlgFreeMusic", "YouTube Audio Library"),
                QT_TRANSLATE_NOOP("DlgFreeMusic",
                        "Free music and sound effects offered by YouTube."),
                "https://www.youtube.com/audiolibrary",
                "",
                QT_TRANSLATE_NOOP("DlgFreeMusic",
                        "Streams free in YouTube (Google account required)")},
        {QT_TRANSLATE_NOOP("DlgFreeMusic", "Jamendo"),
                QT_TRANSLATE_NOOP("DlgFreeMusic",
                        "Independent music with licensing information for creators."),
                "https://www.jamendo.com/",
                "https://www.jamendo.com/search?qs=q=%s",
                QT_TRANSLATE_NOOP("DlgFreeMusic", "Streams free in your browser")},
        {QT_TRANSLATE_NOOP("DlgFreeMusic", "Free Music Archive"),
                QT_TRANSLATE_NOOP("DlgFreeMusic",
                        "Curated music with license details on each track."),
                "https://freemusicarchive.org/",
                "https://freemusicarchive.org/search/?quicksearch=%s",
                QT_TRANSLATE_NOOP("DlgFreeMusic", "Streams free in your browser")},
        {QT_TRANSLATE_NOOP("DlgFreeMusic", "Internet Archive Audio"),
                QT_TRANSLATE_NOOP(
                        "DlgFreeMusic", "Public-domain and openly licensed audio collections."),
                "https://archive.org/details/audio",
                "https://archive.org/search?query=%s&and[]=mediatype%3A%22audio%22",
                QT_TRANSLATE_NOOP("DlgFreeMusic", "Streams free in your browser")},
        {QT_TRANSLATE_NOOP("DlgFreeMusic", "ccMixter"),
                QT_TRANSLATE_NOOP(
                        "DlgFreeMusic", "Creative Commons remixes, samples, and instrumentals."),
                "https://ccmixter.org/",
                "https://dig.ccmixter.org/search?searchp=%s",
                QT_TRANSLATE_NOOP("DlgFreeMusic", "Streams free in your browser")},
        {QT_TRANSLATE_NOOP("DlgFreeMusic", "Bandcamp free downloads"),
                QT_TRANSLATE_NOOP(
                        "DlgFreeMusic", "Search releases where the artist enables free downloads."),
                "https://bandcamp.com/tag/free-download",
                "https://bandcamp.com/search?q=%s&item_type=t",
                QT_TRANSLATE_NOOP("DlgFreeMusic", "Streams free in your browser")},
};

} // namespace

DlgFreeMusic::DlgFreeMusic(QWidget* pParent)
        : QDialog(pParent),
          m_pSources(new QListWidget(this)) {
    setWindowTitle(tr("Free Music Sources"));
    setMinimumSize(520, 420);

    auto* pLayout = new QVBoxLayout(this);

    auto* pIntro = new QLabel(
            tr("Browse legal free-music sources in your web browser. "
               "All of them let you stream tracks for free without downloading. "
               "Streams stay outside Mixxx; to load a track into a deck, download "
               "it legally first. Use the search box to search the selected source."),
            this);
    pIntro->setWordWrap(true);
    pLayout->addWidget(pIntro);

    auto* pSearchRow = new QHBoxLayout;
    pSearchRow->addWidget(new QLabel(tr("Search:"), this));
    m_pSearchEdit = new QLineEdit(this);
    m_pSearchEdit->setPlaceholderText(tr("Search the selected source"));
    m_pSearchEdit->setClearButtonEnabled(true);
    connect(m_pSearchEdit, &QLineEdit::returnPressed, this, &DlgFreeMusic::slotSearchSelected);
    pSearchRow->addWidget(m_pSearchEdit, 1);
    m_pSearchButton = new QPushButton(tr("Search"), this);
    connect(m_pSearchButton, &QPushButton::clicked, this, &DlgFreeMusic::slotSearchSelected);
    pSearchRow->addWidget(m_pSearchButton);
    pLayout->addLayout(pSearchRow);

    for (const auto& source : kFreeMusicSources) {
        QString description = QCoreApplication::translate("DlgFreeMusic", source.description);
        const QString streamingNote =
                QCoreApplication::translate("DlgFreeMusic", source.streamingNote);
        if (!streamingNote.isEmpty()) {
            description += QStringLiteral(" · ") + streamingNote;
        }
        new QListWidgetItem(
                QStringLiteral("%1\n%2")
                        .arg(QCoreApplication::translate("DlgFreeMusic", source.name),
                                description),
                m_pSources);
    }
    m_pSources->setWordWrap(true);
    m_pSources->setMinimumHeight(240);
    connect(m_pSources,
            &QListWidget::itemDoubleClicked,
            this,
            [this](QListWidgetItem*) { slotOpenSelectedSource(); });
    connect(m_pSources, &QListWidget::currentRowChanged, this, &DlgFreeMusic::slotSourceChanged);
    pLayout->addWidget(m_pSources);

    auto* pButtons = new QDialogButtonBox(QDialogButtonBox::Open | QDialogButtonBox::Close, this);
    connect(pButtons->button(QDialogButtonBox::Open),
            &QPushButton::clicked,
            this,
            &DlgFreeMusic::slotOpenSelectedSource);
    connect(pButtons->button(QDialogButtonBox::Close),
            &QPushButton::clicked,
            this,
            &QDialog::close);
    pLayout->addWidget(pButtons);

    m_pSources->setCurrentRow(0);
    slotSourceChanged();
}

void DlgFreeMusic::slotOpenSelectedSource() {
    const auto* pItem = m_pSources->currentItem();
    if (!pItem) {
        return;
    }

    const int sourceIndex = m_pSources->row(pItem);
    if (sourceIndex >= 0 &&
            sourceIndex < static_cast<int>(std::size(kFreeMusicSources))) {
        mixxx::DesktopHelper::openUrl(QUrl(QString::fromUtf8(kFreeMusicSources[sourceIndex].url)));
    }
}

void DlgFreeMusic::slotSearchSelected() {
    const auto* pItem = m_pSources->currentItem();
    if (!pItem) {
        return;
    }
    const int sourceIndex = m_pSources->row(pItem);
    if (sourceIndex < 0 ||
            sourceIndex >= static_cast<int>(std::size(kFreeMusicSources))) {
        return;
    }
    const char* searchUrl = kFreeMusicSources[sourceIndex].searchUrl;
    if (searchUrl[0] == '\0') {
        return;
    }
    const QString query = m_pSearchEdit->text().trimmed();
    if (query.isEmpty()) {
        return;
    }
    QString url = QString::fromUtf8(searchUrl);
    url.replace(QStringLiteral("%s"),
            QString::fromUtf8(QUrl::toPercentEncoding(query)));
    mixxx::DesktopHelper::openUrl(QUrl(url));
}

void DlgFreeMusic::slotSourceChanged() {
    const int sourceIndex = m_pSources->currentRow();
    const bool searchSupported =
            sourceIndex >= 0 &&
            sourceIndex < static_cast<int>(std::size(kFreeMusicSources)) &&
            kFreeMusicSources[sourceIndex].searchUrl[0] != '\0';
    m_pSearchEdit->setEnabled(searchSupported);
    m_pSearchButton->setEnabled(searchSupported);
    m_pSearchEdit->setToolTip(searchSupported
            ? tr("Open the search results for the selected source in your web browser.")
            : tr("This source does not support direct search. Open it to browse instead."));
}
