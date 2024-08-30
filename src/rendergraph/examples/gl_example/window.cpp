#include "window.h"

#include "examplenodes.h"
#include "rendergraph/context.h"
#include "rendergraph/opengl/graph.h"

Window::Window() {
}

Window::~Window() = default;

void Window::closeEvent(QCloseEvent*) {
    // since this is the only and last window, we need to cleanup before destruction,
    // because at destruction the context can't be used anymore
    m_rendergraph.reset();
}

void Window::initializeGL() {
    auto node = std::make_unique<rendergraph::Node>();
    node->appendChildNode(std::make_unique<rendergraph::ExampleNode1>());
    node->appendChildNode(std::make_unique<rendergraph::ExampleNode2>());
    node->appendChildNode(std::make_unique<rendergraph::ExampleNode3>());

    {
        QImage img(":/example/images/test.png");
        rendergraph::Context context;
        static_cast<rendergraph::ExampleNode3*>(node->lastChild())
                ->setTexture(
                        std::make_unique<rendergraph::Texture>(context, img));
    }

    m_rendergraph = std::make_unique<rendergraph::Graph>(std::move(node));
    m_rendergraph->initialize();
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

    m_rendergraph->render();
}
