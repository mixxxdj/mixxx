#include "aoide/tracksearchoverlayfilterdlg.h"

#include "moc_tracksearchoverlayfilterdlg.cpp"

namespace {

const QChar kGenreLabelSeparator = QChar{';'};

} // anonymous namespace

namespace aoide {

TrackSearchOverlayFilterDlg::TrackSearchOverlayFilterDlg(
        TrackSearchOverlayFilter overlayFilter,
        QWidget* parent)
        : QDialog(parent),
          m_overlayFilter(std::move(overlayFilter)) {
    setupUi(this);
    init();
    connect(minBpmResetButton,
            &QAbstractButton::clicked,
            this,
            [this](bool checked) {
                Q_UNUSED(checked)
                minBpmDoubleSpinBox->setValue(mixxx::Bpm::kValueUndefined);
            });
    connect(maxBpmResetButton,
            &QAbstractButton::clicked,
            this,
            [this](bool checked) {
                Q_UNUSED(checked)
                maxBpmDoubleSpinBox->setValue(mixxx::Bpm::kValueUndefined);
            });
}

void TrackSearchOverlayFilterDlg::accept() {
    QDialog::accept();
    apply();
}

void TrackSearchOverlayFilterDlg::reject() {
    QDialog::reject();
    reset();
}

void TrackSearchOverlayFilterDlg::init() {
    minBpmDoubleSpinBox->setMinimum(mixxx::Bpm::kValueMin);
    minBpmDoubleSpinBox->setMaximum(mixxx::Bpm::kValueMax);
    minBpmDoubleSpinBox->setDecimals(0);
    minBpmDoubleSpinBox->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
    minBpmDoubleSpinBox->setAccelerated(true);
    maxBpmDoubleSpinBox->setMinimum(mixxx::Bpm::kValueMin);
    maxBpmDoubleSpinBox->setMaximum(mixxx::Bpm::kValueMax);
    maxBpmDoubleSpinBox->setDecimals(0);
    maxBpmDoubleSpinBox->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
    maxBpmDoubleSpinBox->setAccelerated(true);
    reset();
}

void TrackSearchOverlayFilterDlg::reset() {
    minBpmDoubleSpinBox->setValue(m_overlayFilter.minBpm.valueOr(mixxx::Bpm::kValueUndefined));
    maxBpmDoubleSpinBox->setValue(m_overlayFilter.maxBpm.valueOr(mixxx::Bpm::kValueUndefined));
    genreTextLineEdit->setText(mixxx::library::tags::joinLabelsAsText(
            m_overlayFilter.anyGenreLabels, kGenreLabelSeparator));
    commentTextLineEdit->setText(mixxx::library::tags::joinLabelsAsText(
            m_overlayFilter.allCommentTerms));
}

void TrackSearchOverlayFilterDlg::apply() {
    if (minBpmDoubleSpinBox->hasAcceptableInput()) {
        m_overlayFilter.minBpm = mixxx::Bpm(minBpmDoubleSpinBox->value());
    }
    if (maxBpmDoubleSpinBox->hasAcceptableInput()) {
        m_overlayFilter.maxBpm = mixxx::Bpm(maxBpmDoubleSpinBox->value());
    }
    m_overlayFilter.anyGenreLabels = mixxx::library::tags::splitTextIntoLabels(
            genreTextLineEdit->text(), kGenreLabelSeparator);
    m_overlayFilter.allCommentTerms =
            mixxx::library::tags::splitTextIntoLabelsAtWhitespace(
                    commentTextLineEdit->text());
    reset();
}

} // namespace aoide
