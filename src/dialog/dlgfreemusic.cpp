#include "dialog/dlgfreemusic.h"

#include <iterator>

#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QLabel>
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
};

constexpr FreeMusicSource kFreeMusicSources[] = {
        {QT_TRANSLATE_NOOP("DlgFreeMusic", "YouTube Audio Library"),
                QT_TRANSLATE_NOOP("DlgFreeMusic",
                        "Free music and sound effects offered by YouTube."),
                "https://www.youtube.com/audiolibrary"},
        {QT_TRANSLATE_NOOP("DlgFreeMusic", "Jamendo"),
                QT_TRANSLATE_NOOP("DlgFreeMusic",
                        "Independent music with licensing information for creators."),
                "https://www.jamendo.com/"},
        {QT_TRANSLATE_NOOP("DlgFreeMusic", "Free Music Archive"),
                QT_TRANSLATE_NOOP("DlgFreeMusic",
                        "Curated music with license details on each track."),
                "https://freemusicarchive.org/"},
        {QT_TRANSLATE_NOOP("DlgFreeMusic", "Internet Archive Audio"),
                QT_TRANSLATE_NOOP(
                        "DlgFreeMusic", "Public-domain and openly licensed audio collections."),
                "https://archive.org/details/audio"},
        {QT_TRANSLATE_NOOP("DlgFreeMusic", "ccMixter"),
                QT_TRANSLATE_NOOP(
                        "DlgFreeMusic", "Creative Commons remixes, samples, and instrumentals."),
                "https://ccmixter.org/"},
        {QT_TRANSLATE_NOOP("DlgFreeMusic", "Bandcamp free downloads"),
                QT_TRANSLATE_NOOP(
                        "DlgFreeMusic", "Search releases where the artist enables free downloads."),
                "https://bandcamp.com/tag/free-download"},
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
               "Streaming pages stay outside Mixxx; tracks must be legally downloaded "
               "before they can be loaded into a deck."),
            this);
    pIntro->setWordWrap(true);
    pLayout->addWidget(pIntro);

    for (const auto& source : kFreeMusicSources) {
        auto* pItem = new QListWidgetItem(
                QStringLiteral("%1\n%2")
                        .arg(QCoreApplication::translate("DlgFreeMusic", source.name),
                                QCoreApplication::translate("DlgFreeMusic", source.description)),
                m_pSources);
    }
    m_pSources->setWordWrap(true);
    m_pSources->setMinimumHeight(240);
    connect(m_pSources,
            &QListWidget::itemDoubleClicked,
            this,
            [this](QListWidgetItem*) { slotOpenSelectedSource(); });
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
