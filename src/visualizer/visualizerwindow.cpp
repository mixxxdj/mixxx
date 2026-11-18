#include "visualizer/visualizerwindow.h"

#include <QCloseEvent>
#include <QKeyEvent>
#include <QOpenGLWindow>
#include <QVector2D>
#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

#include "mixer/basetrackplayer.h"
#include "mixer/playermanager.h"
#include "moc_visualizerwindow.cpp"
#include "util/assert.h"
#include "visualizer/visualizercontrols.h"
#include "visualizer/visualizerdeck.h"
#include "waveform/waveformwidgetfactory.h"

namespace mixxx {

namespace {

const QString kVisualizerGroup = QStringLiteral("[Visualizer]");
const QString kAppGroup = QStringLiteral("[App]");

constexpr int kDefaultWidth = 960;
constexpr int kDefaultHeight = 540;

// [AI-generated] A longer gap between two frames, for example while the
// window was hidden, must not make the animation jump. [/AI-generated]
constexpr double kMaxFrameSeconds = 0.1;

// [AI-generated] The speed of the animation phases in radians per second.
// [/AI-generated]
constexpr double kSlowPhaseSpeed = 0.7;
constexpr double kFastPhaseSpeed = 4.0;

// [AI-generated] The share of the window that the cover takes up along the
// axis in which it fits tightest. [/AI-generated]
constexpr float kCoverScale = 0.8f;

constexpr int kCoverTextureUnit = 0;
constexpr int kBlurredCoverTextureUnit = 1;

float advancedPhase(float phase, double speed, double elapsedSeconds) {
    return static_cast<float>(
            std::fmod(phase + speed * elapsedSeconds, 2 * std::numbers::pi));
}

} // namespace

VisualizerWindow::VisualizerWindow(std::shared_ptr<PlayerManager> pPlayerManager)
        : WGLWidget(nullptr),
          m_pPlayerManager(std::move(pPlayerManager)),
          m_bassIntensity(kVisualizerGroup, QStringLiteral("bass_intensity")),
          m_midIntensity(kVisualizerGroup, QStringLiteral("mid_intensity")),
          m_highIntensity(kVisualizerGroup, QStringLiteral("high_intensity")),
          m_style(kVisualizerGroup, QStringLiteral("style")),
          m_showWindow(kVisualizerGroup, QStringLiteral("show_window")),
          m_bandMetersEnabled(kAppGroup, QStringLiteral("band_meters_enabled")),
          m_slowPhase(0.0f),
          m_fastPhase(0.0f) {
    setWindowTitle(tr("Mixxx Visualizer"));
    resize(kDefaultWidth, kDefaultHeight);
    // [AI-generated] Mixxx quits when its main window is closed, even if
    // this window is still open. [/AI-generated]
    setAttribute(Qt::WA_QuitOnClose, false);

    connect(m_pPlayerManager.get(),
            &PlayerManager::numberOfDecksChanged,
            this,
            &VisualizerWindow::slotNumberOfDecksChanged);
    slotNumberOfDecksChanged(m_pPlayerManager->numberOfDecks());

    WaveformWidgetFactory* pWaveformWidgetFactory = WaveformWidgetFactory::instance();
    connect(pWaveformWidgetFactory,
            &WaveformWidgetFactory::renderSpinnies,
            this,
            &VisualizerWindow::render);
    connect(pWaveformWidgetFactory,
            &WaveformWidgetFactory::swapSpinnies,
            this,
            &VisualizerWindow::swap);
}

VisualizerWindow::~VisualizerWindow() {
    m_bandMetersEnabled.set(0.0);
    makeCurrentIfNeeded();
    for (const auto& pDeckVisual : m_decks) {
        pDeckVisual->m_coverTexture.destroy();
        pDeckVisual->m_blurredCoverTexture.destroy();
    }
    doneCurrent();
}

void VisualizerWindow::render(VSyncThread*) {
    if (!shouldRender()) {
        return;
    }
    double elapsedSeconds = 0.0;
    if (m_frameTimer.running()) {
        elapsedSeconds = std::min(m_frameTimer.restart().toDoubleSeconds(), kMaxFrameSeconds);
    } else {
        m_frameTimer.start();
    }
    updateAnimation(elapsedSeconds);

    makeCurrentIfNeeded();
    paintGL();
    doneCurrent();
}

void VisualizerWindow::swap() {
    if (!shouldRender()) {
        return;
    }
    makeCurrentIfNeeded();
    swapBuffers();
    doneCurrent();
}

void VisualizerWindow::showEvent(QShowEvent* pEvent) {
    m_bandMetersEnabled.set(1.0);
    WGLWidget::showEvent(pEvent);
}

void VisualizerWindow::hideEvent(QHideEvent* pEvent) {
    m_bandMetersEnabled.set(0.0);
    WGLWidget::hideEvent(pEvent);
}

void VisualizerWindow::closeEvent(QCloseEvent* pEvent) {
    m_showWindow.set(0.0);
    pEvent->accept();
}

void VisualizerWindow::mouseDoubleClickEvent(QMouseEvent*) {
    setFullScreen(!isFullScreen());
}

void VisualizerWindow::keyPressEvent(QKeyEvent* pEvent) {
    if (pEvent->key() == Qt::Key_Escape && isFullScreen()) {
        setFullScreen(false);
        return;
    }
    WGLWidget::keyPressEvent(pEvent);
}

void VisualizerWindow::slotNumberOfDecksChanged(int numberOfDecks) {
    for (int deckIndex = static_cast<int>(m_decks.size());
            deckIndex < numberOfDecks;
            ++deckIndex) {
        BaseTrackPlayer* pPlayer = m_pPlayerManager->getDeckBase(deckIndex);
        VERIFY_OR_DEBUG_ASSERT(pPlayer) {
            return;
        }
        auto pDeckVisual = std::make_unique<DeckVisual>();
        pDeckVisual->m_pDeck = make_parented<VisualizerDeck>(pPlayer, this);
        pDeckVisual->m_uploadedCoverRevision = -1;
        pDeckVisual->m_weight = 0.0f;
        m_decks.push_back(std::move(pDeckVisual));
    }
}

void VisualizerWindow::initializeGL() {
    initializeOpenGLFunctions();
    m_coverDistortionShader.init();
    for (const auto& pDeckVisual : m_decks) {
        pDeckVisual->m_uploadedCoverRevision = -1;
    }
}

void VisualizerWindow::paintGL() {
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    switch (static_cast<VisualizerStyle>(static_cast<int>(m_style.get()))) {
    case VisualizerStyle::CoverDistortion:
    default:
        paintCoverDistortion();
        break;
    }
}

void VisualizerWindow::updateAnimation(double elapsedSeconds) {
    m_slowPhase = advancedPhase(m_slowPhase, kSlowPhaseSpeed, elapsedSeconds);
    m_fastPhase = advancedPhase(m_fastPhase, kFastPhaseSpeed, elapsedSeconds);

    const double bassIntensity = m_bassIntensity.get();
    const double midIntensity = m_midIntensity.get();
    const double highIntensity = m_highIntensity.get();

    float totalLoudness = 0.0f;
    for (const auto& pDeckVisual : m_decks) {
        pDeckVisual->m_pDeck->update(elapsedSeconds);
        totalLoudness += pDeckVisual->m_pDeck->loudness();
    }
    for (const auto& pDeckVisual : m_decks) {
        const VisualizerDeck& deck = *pDeckVisual->m_pDeck;
        pDeckVisual->m_weight = VisualizerDeck::mixWeight(deck.loudness(), totalLoudness);
        pDeckVisual->m_amounts = QVector3D(
                VisualizerDeck::bandAmount(bassIntensity, deck.bassLevel()),
                VisualizerDeck::bandAmount(midIntensity, deck.midLevel()),
                VisualizerDeck::bandAmount(highIntensity, deck.highLevel()));
    }
}

void VisualizerWindow::paintCoverDistortion() {
    // [AI-generated] The pictures of the decks are added up, each multiplied
    // by its weight. The weights sum up to 1 at most, which cross-fades
    // between the decks. [/AI-generated]
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    const std::array<float, 8> positions = {-1.f, -1.f, 1.f, -1.f, -1.f, 1.f, 1.f, 1.f};
    const std::array<float, 8> texcoords = {0.f, 1.f, 1.f, 1.f, 0.f, 0.f, 1.f, 0.f};

    m_coverDistortionShader.bind();
    const int positionLocation = m_coverDistortionShader.positionLocation();
    const int texcoordLocation = m_coverDistortionShader.texcoordLocation();
    m_coverDistortionShader.enableAttributeArray(positionLocation);
    m_coverDistortionShader.enableAttributeArray(texcoordLocation);
    m_coverDistortionShader.setAttributeArray(
            positionLocation, GL_FLOAT, positions.data(), 2);
    m_coverDistortionShader.setAttributeArray(
            texcoordLocation, GL_FLOAT, texcoords.data(), 2);
    m_coverDistortionShader.setUniformValue(
            m_coverDistortionShader.textureLocation(), kCoverTextureUnit);
    m_coverDistortionShader.setUniformValue(
            m_coverDistortionShader.blurredTextureLocation(), kBlurredCoverTextureUnit);
    m_coverDistortionShader.setUniformValue(
            m_coverDistortionShader.phasesLocation(),
            QVector2D(m_slowPhase, m_fastPhase));

    const float windowAspect = height() > 0
            ? static_cast<float>(width()) / static_cast<float>(height())
            : 1.0f;
    m_coverDistortionShader.setUniformValue(
            m_coverDistortionShader.windowAspectLocation(), windowAspect);

    for (const auto& pDeckVisual : m_decks) {
        const VisualizerDeck& deck = *pDeckVisual->m_pDeck;
        OpenGLTexture2D& texture = pDeckVisual->m_coverTexture;
        OpenGLTexture2D& blurredTexture = pDeckVisual->m_blurredCoverTexture;
        if (pDeckVisual->m_uploadedCoverRevision != deck.coverRevision()) {
            texture.setData(deck.cover());
            blurredTexture.setData(deck.blurredCover());
            pDeckVisual->m_uploadedCoverRevision = deck.coverRevision();
        }
        if (!texture.isStorageAllocated() ||
                !blurredTexture.isStorageAllocated() ||
                pDeckVisual->m_weight <= 0.0f) {
            continue;
        }

        // [AI-generated] The cover is scaled to fit into the window with a
        // margin around it. Its blurred copy is scaled to fill the window.
        // [/AI-generated]
        const float coverAspect = static_cast<float>(texture.width()) /
                static_cast<float>(texture.height());
        const QVector2D fit = QVector2D(std::max(windowAspect / coverAspect, 1.0f),
                                      std::max(coverAspect / windowAspect, 1.0f)) /
                kCoverScale;
        const QVector2D backgroundFit(std::min(windowAspect / coverAspect, 1.0f),
                std::min(coverAspect / windowAspect, 1.0f));

        m_coverDistortionShader.setUniformValue(m_coverDistortionShader.fitLocation(), fit);
        m_coverDistortionShader.setUniformValue(
                m_coverDistortionShader.backgroundFitLocation(), backgroundFit);
        m_coverDistortionShader.setUniformValue(
                m_coverDistortionShader.flowPhaseLocation(), deck.flowPhase());
        m_coverDistortionShader.setUniformValue(
                m_coverDistortionShader.coverAspectLocation(), coverAspect);
        m_coverDistortionShader.setUniformValue(
                m_coverDistortionShader.amountsLocation(), pDeckVisual->m_amounts);
        m_coverDistortionShader.setUniformValue(
                m_coverDistortionShader.alphaLocation(), pDeckVisual->m_weight);

        blurredTexture.bind(kBlurredCoverTextureUnit);
        texture.bind(kCoverTextureUnit);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        texture.release(kCoverTextureUnit);
        blurredTexture.release(kBlurredCoverTextureUnit);
    }

    m_coverDistortionShader.disableAttributeArray(positionLocation);
    m_coverDistortionShader.disableAttributeArray(texcoordLocation);
    m_coverDistortionShader.release();
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_BLEND);
}

void VisualizerWindow::setFullScreen(bool fullScreen) {
    if (fullScreen) {
        showFullScreen();
    } else {
        showNormal();
    }
    // [AI-generated] The mouse pointer would distract on a projector.
    // [/AI-generated]
    const Qt::CursorShape cursorShape = fullScreen ? Qt::BlankCursor : Qt::ArrowCursor;
    setCursor(cursorShape);
    QOpenGLWindow* pOpenGLWindow = getOpenGLWindow();
    if (pOpenGLWindow) {
        pOpenGLWindow->setCursor(cursorShape);
    }
}

} // namespace mixxx
