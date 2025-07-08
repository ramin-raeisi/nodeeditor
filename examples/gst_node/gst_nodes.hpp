#pragma once

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeData>
#include <QtCore/QObject>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QDateTime>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QFileDialog>

using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDelegateModel;
using QtNodes::PortIndex;
using QtNodes::PortType;

// Connection data for linking nodes
class ConnectionData : public NodeData
{
public:
    ConnectionData() = default;
    ConnectionData(const QString& type) : _type(type) {}
    
    NodeDataType type() const override {
        return NodeDataType {"Connection", "Pipeline Connection"};
    }
    
    QString connectionType() const { return _type; }
    
private:
    QString _type = "video";
};

// File Source Node - Perfect for embedded systems working with video files
class FileSourceNode : public NodeDelegateModel
{
    Q_OBJECT
    
public:
    FileSourceNode() {
        _widget = new QWidget();
        auto layout = new QVBoxLayout(_widget);
        
        // File path selection
        auto pathLayout = new QHBoxLayout();
        pathLayout->addWidget(new QLabel("File Path:"));
        _pathEdit = new QLineEdit("/path/to/video.mp4");
        pathLayout->addWidget(_pathEdit);
        
        _browseBtn = new QPushButton("Browse");
        _browseBtn->setMaximumWidth(80);
        pathLayout->addWidget(_browseBtn);
        layout->addLayout(pathLayout);
        
        // Loop playback
        layout->addWidget(new QLabel("Playback:"));
        _loopCheckBox = new QCheckBox("Loop playback");
        _loopCheckBox->setChecked(true);
        layout->addWidget(_loopCheckBox);
        
        // Start position
        layout->addWidget(new QLabel("Start Position (seconds):"));
        _startPosSpinBox = new QDoubleSpinBox();
        _startPosSpinBox->setRange(0, 999999);
        _startPosSpinBox->setValue(0);
        _startPosSpinBox->setDecimals(2);
        layout->addWidget(_startPosSpinBox);
        
        // Duration limit
        layout->addWidget(new QLabel("Duration (seconds, 0=unlimited):"));
        _durationSpinBox = new QDoubleSpinBox();
        _durationSpinBox->setRange(0, 999999);
        _durationSpinBox->setValue(0);
        _durationSpinBox->setDecimals(2);
        layout->addWidget(_durationSpinBox);
        
        // Buffer size
        layout->addWidget(new QLabel("Buffer Size (KB):"));
        _bufferSizeSpinBox = new QSpinBox();
        _bufferSizeSpinBox->setRange(1, 10240);
        _bufferSizeSpinBox->setValue(64);
        layout->addWidget(_bufferSizeSpinBox);
        
        // File format detection
        layout->addWidget(new QLabel("Format:"));
        _formatCombo = new QComboBox();
        _formatCombo->addItems({
            "auto-detect",
            "mp4", "mkv", "avi", "mov", "wmv", "flv", "webm",
            "ts", "m2ts", "mts", "3gp", "ogv", "rm", "rmvb"
        });
        layout->addWidget(_formatCombo);
        
        // Typefind (for auto-detection)
        layout->addWidget(new QLabel("Type Detection:"));
        _typefindCheckBox = new QCheckBox("Enable typefind");
        _typefindCheckBox->setChecked(true);
        layout->addWidget(_typefindCheckBox);
        
        // Connection status
        _statusLabel = new QLabel("Ready");
        _statusLabel->setStyleSheet("color: green; font-weight: bold;");
        layout->addWidget(_statusLabel);
        
        _output = std::make_shared<ConnectionData>("video");
        
        // Connect signals
        connect(_browseBtn, &QPushButton::clicked, this, &FileSourceNode::browseFile);
        connect(_pathEdit, &QLineEdit::textChanged, this, &FileSourceNode::updateStatus);
        connect(_loopCheckBox, &QCheckBox::toggled, this, &FileSourceNode::updateStatus);
        connect(_startPosSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FileSourceNode::updateStatus);
        connect(_durationSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FileSourceNode::updateStatus);
        
        updateStatus();
    }
    
    QString caption() const override { return "File Source"; }
    QString name() const override { return "FileSourceNode"; }
    
    unsigned int nPorts(PortType portType) const override {
        return (portType == PortType::Out) ? 1 : 0;
    }
    
    NodeDataType dataType(PortType portType, PortIndex portIndex) const override {
        return ConnectionData().type();
    }
    
    std::shared_ptr<NodeData> outData(PortIndex port) override {
        return _output;
    }
    
    void setInData(std::shared_ptr<NodeData>, PortIndex) override {}
    
    QWidget* embeddedWidget() override { return _widget; }
    
    QJsonObject toJson() const {
        QJsonObject obj;
        obj["type"] = "file_source";
        obj["file_path"] = _pathEdit->text();
        obj["loop"] = _loopCheckBox->isChecked();
        obj["start_position"] = _startPosSpinBox->value();
        obj["duration"] = _durationSpinBox->value();
        obj["buffer_size_kb"] = _bufferSizeSpinBox->value();
        obj["format"] = _formatCombo->currentText();
        obj["typefind"] = _typefindCheckBox->isChecked();
        
        // Generate GStreamer command based on settings
        QString gstCommand;
        
        if (_typefindCheckBox->isChecked() && _formatCombo->currentText() == "auto-detect") {
            // Use filesrc with typefind for auto-detection
            gstCommand = QString("filesrc location=%1 ! typefind ! decodebin").arg(_pathEdit->text());
        } else {
            // Use filesrc with specific format
            gstCommand = QString("filesrc location=%1").arg(_pathEdit->text());
            
            // Add format-specific demuxer if not auto-detect
            if (_formatCombo->currentText() != "auto-detect") {
                QString format = _formatCombo->currentText();
                QString demuxer;
                
                if (format == "mp4") demuxer = "qtdemux";
                else if (format == "mkv") demuxer = "matroskademux";
                else if (format == "avi") demuxer = "avidemux";
                else if (format == "mov") demuxer = "qtdemux";
                else if (format == "ts") demuxer = "tsdemux";
                else if (format == "flv") demuxer = "flvdemux";
                else if (format == "webm") demuxer = "matroskademux";
                else demuxer = "decodebin";
                
                gstCommand += QString(" ! %1").arg(demuxer);
            }
        }
        
        // Add buffer size if specified
        if (_bufferSizeSpinBox->value() > 0) {
            gstCommand = QString("filesrc location=%1 blocksize=%2 ! ").arg(_pathEdit->text()).arg(_bufferSizeSpinBox->value() * 1024) + gstCommand.split(" ! ", Qt::SkipEmptyParts).mid(1).join(" ! ");
        }
        
        obj["gst_command"] = gstCommand;
        
        // Add Python integration example
        obj["python_example"] = QString(
            "import gi\n"
            "gi.require_version('Gst', '1.0')\n"
            "from gi.repository import Gst\n"
            "\n"
            "# Initialize GStreamer\n"
            "Gst.init(None)\n"
            "\n"
            "# Create pipeline\n"
            "pipeline_str = \"%1\"\n"
            "pipeline = Gst.parse_launch(pipeline_str)\n"
            "\n"
            "# Perfect for embedded systems!"
        ).arg(gstCommand);
        
        // Add C++ integration example  
        obj["cpp_example"] = QString(
            "#include <gst/gst.h>\n"
            "\n"
            "int main() {\n"
            "    gst_init(nullptr, nullptr);\n"
            "    \n"
            "    GstElement* pipeline = gst_parse_launch(\"%1\", nullptr);\n"
            "    \n"
            "    gst_element_set_state(pipeline, GST_STATE_PLAYING);\n"
            "    \n"
            "    // Perfect for embedded C++ applications!\n"
            "    return 0;\n"
            "}"
        ).arg(gstCommand);
        
        return obj;
    }
    
private slots:
    void browseFile() {
        QString fileName = QFileDialog::getOpenFileName(
            _widget,
            "Select Video File",
            _pathEdit->text(),
            "Video Files (*.mp4 *.mkv *.avi *.mov *.wmv *.flv *.webm *.ts *.m2ts *.mts *.3gp *.ogv *.rm *.rmvb);;All Files (*)"
        );
        
        if (!fileName.isEmpty()) {
            _pathEdit->setText(fileName);
            
            // Auto-detect format based on file extension
            QString ext = fileName.split('.').last().toLower();
            for (int i = 0; i < _formatCombo->count(); ++i) {
                if (_formatCombo->itemText(i) == ext) {
                    _formatCombo->setCurrentIndex(i);
                    break;
                }
            }
        }
    }
    
    void updateStatus() {
        QString path = _pathEdit->text();
        if (path.isEmpty()) {
            _statusLabel->setText("No file selected");
            _statusLabel->setStyleSheet("color: red; font-weight: bold;");
        } else if (!QFile::exists(path)) {
            _statusLabel->setText("File not found");
            _statusLabel->setStyleSheet("color: red; font-weight: bold;");
        } else {
            QString status = "Ready";
            if (_loopCheckBox->isChecked()) status += " (Loop)";
            if (_startPosSpinBox->value() > 0) status += QString(" (Start: %1s)").arg(_startPosSpinBox->value());
            if (_durationSpinBox->value() > 0) status += QString(" (Duration: %1s)").arg(_durationSpinBox->value());
            
            _statusLabel->setText(status);
            _statusLabel->setStyleSheet("color: green; font-weight: bold;");
        }
    }
    
private:
    QWidget* _widget;
    QLineEdit* _pathEdit;
    QPushButton* _browseBtn;
    QCheckBox* _loopCheckBox;
    QDoubleSpinBox* _startPosSpinBox;
    QDoubleSpinBox* _durationSpinBox;
    QSpinBox* _bufferSizeSpinBox;
    QComboBox* _formatCombo;
    QCheckBox* _typefindCheckBox;
    QLabel* _statusLabel;
    std::shared_ptr<ConnectionData> _output;
};