#include <QtNodes/DataFlowGraphModel>
#include <QtNodes/DataFlowGraphicsScene>
#include <QtNodes/GraphicsView>
#include <QtNodes/NodeData>
#include <QtNodes/NodeDelegateModelRegistry>

#include <QtGui/QScreen>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>

#include "gst_nodes.hpp"
#include <gst/gst.h>

using QtNodes::DataFlowGraphModel;
using QtNodes::DataFlowGraphicsScene;
using QtNodes::GraphicsView;
using QtNodes::NodeDelegateModelRegistry;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Initialize GStreamer
    gst_init(&argc, &argv);

    // Main window setup
    QMainWindow window;
    window.setWindowTitle("GStreamer Pipeline Editor - Embedded Camera Tool");
    window.resize(1400, 900);

    // Central widget
    QWidget *central = new QWidget(&window);
    QVBoxLayout *layout = new QVBoxLayout(central);

    // Create registry and register nodes
    auto registry = std::make_shared<NodeDelegateModelRegistry>();
    registry->registerModel<RTSPSourceNode>("Sources");
    registry->registerModel<CameraSourceNode>("Sources");
    registry->registerModel<H264EncoderNode>("Encoders");
    registry->registerModel<RTMPSinkNode>("Sinks");

    // Create graph model and scene
    auto model = std::make_shared<DataFlowGraphModel>(registry);
    auto scene = new DataFlowGraphicsScene(*model, &window);
    auto view = new GraphicsView(scene);

    layout->addWidget(view);
    window.setCentralWidget(central);

    // Status bar
    window.statusBar()->showMessage("Ready - Right-click to add nodes for your camera pipeline");

    window.show();

    return app.exec();
}

// Include the MOC file for Qt's meta-object system
#include "main.moc"
