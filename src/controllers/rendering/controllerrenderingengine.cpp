#include "controllers/rendering/controllerrenderingengine.h"

#include <QEvent>
#include <QLabel>
#include <QOffscreenSurface>
#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickGraphicsDevice>
#include <QQuickItem>
#include <QQuickRenderControl>
#include <QQuickRenderTarget>
#include <QQuickWindow>
#include <QScreen>

#include "controllers/controller.h"
#include "controllers/scripting/legacy/controllerscriptenginelegacy.h"
#include "controllers/scripting/legacy/controllerscriptinterfacelegacy.h"
#include "moc_controllerrenderingengine.cpp"
#include "qml/qmlwaveformoverview.h"
#include "util/cmdlineargs.h"
#include "util/time.h"

ControllerRenderingEngine::ControllerRenderingEngine(
        const LegacyControllerMapping::ScreenInfo& info)
        : QObject(),
          m_screenInfo(info),
          m_GLDataFormat(GL_RGBA),
          m_GLDataType(GL_UNSIGNED_BYTE),
          m_isValid(true) {
    switch (m_screenInfo.pixelFormat) {
    case QImage::Format_RGB16:
        m_GLDataFormat = GL_RGB;
        m_GLDataType = m_screenInfo.reversedColor
                ? GL_UNSIGNED_SHORT_5_6_5_REV
                : GL_UNSIGNED_SHORT_5_6_5;
        break;
    case QImage::Format_RGB888:
        m_GLDataFormat = m_screenInfo.reversedColor ? GL_BGR : GL_RGB;
        m_GLDataType = GL_UNSIGNED_BYTE;
        break;
    case QImage::Format_RGBA8888:
        m_GLDataFormat = m_screenInfo.reversedColor ? GL_BGRA : GL_RGBA;
        m_GLDataType = GL_UNSIGNED_BYTE;
        break;
    default:
        m_isValid = false;
        DEBUG_ASSERT(!"Unsupported format");
    }

    if (!m_isValid)
        return;

    prepare();
}

void ControllerRenderingEngine::prepare() {
    m_pThread = std::make_unique<QThread>();
    m_pThread->setObjectName("ControllerScreenRenderer");

    moveToThread(m_pThread.get());
    connect(this,
            &ControllerRenderingEngine::setupRequested,
            this,
            &ControllerRenderingEngine::setup);
    connect(this,
            &ControllerRenderingEngine::sendRequested,
            this,
            &ControllerRenderingEngine::send);
    connect(this,
            &ControllerRenderingEngine::stopRequested,
            this,
            &ControllerRenderingEngine::finish);

    m_pThread->start(QThread::NormalPriority);
}

ControllerRenderingEngine::~ControllerRenderingEngine() {
    VERIFY_OR_DEBUG_ASSERT(!m_fbo) {
        qWarning() << "The ControllerEngine is being deleted but hasn't been "
                      "cleaned up. Brace for impact";
    };
}

void ControllerRenderingEngine::start() {
    VERIFY_OR_DEBUG_ASSERT(!thread()->isFinished() && !thread()->isInterruptionRequested()) {
        qWarning() << "Render thread has or ir about to terminate. Cannot "
                      "start this render anymore.";
        return;
    }
    QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
}

void ControllerRenderingEngine::requestSetup(std::shared_ptr<QQmlEngine> qmlEngine) {
    VERIFY_OR_DEBUG_ASSERT(QThread::currentThread() != thread()) {
        qWarning() << "Unable to setup OpenGL rendering context from the same "
                      "thread as the render object";
        return;
    }
    emit setupRequested(qmlEngine);

    const auto lock = lockMutex(&m_mutex);
    if (!m_quickWindow) {
        m_waitCondition.wait(&m_mutex);
    }
}

void ControllerRenderingEngine::requestSend(Controller* controller, const QByteArray& frame) {
    emit sendRequested(controller, frame);
}

void ControllerRenderingEngine::setup(std::shared_ptr<QQmlEngine> qmlEngine) {
    QSurfaceFormat format;
    format.setDepthBufferSize(16);
    format.setStencilBufferSize(8);

    m_context = std::make_unique<QOpenGLContext>();
    m_context->setFormat(format);
    VERIFY_OR_DEBUG_ASSERT(m_context->create()) {
        qWarning() << "Unable to intiliaze controller screen rendering. Giving up";
        m_isValid = false;
        m_waitCondition.wakeAll();
        finish();
        return;
    }
    connect(m_context.get(),
            &QOpenGLContext::aboutToBeDestroyed,
            this,
            &ControllerRenderingEngine::finish,
            Qt::BlockingQueuedConnection);

    m_offscreenSurface = std::make_unique<QOffscreenSurface>();
    m_offscreenSurface->setFormat(m_context->format());

    QMetaObject::invokeMethod(
            qApp,
            [this] {
                m_offscreenSurface->create();
            },
            // This invocation will block the current thread!
            Qt::BlockingQueuedConnection);

    m_renderControl = std::make_unique<QQuickRenderControl>(this);
    m_quickWindow = std::make_unique<QQuickWindow>(m_renderControl.get());

    if (!qmlEngine->incubationController())
        qmlEngine->setIncubationController(m_quickWindow->incubationController());

    m_quickWindow->setGeometry(0, 0, m_screenInfo.size.width(), m_screenInfo.size.height());

    m_waitCondition.wakeAll();
}

void ControllerRenderingEngine::finish() {
    disconnect(this);

    if (m_context && m_context->isValid()) {
        disconnect(m_context.get(),
                &QOpenGLContext::aboutToBeDestroyed,
                this,
                &ControllerRenderingEngine::finish);
        m_context->makeCurrent(m_offscreenSurface.get());
        m_renderControl.reset();
        m_offscreenSurface.reset();
        m_quickWindow.reset();

        // Free the engine and FBO
        m_fbo.reset();

        m_context->doneCurrent();
    }
    m_context.reset();
    m_pThread->quit();
}

void ControllerRenderingEngine::renderFrame() {
    VERIFY_OR_DEBUG_ASSERT(m_offscreenSurface->isValid()) {
        qWarning() << "OffscrenSurface isn't valid anymore.";
        finish();
        return;
    };
    VERIFY_OR_DEBUG_ASSERT(m_context->isValid()) {
        qWarning() << "GLContext isn't valid anymore.";
        finish();
        return;
    };
    VERIFY_OR_DEBUG_ASSERT(m_context->makeCurrent(m_offscreenSurface.get())) {
        qWarning() << "Couldn't make the GLContext current to the OffscrenSurface.";
        finish();
        return;
    };

    if (!m_fbo) {
        m_fbo = std::make_unique<QOpenGLFramebufferObject>(
                m_screenInfo.size, QOpenGLFramebufferObject::CombinedDepthStencil);

        m_quickWindow->setGraphicsDevice(QQuickGraphicsDevice::fromOpenGLContext(m_context.get()));

        VERIFY_OR_DEBUG_ASSERT(m_renderControl->initialize()) {
            qWarning() << "Failed to initialize redirected Qt Quick rendering";
        };

        m_quickWindow->setRenderTarget(QQuickRenderTarget::fromOpenGLTexture(m_fbo->texture(),
                m_screenInfo.size));

        m_quickWindow->setGeometry(0, 0, m_screenInfo.size.width(), m_screenInfo.size.height());
    }

    m_nextFrameStart = mixxx::Time::elapsed();
    m_renderControl->polishItems();

    m_renderControl->beginFrame();
    VERIFY_OR_DEBUG_ASSERT(m_renderControl->sync()) {
        qWarning() << "Couldn't sync the render control.";
    };
    QImage fboImage(m_screenInfo.size, m_screenInfo.pixelFormat);

    VERIFY_OR_DEBUG_ASSERT(m_fbo->bind()) {
        qWarning() << "Couldn't bind the FBO.";
    }
    GLenum glError;
    m_context->functions()->glFlush();
    glError = m_context->functions()->glGetError();
    VERIFY_OR_DEBUG_ASSERT(glError == GL_NO_ERROR) {
        qWarning() << "GLError: " << glError;
        finish();
    }
    if (m_screenInfo.endian != std::endian::native) {
        m_context->functions()->glPixelStorei(GL_PACK_SWAP_BYTES, GL_TRUE);
    }
    glError = m_context->functions()->glGetError();
    VERIFY_OR_DEBUG_ASSERT(glError == GL_NO_ERROR) {
        qWarning() << "GLError: " << glError;
        finish();
    }

    QDateTime timestamp = QDateTime::currentDateTime();
    m_renderControl->render();
    m_renderControl->endFrame();

    while (m_context->functions()->glGetError())
        ;
    m_context->functions()->glReadPixels(0,
            0,
            m_screenInfo.size.width(),
            m_screenInfo.size.height(),
            m_GLDataFormat,
            m_GLDataType,
            fboImage.bits());
    glError = m_context->functions()->glGetError();
    VERIFY_OR_DEBUG_ASSERT(glError == GL_NO_ERROR) {
        qWarning() << "GLError: " << glError;
        finish();
    }
    VERIFY_OR_DEBUG_ASSERT(!fboImage.isNull()) {
        qWarning() << "Screen frame is null!";
    }
    VERIFY_OR_DEBUG_ASSERT(m_fbo->release()) {
        qDebug() << "Couldn't release the FBO.";
    }

    fboImage.mirror(false, true);

    emit frameRendered(m_screenInfo, fboImage, timestamp);

    m_context->doneCurrent();
}

bool ControllerRenderingEngine::stop() {
    emit stopRequested();
    return m_pThread->wait();
}

void ControllerRenderingEngine::send(Controller* controller, const QByteArray& frame) {
    if (!frame.isEmpty()) {
        controller->sendBytes(frame);
    }

    if (CmdlineArgs::Instance()
                    .getControllerDebug()) {
        auto endOfRender = mixxx::Time::elapsed();
        qDebug() << "Fame took "
                 << (endOfRender - m_nextFrameStart).formatMillisWithUnit()
                 << " and frame has" << frame.size() << "bytes";
    }

    m_nextFrameStart += mixxx::Duration::fromSeconds(1.0 / (double)m_screenInfo.target_fps);

    auto durationToWaitBeforeFrame = (m_nextFrameStart - mixxx::Time::elapsed());
    auto msecToWaitBeforeFrame = durationToWaitBeforeFrame.toIntegerMillis();

    if (msecToWaitBeforeFrame > 0) {
        if (CmdlineArgs::Instance()
                        .getControllerDebug()) {
            qDebug() << "Waiting for "
                     << durationToWaitBeforeFrame.formatMillisWithUnit()
                     << " before rendering next frame";
        }
        QTimer::singleShot(msecToWaitBeforeFrame,
                Qt::PreciseTimer,
                this,
                &ControllerRenderingEngine::renderFrame);
    } else {
        QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    }
}

bool ControllerRenderingEngine::event(QEvent* event) {
    if (event->type() == QEvent::UpdateRequest) {
        renderFrame();
        return true;
    }

    return QObject::event(event);
}
