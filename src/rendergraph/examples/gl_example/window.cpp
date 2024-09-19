#include "window.h"

#include "examplenodes.h"
#include "rendergraph/context.h"

Window::Window() {
}

void Window::closeEvent(QCloseEvent*) {
    // since this is the only and last window, we need to cleanup before destruction,
    // because at destruction the context can't be used anymore
    m_pEngine.reset();
}

void Window::initializeGL() {
    rendergraph::Context context;

    auto node = std::make_unique<rendergraph::ExampleTopNode>(context);

    m_pEngine = std::make_unique<rendergraph::Engine>(std::move(node));
    m_pEngine->initialize();
}

void Window::resizeGL(int, int) {
}

void Window::paintGL() {
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    // qt scene graph uses premultiplied alpha color in the shader,
    // so we need to do the same
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    m_pEngine->render();
}
