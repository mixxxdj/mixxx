#pragma once

#include <QFutureWatcher>
#include <QOpenGLFunctions>
#include <QVector3D>
#include <memory>
#include <vector>

#include "control/pollingcontrolproxy.h"
#include "shaders/covercombineshader.h"
#include "shaders/coverdistortionshader.h"
#include "util/opengltexture2d.h"
#include "util/parented_ptr.h"
#include "util/performancetimer.h"
#include "visualizer/visualizerdeck.h"
#include "visualizer/visualizerlayout.h"
#include "widget/wglwidget.h"

class PlayerManager;
class VSyncThread;
QT_FORWARD_DECLARE_CLASS(QOpenGLFramebufferObject);

namespace mixxx {

/// A separate window that shows nothing but visuals driven by
/// the music of the decks, intended for a second screen or a projector. It
/// has no controls of its own. The style and the intensities are set with
/// the controls of the group [Visualizer], see VisualizerControls. A double
/// click toggles full screen.
class VisualizerWindow : public WGLWidget, private QOpenGLFunctions {
    Q_OBJECT
  public:
    explicit VisualizerWindow(std::shared_ptr<PlayerManager> pPlayerManager);
    ~VisualizerWindow() override;

    /// Sets the image file that the style CustomImage shows
    /// in place of the covers. The file is loaded in a worker thread. With
    /// an empty path, or if the file cannot be loaded, that style shows the
    /// covers.
    void setCustomImage(const QString& path);

  public slots:
    /// Advances the animation and draws the next frame into
    /// the back buffer. Connected to the frame clock of the waveforms.
    void render(VSyncThread* pVSyncThread);
    /// Shows the frame drawn by render().
    void swap();

  protected:
    void showEvent(QShowEvent* pEvent) override;
    void hideEvent(QHideEvent* pEvent) override;
    void closeEvent(QCloseEvent* pEvent) override;
    void mouseDoubleClickEvent(QMouseEvent* pEvent) override;
    void keyPressEvent(QKeyEvent* pEvent) override;

  private slots:
    void slotNumberOfDecksChanged(int numberOfDecks);
    void slotCustomImageLoaded();

  private:
    struct DeckVisual {
        parented_ptr<VisualizerDeck> m_pDeck;
        OpenGLTexture2D m_coverTexture;
        OpenGLTexture2D m_blurredCoverTexture;
        int m_uploadedCoverRevision;
    };

    void initializeGL() override;
    void paintGL() override;

    void updateAnimation(double elapsedSeconds);
    void paintCoverDistortion();
    void paintCustomImage();
    void drawDistorted(GLuint textureId, GLuint blurredTextureId, float aspect);
    void combineCovers(QOpenGLFramebufferObject* pTarget, bool blurred);
    void setFullScreen(bool fullScreen);
    void setDecksActive(bool active);
    bool shouldDraw() const;

    static VisualizerDeck::PreparedCover loadCustomImage(int request, QString path);

    std::shared_ptr<PlayerManager> m_pPlayerManager;
    std::vector<std::unique_ptr<DeckVisual>> m_decks;
    VisualizerLayout m_layout;
    VisualizerLayout::PerDeck m_shares;

    PollingControlProxy m_bassIntensity;
    PollingControlProxy m_midIntensity;
    PollingControlProxy m_highIntensity;
    PollingControlProxy m_style;
    PollingControlProxy m_showWindow;
    PollingControlProxy m_bandMetersEnabled;

    CoverCombineShader m_coverCombineShader;
    CoverDistortionShader m_coverDistortionShader;
    std::unique_ptr<QOpenGLFramebufferObject> m_pCombinedCover;
    std::unique_ptr<QOpenGLFramebufferObject> m_pCombinedBlurredCover;
    QVector3D m_amounts;

    QFutureWatcher<VisualizerDeck::PreparedCover> m_customImageWatcher;
    int m_customImageRequest;
    QImage m_customImage;
    QImage m_blurredCustomImage;
    bool m_customImageChanged;
    OpenGLTexture2D m_customImageTexture;
    OpenGLTexture2D m_blurredCustomImageTexture;

    PerformanceTimer m_frameTimer;
    float m_slowPhase;
    float m_fastPhase;
    float m_flowPhase;
};

} // namespace mixxx
