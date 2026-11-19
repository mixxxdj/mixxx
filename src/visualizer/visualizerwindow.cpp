#include "visualizer/visualizerwindow.h"

#include <QCloseEvent>
#include <QKeyEvent>
#include <QOpenGLFramebufferObject>
#include <QOpenGLWindow>
#include <QVector2D>
#include <QtConcurrentRun>
#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

#include "engine/enginebandmeter.h"
#include "mixer/basetrackplayer.h"
#include "mixer/playermanager.h"
#include "moc_visualizerwindow.cpp"
#include "util/assert.h"
#include "visualizer/visualizercontrols.h"
#include "waveform/waveformwidgetfactory.h"

namespace mixxx {

namespace {

using namespace std::chrono_literals;

constexpr int kDefaultWidth = 960;
constexpr int kDefaultHeight = 540;

// A longer gap between two frames, for example while the window was hidden,
// must not make the animation jump.
constexpr std::chrono::duration<double> kMaxFrameTime = 100ms;

// The speed of the animation phases in radians per second.
constexpr double kSlowPhaseSpeed = 0.7;
constexpr double kFastPhaseSpeed = 4.0;

// The share of the window that the cover takes up along the axis in which it
// fits tightest.
constexpr float kCoverScale = 0.8f;

// The covers of the audible decks are combined into one square picture of this
// size, and their blurred copies into a small one.
constexpr int kCombinedCoverSize = 1024;
constexpr int kCombinedBlurredCoverSize = 64;

// In the combined blurred picture the borders between the covers fade over this
// share of the picture, which is about as wide as the blur of the covers
// themselves.
constexpr float kBlurredBorderWidth = 0.2f;

constexpr int kCoverTextureUnit = 0;
constexpr int kBlurredCoverTextureUnit = 1;

float advancedPhase(float phase, double speed, std::chrono::duration<double> elapsed) {
    return static_cast<float>(
            std::fmod(phase + speed * elapsed.count(), 2 * std::numbers::pi));
}

} // namespace

VisualizerWindow::VisualizerWindow(std::shared_ptr<PlayerManager> pPlayerManager)
        : WGLWidget(nullptr),
          m_pPlayerManager(std::move(pPlayerManager)),
          m_shares(),
          m_bassIntensity(kVisualizerBassIntensityConfigKey),
          m_midIntensity(kVisualizerMidIntensityConfigKey),
          m_highIntensity(kVisualizerHighIntensityConfigKey),
          m_style(kVisualizerStyleConfigKey),
          m_showWindow(kVisualizerShowWindowConfigKey),
          m_customImageRequest(0),
          m_customImageChanged(false),
          m_slowPhase(0.0f),
          m_fastPhase(0.0f),
          m_flowPhase(0.0f) {
    connect(&m_customImageWatcher,
            &QFutureWatcher<VisualizerDeck::PreparedCover>::finished,
            this,
            &VisualizerWindow::slotCustomImageLoaded);
    setWindowTitle(tr("Mixxx Visualizer"));
    resize(kDefaultWidth, kDefaultHeight);
    // Mixxx quits when its main window is closed, even if this window is still
    // open.
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
    makeCurrentIfNeeded();
    for (const auto& pDeckVisual : m_decks) {
        pDeckVisual->m_coverTexture.destroy();
        pDeckVisual->m_blurredCoverTexture.destroy();
    }
    m_customImageTexture.destroy();
    m_blurredCustomImageTexture.destroy();
    m_pCombinedCover.reset();
    m_pCombinedBlurredCover.reset();
    doneCurrent();
}

void VisualizerWindow::setCustomImage(const QString& path) {
    // The new request number invalidates an image that is still being loaded.
    ++m_customImageRequest;
    if (path.isEmpty()) {
        m_customImage = QImage();
        m_blurredCustomImage = QImage();
        m_customImageChanged = true;
        return;
    }
    m_customImageWatcher.setFuture(QtConcurrent::run(
            &VisualizerWindow::loadCustomImage, m_customImageRequest, path));
}

// static
VisualizerDeck::PreparedCover VisualizerWindow::loadCustomImage(int request, QString path) {
    QImage image(path);
    if (image.isNull()) {
        qWarning() << "Could not load the image for the visualizer:" << path;
        return VisualizerDeck::PreparedCover{request, QImage(), QImage()};
    }
    // Converting from the premultiplied format puts the transparent parts of
    // the image on a black background.
    image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied)
                    .convertToFormat(QImage::Format_RGB32);
    return VisualizerDeck::prepareCover(request, image);
}

void VisualizerWindow::slotCustomImageLoaded() {
    const VisualizerDeck::PreparedCover loaded = m_customImageWatcher.result();
    if (loaded.m_request != m_customImageRequest) {
        return;
    }
    m_customImage = loaded.m_cover;
    m_blurredCustomImage = loaded.m_blurredCover;
    m_customImageChanged = true;
}

void VisualizerWindow::render(VSyncThread*) {
    if (!shouldDraw()) {
        return;
    }
    std::chrono::duration<double> elapsed = 0s;
    if (m_frameTimer.running()) {
        elapsed = std::min<std::chrono::duration<double>>(
                m_frameTimer.restart().toStdDuration(), kMaxFrameTime);
    } else {
        m_frameTimer.start();
    }
    updateAnimation(elapsed);

    makeCurrentIfNeeded();
    paintGL();
    doneCurrent();
}

void VisualizerWindow::swap() {
    if (!shouldDraw()) {
        return;
    }
    makeCurrentIfNeeded();
    swapBuffers();
    doneCurrent();
}

void VisualizerWindow::showEvent(QShowEvent* pEvent) {
    m_pBandMeterRequest = std::make_unique<EngineBandMeterRequest>();
    setDecksActive(true);
    WGLWidget::showEvent(pEvent);
}

void VisualizerWindow::hideEvent(QHideEvent* pEvent) {
    m_pBandMeterRequest.reset();
    setDecksActive(false);
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
    VERIFY_OR_DEBUG_ASSERT(numberOfDecks <= kMaxNumberOfDecks) {
        numberOfDecks = kMaxNumberOfDecks;
    }
    for (int deckIndex = static_cast<int>(m_decks.size());
            deckIndex < numberOfDecks;
            ++deckIndex) {
        BaseTrackPlayer* pPlayer = m_pPlayerManager->getDeckBase(deckIndex);
        VERIFY_OR_DEBUG_ASSERT(pPlayer) {
            return;
        }
        auto pDeckVisual = std::make_unique<DeckVisual>();
        pDeckVisual->m_pDeck = make_parented<VisualizerDeck>(pPlayer, this);
        pDeckVisual->m_pDeck->setActive(isVisible());
        pDeckVisual->m_uploadedCoverRevision = -1;
        m_decks.push_back(std::move(pDeckVisual));
    }
}

void VisualizerWindow::initializeGL() {
    initializeOpenGLFunctions();
    m_coverCombineShader.init();
    m_coverDistortionShader.init();
    m_pCombinedCover.reset();
    m_pCombinedBlurredCover.reset();
    m_customImageChanged = true;
    for (const auto& pDeckVisual : m_decks) {
        pDeckVisual->m_uploadedCoverRevision = -1;
    }
}

void VisualizerWindow::paintGL() {
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    switch (VisualizerControls::styleFromControlValue(m_style.get())) {
    case VisualizerControls::Style::CustomImage:
        paintCustomImage();
        break;
    case VisualizerControls::Style::CoverDistortion:
        paintCoverDistortion();
        break;
    }
}

void VisualizerWindow::updateAnimation(std::chrono::duration<double> elapsed) {
    m_slowPhase = advancedPhase(m_slowPhase, kSlowPhaseSpeed, elapsed);
    m_fastPhase = advancedPhase(m_fastPhase, kFastPhaseSpeed, elapsed);

    const double bassIntensity = m_bassIntensity.get();
    const double midIntensity = m_midIntensity.get();
    const double highIntensity = m_highIntensity.get();

    // The decks are treated as one source of music: their band levels add up,
    // limited to the level of a single deck at full volume, and the tempo is
    // the tempo of the loudest deck.
    VisualizerLayout::PerDeck loudness{};
    float bassLevel = 0.0f;
    float midLevel = 0.0f;
    float highLevel = 0.0f;
    for (std::size_t deckIndex = 0; deckIndex < m_decks.size(); ++deckIndex) {
        VisualizerDeck& deck = *m_decks[deckIndex]->m_pDeck;
        deck.update(elapsed);
        loudness[deckIndex] = deck.loudness();
        bassLevel += deck.bassLevel();
        midLevel += deck.midLevel();
        highLevel += deck.highLevel();
    }
    m_amounts = QVector3D(
            VisualizerDeck::bandAmount(bassIntensity, std::min(bassLevel, 1.0f)),
            VisualizerDeck::bandAmount(midIntensity, std::min(midLevel, 1.0f)),
            VisualizerDeck::bandAmount(highIntensity, std::min(highLevel, 1.0f)));

    m_shares = VisualizerLayout::updatedShares(loudness, m_shares);
    m_layout.update(m_shares,
            elapsed,
            QSizeF(kCombinedCoverSize, kCombinedCoverSize));

    double bpm = 0.0;
    float largestShare = 0.0f;
    for (std::size_t deckIndex = 0; deckIndex < m_decks.size(); ++deckIndex) {
        if (m_shares[deckIndex] > largestShare) {
            largestShare = m_shares[deckIndex];
            bpm = m_decks[deckIndex]->m_pDeck->bpm();
        }
    }
    m_flowPhase = VisualizerDeck::advancedFlowPhase(m_flowPhase, bpm, elapsed);
}

void VisualizerWindow::paintCoverDistortion() {
    bool hasCover = false;
    for (std::size_t deckIndex = 0; deckIndex < m_decks.size(); ++deckIndex) {
        const auto& pDeckVisual = m_decks[deckIndex];
        const VisualizerDeck& deck = *pDeckVisual->m_pDeck;
        if (pDeckVisual->m_uploadedCoverRevision != deck.coverRevision()) {
            pDeckVisual->m_coverTexture.setData(deck.cover());
            pDeckVisual->m_blurredCoverTexture.setData(deck.blurredCover());
            pDeckVisual->m_uploadedCoverRevision = deck.coverRevision();
        }
        hasCover = hasCover ||
                (m_layout.cell(static_cast<int>(deckIndex)).m_area > 0.0f &&
                        pDeckVisual->m_coverTexture.isStorageAllocated());
    }
    if (!hasCover) {
        return;
    }

    if (!m_pCombinedCover) {
        m_pCombinedCover = std::make_unique<QOpenGLFramebufferObject>(
                kCombinedCoverSize, kCombinedCoverSize);
        m_pCombinedBlurredCover = std::make_unique<QOpenGLFramebufferObject>(
                kCombinedBlurredCoverSize, kCombinedBlurredCoverSize);
    }

    // First combine the covers of the audible decks into one picture and their
    // blurred copies into another one. Then draw these two like the cover of a
    // single deck.
    std::array<GLint, 4> viewport;
    glGetIntegerv(GL_VIEWPORT, viewport.data());
    combineCovers(m_pCombinedCover.get(), false);
    combineCovers(m_pCombinedBlurredCover.get(), true);
    QOpenGLFramebufferObject::bindDefault();
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

    // The combined cover is a square.
    drawDistorted(m_pCombinedCover->texture(), m_pCombinedBlurredCover->texture(), 1.0f);
}

void VisualizerWindow::paintCustomImage() {
    if (m_customImageChanged) {
        m_customImageTexture.setData(m_customImage);
        m_blurredCustomImageTexture.setData(m_blurredCustomImage);
        m_customImageChanged = false;
    }
    if (!m_customImageTexture.isStorageAllocated() ||
            !m_blurredCustomImageTexture.isStorageAllocated()) {
        // No image has been selected, or it could not be loaded.
        paintCoverDistortion();
        return;
    }
    drawDistorted(m_customImageTexture.textureId(),
            m_blurredCustomImageTexture.textureId(),
            static_cast<float>(m_customImageTexture.width()) /
                    static_cast<float>(m_customImageTexture.height()));
}

void VisualizerWindow::drawDistorted(GLuint textureId, GLuint blurredTextureId, float aspect) {
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

    // The picture is scaled to fit into the window with a margin around it, and
    // its blurred copy is scaled to fill the window.
    const float windowAspect = height() > 0
            ? static_cast<float>(width()) / static_cast<float>(height())
            : 1.0f;
    const QVector2D fit = QVector2D(std::max(windowAspect / aspect, 1.0f),
                                  std::max(aspect / windowAspect, 1.0f)) /
            kCoverScale;
    const QVector2D backgroundFit(std::min(windowAspect / aspect, 1.0f),
            std::min(aspect / windowAspect, 1.0f));

    m_coverDistortionShader.setUniformValue(
            m_coverDistortionShader.textureLocation(), kCoverTextureUnit);
    m_coverDistortionShader.setUniformValue(
            m_coverDistortionShader.blurredTextureLocation(), kBlurredCoverTextureUnit);
    m_coverDistortionShader.setUniformValue(m_coverDistortionShader.fitLocation(), fit);
    m_coverDistortionShader.setUniformValue(
            m_coverDistortionShader.backgroundFitLocation(), backgroundFit);
    m_coverDistortionShader.setUniformValue(
            m_coverDistortionShader.windowAspectLocation(), windowAspect);
    m_coverDistortionShader.setUniformValue(
            m_coverDistortionShader.coverAspectLocation(), aspect);
    m_coverDistortionShader.setUniformValue(
            m_coverDistortionShader.amountsLocation(), m_amounts);
    m_coverDistortionShader.setUniformValue(
            m_coverDistortionShader.phasesLocation(),
            QVector2D(m_slowPhase, m_fastPhase));
    m_coverDistortionShader.setUniformValue(
            m_coverDistortionShader.flowPhaseLocation(), m_flowPhase);

    glActiveTexture(GL_TEXTURE0 + kBlurredCoverTextureUnit);
    glBindTexture(GL_TEXTURE_2D, blurredTextureId);
    glActiveTexture(GL_TEXTURE0 + kCoverTextureUnit);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0 + kBlurredCoverTextureUnit);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);

    m_coverDistortionShader.disableAttributeArray(positionLocation);
    m_coverDistortionShader.disableAttributeArray(texcoordLocation);
    m_coverDistortionShader.release();
}

void VisualizerWindow::combineCovers(QOpenGLFramebufferObject* pTarget, bool blurred) {
    pTarget->bind();
    glViewport(0, 0, pTarget->width(), pTarget->height());
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT);
    // The parts of the covers are added up. Where the borders of their cells
    // are smoothed, neighbor cells overlap.
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    // The texture coordinates are flipped compared to the window, because the
    // first row of a framebuffer texture is its bottom row, while the first row
    // of a cover texture is its top row.
    const std::array<float, 8> positions = {-1.f, -1.f, 1.f, -1.f, -1.f, 1.f, 1.f, 1.f};
    const std::array<float, 8> texcoords = {0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 1.f, 1.f};

    m_coverCombineShader.bind();
    const int positionLocation = m_coverCombineShader.positionLocation();
    const int texcoordLocation = m_coverCombineShader.texcoordLocation();
    m_coverCombineShader.enableAttributeArray(positionLocation);
    m_coverCombineShader.enableAttributeArray(texcoordLocation);
    m_coverCombineShader.setAttributeArray(positionLocation, GL_FLOAT, positions.data(), 2);
    m_coverCombineShader.setAttributeArray(texcoordLocation, GL_FLOAT, texcoords.data(), 2);
    m_coverCombineShader.setUniformValue(m_coverCombineShader.textureLocation(), 0);
    // The borders of the layout measure pixels of the combined cover. They are
    // smoothed over one pixel there.
    m_coverCombineShader.setUniformValue(m_coverCombineShader.sharpnessLocation(),
            blurred ? 1.0f / (kBlurredBorderWidth * kCombinedCoverSize) : 1.0f);

    for (std::size_t deckIndex = 0; deckIndex < m_decks.size(); ++deckIndex) {
        const auto& pDeckVisual = m_decks[deckIndex];
        const VisualizerLayout::Cell& cell = m_layout.cell(static_cast<int>(deckIndex));
        OpenGLTexture2D& texture = blurred
                ? pDeckVisual->m_blurredCoverTexture
                : pDeckVisual->m_coverTexture;
        if (cell.m_area <= 0.0f || !texture.isStorageAllocated()) {
            continue;
        }
        // A cover that is not square is cropped to the square around its
        // center.
        const float coverAspect = static_cast<float>(texture.width()) /
                static_cast<float>(texture.height());
        const QVector2D fit(std::min(1.0f, 1.0f / coverAspect), std::min(1.0f, coverAspect));
        m_coverCombineShader.setUniformValueArray(
                m_coverCombineShader.bordersLocation(),
                cell.m_borders.data(),
                static_cast<int>(cell.m_borders.size()));
        m_coverCombineShader.setUniformValue(m_coverCombineShader.fitLocation(), fit);
        texture.bind();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        texture.release();
    }

    m_coverCombineShader.disableAttributeArray(positionLocation);
    m_coverCombineShader.disableAttributeArray(texcoordLocation);
    m_coverCombineShader.release();
    glDisable(GL_BLEND);

    // The combined picture is drawn enlarged, so it needs linear filtering.
    glBindTexture(GL_TEXTURE_2D, pTarget->texture());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void VisualizerWindow::setDecksActive(bool active) {
    for (const auto& pDeckVisual : m_decks) {
        pDeckVisual->m_pDeck->setActive(active);
    }
}

bool VisualizerWindow::shouldDraw() const {
    // Nobody can see the picture while the window is hidden
    // or minimized. Whether the window system reports the window as exposed
    // must not be used here: a full screen window on a second screen is
    // reported as not exposed when it loses the focus, although it is still
    // in plain sight.
    return shouldRender() && isVisible() && !isMinimized();
}

void VisualizerWindow::setFullScreen(bool fullScreen) {
    if (fullScreen) {
        showFullScreen();
    } else {
        showNormal();
    }
    // The mouse pointer would distract on a projector.
    const Qt::CursorShape cursorShape = fullScreen ? Qt::BlankCursor : Qt::ArrowCursor;
    setCursor(cursorShape);
    QOpenGLWindow* pOpenGLWindow = getOpenGLWindow();
    if (pOpenGLWindow) {
        pOpenGLWindow->setCursor(cursorShape);
    }
}

} // namespace mixxx
