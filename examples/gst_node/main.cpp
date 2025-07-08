#include <QtNodes/DataFlowGraphModel>
#include <QtNodes/DataFlowGraphicsScene>
#include <QtNodes/GraphicsView>
#include <QtNodes/NodeData>
#include <QtNodes/NodeDelegateModelRegistry>

#include <QtGui/QScreen>
#include <QtGui/QFont>
#include <QtGui/QClipboard>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QWidget>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QLabel>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QDateTime>
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>

#include "gst_nodes.hpp"

using QtNodes::DataFlowGraphModel;
using QtNodes::DataFlowGraphicsScene;
using QtNodes::GraphicsView;
using QtNodes::NodeDelegateModelRegistry;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("GStreamer File Source Pipeline Designer - Embedded Systems Tool");
        resize(1400, 900);
        
        setupUI();
        setupConnections();
        
        // Generate initial JSON
        exportToJson();
    }
    
private:
    void setupUI() {
        // Create central widget with splitter
        auto central = new QWidget(this);
        auto layout = new QHBoxLayout(central);
        
        auto splitter = new QSplitter(Qt::Horizontal);
        
        // Left side - Node editor
        auto leftWidget = new QWidget();
        auto leftLayout = new QVBoxLayout(leftWidget);
        
        // Title
        auto titleLabel = new QLabel("GStreamer File Source Designer");
        titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50; margin: 15px; text-align: center;");
        titleLabel->setAlignment(Qt::AlignCenter);
        leftLayout->addWidget(titleLabel);
        
        auto subtitleLabel = new QLabel("Perfect for Embedded Systems • Python • C++ • C");
        subtitleLabel->setStyleSheet("font-size: 12px; color: #7f8c8d; margin-bottom: 10px; text-align: center;");
        subtitleLabel->setAlignment(Qt::AlignCenter);
        leftLayout->addWidget(subtitleLabel);
        
        // Create registry and register only file source node
        auto registry = std::make_shared<NodeDelegateModelRegistry>();
        registry->registerModel<FileSourceNode>("Sources");
        
        // Create graph model and scene
        _model = std::make_shared<DataFlowGraphModel>(registry);
        auto scene = new DataFlowGraphicsScene(*_model, this);
        _view = new GraphicsView(scene);
        
        leftLayout->addWidget(_view);
        
        // Control buttons
        auto buttonLayout = new QHBoxLayout();
        
        _exportBtn = new QPushButton("Export JSON");
        _exportBtn->setStyleSheet("QPushButton { background-color: #3498db; color: white; font-weight: bold; padding: 12px; border: none; border-radius: 6px; }");
        buttonLayout->addWidget(_exportBtn);
        
        _saveBtn = new QPushButton("Save JSON");
        _saveBtn->setStyleSheet("QPushButton { background-color: #27ae60; color: white; font-weight: bold; padding: 12px; border: none; border-radius: 6px; }");
        buttonLayout->addWidget(_saveBtn);
        
        _copyBtn = new QPushButton("Copy JSON");
        _copyBtn->setStyleSheet("QPushButton { background-color: #e67e22; color: white; font-weight: bold; padding: 12px; border: none; border-radius: 6px; }");
        buttonLayout->addWidget(_copyBtn);
        
        leftLayout->addLayout(buttonLayout);
        
        // Right side - JSON output
        auto rightWidget = new QWidget();
        auto rightLayout = new QVBoxLayout(rightWidget);
        
        // JSON output section
        auto jsonGroup = new QGroupBox("Generated JSON Pipeline");
        jsonGroup->setStyleSheet("QGroupBox { font-weight: bold; margin-top: 10px; color: #2c3e50; } QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px 0 5px; }");
        auto jsonLayout = new QVBoxLayout(jsonGroup);
        
        _jsonOutput = new QTextEdit();
        _jsonOutput->setFont(QFont("Consolas", 10));
        _jsonOutput->setStyleSheet("QTextEdit { background-color: #2c3e50; color: #ecf0f1; border: 1px solid #34495e; border-radius: 5px; }");
        jsonLayout->addWidget(_jsonOutput);
        
        rightLayout->addWidget(jsonGroup);
        
        // Info section
        auto infoGroup = new QGroupBox("Integration Information");
        infoGroup->setStyleSheet("QGroupBox { font-weight: bold; margin-top: 10px; color: #2c3e50; } QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px 0 5px; }");
        auto infoLayout = new QVBoxLayout(infoGroup);
        
        _infoText = new QTextEdit();
        _infoText->setMaximumHeight(250);
        _infoText->setStyleSheet("QTextEdit { background-color: #34495e; color: #ecf0f1; border: 1px solid #2c3e50; border-radius: 5px; }");
        infoLayout->addWidget(_infoText);
        
        rightLayout->addWidget(infoGroup);
        
        // Add to splitter
        splitter->addWidget(leftWidget);
        splitter->addWidget(rightWidget);
        splitter->setSizes({900, 500});
        
        layout->addWidget(splitter);
        setCentralWidget(central);
        
        // Status bar
        statusBar()->showMessage("Ready - Right-click to add File Source node for your embedded application");
        
        // Initial info
        updateInfo();
    }
    
    void setupConnections() {
        connect(_exportBtn, &QPushButton::clicked, this, &MainWindow::exportToJson);
        connect(_saveBtn, &QPushButton::clicked, this, &MainWindow::saveJsonToFile);
        connect(_copyBtn, &QPushButton::clicked, this, &MainWindow::copyJsonToClipboard);
    }
    
    void updateInfo() {
        QString info = QString(
            "=== GStreamer File Source Designer ===\n"
            "Created by: muji\n"
            "Date: %1\n"
            "Target: Embedded Systems with File Processing\n\n"
            "=== Features ===\n"
            "• File Source Node - Video file input\n"
            "• Auto-format detection\n"
            "• Loop playback support\n"
            "• Start position control\n"
            "• Duration limiting\n"
            "• Buffer size optimization\n\n"
            "=== Supported Formats ===\n"
            "• MP4, MKV, AVI, MOV, WMV\n"
            "• FLV, WebM, TS, M2TS, MTS\n"
            "• 3GP, OGV, RM, RMVB\n\n"
            "=== Integration ===\n"
            "• Python GStreamer bindings\n"
            "• C++ GStreamer API\n"
            "• C GStreamer library\n"
            "• Perfect for embedded boards\n\n"
            "=== Usage ===\n"
            "1. Right-click to add File Source node\n"
            "2. Configure file path and settings\n"
            "3. Export JSON for your application\n"
            "4. Use in Python/C++/C embedded projects\n"
        ).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        
        _infoText->setPlainText(info);
    }
    
private slots:
    void exportToJson() {
        QJsonObject pipeline;
        QJsonArray nodes;
        
        // Example File Source node configuration
        QJsonObject fileSourceNode;
        fileSourceNode["id"] = "file_source_1";
        fileSourceNode["type"] = "file_source";
        fileSourceNode["config"] = QJsonObject{
            {"file_path", "/path/to/video.mp4"},
            {"loop", true},
            {"start_position", 0.0},
            {"duration", 0.0},
            {"buffer_size_kb", 64},
            {"format", "auto-detect"},
            {"typefind", true},
            {"gst_command", "filesrc location=/path/to/video.mp4 ! typefind ! decodebin"},
            {"python_example", "import gi; gi.require_version('Gst', '1.0'); from gi.repository import Gst; Gst.init(None); pipeline = Gst.parse_launch('filesrc location=/path/to/video.mp4 ! typefind ! decodebin')"},
            {"cpp_example", "#include <gst/gst.h>\nint main() { gst_init(nullptr, nullptr); GstElement* pipeline = gst_parse_launch(\"filesrc location=/path/to/video.mp4 ! typefind ! decodebin\", nullptr); return 0; }"}
        };
        nodes.append(fileSourceNode);
        
        // Pipeline metadata
        pipeline["nodes"] = nodes;
        pipeline["connections"] = QJsonArray{}; // No connections for single node
        
        pipeline["metadata"] = QJsonObject{
            {"created_by", "muji"},
            {"timestamp", QDateTime::currentDateTime().toString(Qt::ISODate)},
            {"version", "1.0"},
            {"target_platform", "embedded_linux"},
            {"gstreamer_version", "1.0+"},
            {"description", "GStreamer file source pipeline for embedded systems"},
            {"optimized_for", QJsonArray{"Python", "C++", "C", "Embedded Boards", "Single Board Computers"}}
        };
        
        // Complete GStreamer command examples
        pipeline["gstreamer_commands"] = QJsonObject{
            {"basic_playback", "gst-launch-1.0 filesrc location=/path/to/video.mp4 ! typefind ! decodebin ! autovideosink"},
            {"with_audio", "gst-launch-1.0 filesrc location=/path/to/video.mp4 ! typefind ! decodebin name=d d. ! queue ! autovideosink d. ! queue ! autoaudiosink"},
            {"loop_playback", "gst-launch-1.0 filesrc location=/path/to/video.mp4 ! typefind ! decodebin ! videoconvert ! autovideosink"},
            {"custom_format", "gst-launch-1.0 filesrc location=/path/to/video.mp4 ! qtdemux ! h264parse ! avdec_h264 ! autovideosink"}
        };
        
        // Embedded system optimization tips
        pipeline["embedded_optimization"] = QJsonObject{
            {"buffer_size", "Adjust buffer size based on available memory"},
            {"format_detection", "Use specific demuxer instead of typefind for better performance"},
            {"hardware_decode", "Use hardware decoders like omxh264dec on Raspberry Pi"},
            {"memory_management", "Monitor memory usage in constrained environments"},
            {"performance_tips", QJsonArray{
                "Use blocksize parameter for optimal file reading",
                "Disable typefind if format is known",
                "Use queue elements for buffering",
                "Consider using appsink for custom processing"
            }}
        };
        
        // Integration examples for your workflow
        pipeline["integration_examples"] = QJsonObject{
            {"python_gst", QJsonObject{
                {"basic_setup", "import gi; gi.require_version('Gst', '1.0'); from gi.repository import Gst, GObject"},
                {"pipeline_creation", "pipeline = Gst.parse_launch('filesrc location=video.mp4 ! decodebin ! autovideosink')"},
                {"state_control", "pipeline.set_state(Gst.State.PLAYING)"},
                {"perfect_for", "Embedded Python applications with GStreamer"}
            }},
            {"cpp_gst", QJsonObject{
                {"headers", "#include <gst/gst.h>"},
                {"init", "gst_init(&argc, &argv);"},
                {"pipeline", "GstElement* pipeline = gst_parse_launch(\"filesrc location=video.mp4 ! decodebin ! autovideosink\", nullptr);"},
                {"perfect_for", "High-performance C++ embedded applications"}
            }},
            {"c_gst", QJsonObject{
                {"basic_c", "Pure C GStreamer integration for minimal overhead"},
                {"embedded_focus", "Perfect for resource-constrained embedded systems"},
                {"memory_efficient", "Direct memory management for embedded boards"}
            }}
        };
        
        // Display JSON
        QJsonDocument doc(pipeline);
        _jsonOutput->setPlainText(doc.toJson(QJsonDocument::Indented));
        
        statusBar()->showMessage("File Source pipeline exported to JSON! Ready for your embedded Python/C++/C application", 3000);
    }
    
    void saveJsonToFile() {
        QString fileName = QFileDialog::getSaveFileName(
            this,
            "Save File Source Pipeline JSON",
            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/gstreamer_file_source.json",
            "JSON Files (*.json)"
        );
        
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(_jsonOutput->toPlainText().toUtf8());
                statusBar()->showMessage("JSON saved to " + fileName + " - Ready for embedded integration!", 3000);
            }
        }
    }
    
    void copyJsonToClipboard() {
        QApplication::clipboard()->setText(_jsonOutput->toPlainText());
        statusBar()->showMessage("JSON copied to clipboard - Perfect for your embedded GStreamer applications!", 2000);
    }
    
private:
    std::shared_ptr<DataFlowGraphModel> _model;
    GraphicsView* _view;
    QTextEdit* _jsonOutput;
    QTextEdit* _infoText;
    QPushButton* _exportBtn;
    QPushButton* _saveBtn;
    QPushButton* _copyBtn;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    app.setApplicationName("GStreamer File Source Designer");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("muji");
    app.setOrganizationDomain("embedded-gstreamer");
    
    MainWindow window;
    window.show();
    
    return app.exec();
}

#include "main.moc"