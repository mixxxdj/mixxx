#include "window.h"

#include "examplenode.h"
#include "rendergraph/context.h"
#include "rendergraph/engine.h"

Window::Window() {
    resize(640, 480);
}

void Window::closeEvent(QCloseEvent*) {
    // since this is the only and last window, we need to cleanup before destruction,
    // because at destruction the context can't be used anymore
    m_pEngine.reset();
}

void Window::initializeGL() {
    rendergraph::Context context;

    auto node = std::make_unique<rendergraph::ExampleNode>(&context);
    m_pEngine = std::make_unique<rendergraph::Engine>(std::move(node));
}

void Window::resizeGL(int w, int h) {
    m_pEngine->resize(w, h);
}

void Window::paintGL() {
    glClearColor(0.f, 0.f, 1.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    // qt scene graph uses premultiplied alpha color in the shader,
    // so we need to do the same
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    m_pEngine->render();
}
