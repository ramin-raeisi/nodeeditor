#include "gst_nodes.hpp"
#include <QtWidgets/QApplication>
#include <QtCore/QDebug>
#include <QtCore/QDir>

// Base Node Implementation
GstBaseNode::GstBaseNode()
{
    _widget = new QWidget();
}

GstBaseNode::~GstBaseNode()
{
    if (_element) {
        gst_object_unref(_element);
    }
}

// RTSP Source Node Implementation
RTSPSourceNode::RTSPSourceNode()
{
    _element = gst_element_factory_make("rtspsrc", "rtsp_source");
    if (_element) {
        _output = std::make_shared<GstPipelineData>(_element);
    }

    // Create UI
    auto layout = new QVBoxLayout(_widget);

    layout->addWidget(new QLabel("RTSP URL:"));
    _urlEdit = new QLineEdit("rtsp://192.168.1.100:554/stream");
    layout->addWidget(_urlEdit);

    _connectBtn = new QPushButton("Connect");
    layout->addWidget(_connectBtn);

    _statusLabel = new QLabel("Disconnected");
    layout->addWidget(_statusLabel);

    connect(_urlEdit, &QLineEdit::textChanged, this, &RTSPSourceNode::onUrlChanged);
    connect(_connectBtn, &QPushButton::clicked, this, &RTSPSourceNode::onConnect);
}

unsigned int RTSPSourceNode::nPorts(PortType portType) const
{
    return (portType == PortType::Out) ? 1 : 0;
}

NodeDataType RTSPSourceNode::dataType(PortType portType, PortIndex portIndex) const
{
    return GstPipelineData().type();
}

std::shared_ptr<NodeData> RTSPSourceNode::outData(PortIndex port)
{
    return _output;
}

void RTSPSourceNode::onUrlChanged()
{
    if (_element) {
        g_object_set(_element, "location", _urlEdit->text().toStdString().c_str(), NULL);
    }
}

void RTSPSourceNode::onConnect()
{
    if (_element) {
        _statusLabel->setText("Connecting...");
        // In a real implementation, you'd start the pipeline here
        _statusLabel->setText("Connected");
    }
}

// Camera Source Node Implementation
CameraSourceNode::CameraSourceNode()
{
    _element = gst_element_factory_make("v4l2src", "camera_source");
    if (_element) {
        _output = std::make_shared<GstPipelineData>(_element);
    }

    // Create UI
    auto layout = new QVBoxLayout(_widget);

    layout->addWidget(new QLabel("Camera Device:"));
    _deviceCombo = new QComboBox();
    _deviceCombo->addItem("/dev/video0");
    _deviceCombo->addItem("/dev/video1");
    _deviceCombo->addItem("/dev/video2");
    layout->addWidget(_deviceCombo);

    _startBtn = new QPushButton("Start");
    layout->addWidget(_startBtn);

    _statusLabel = new QLabel("Stopped");
    layout->addWidget(_statusLabel);

    connect(_deviceCombo, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
            this, &CameraSourceNode::onDeviceChanged);
    connect(_startBtn, &QPushButton::clicked, this, &CameraSourceNode::onStart);
}

unsigned int CameraSourceNode::nPorts(PortType portType) const
{
    return (portType == PortType::Out) ? 1 : 0;
}

NodeDataType CameraSourceNode::dataType(PortType portType, PortIndex portIndex) const
{
    return GstPipelineData().type();
}

std::shared_ptr<NodeData> CameraSourceNode::outData(PortIndex port)
{
    return _output;
}

void CameraSourceNode::onDeviceChanged()
{
    if (_element) {
        g_object_set(_element, "device", _deviceCombo->currentText().toStdString().c_str(), NULL);
    }
}

void CameraSourceNode::onStart()
{
    if (_element) {
        _statusLabel->setText("Starting...");
        // In a real implementation, you'd start the pipeline here
        _statusLabel->setText("Running");
    }
}

// H.264 Encoder Node Implementation
H264EncoderNode::H264EncoderNode()
{
    _element = gst_element_factory_make("x264enc", "h264_encoder");
    if (_element) {
        _output = std::make_shared<GstPipelineData>(_element);
    }

    // Create UI
    auto layout = new QVBoxLayout(_widget);

    layout->addWidget(new QLabel("Bitrate (kbps):"));
    _bitrateEdit = new QLineEdit("1000");
    layout->addWidget(_bitrateEdit);

    layout->addWidget(new QLabel("Preset:"));
    _presetCombo = new QComboBox();
    _presetCombo->addItems({"ultrafast", "superfast", "veryfast", "faster", "fast", "medium", "slow", "slower", "veryslow"});
    _presetCombo->setCurrentText("fast");
    layout->addWidget(_presetCombo);

    connect(_bitrateEdit, &QLineEdit::textChanged, this, &H264EncoderNode::onBitrateChanged);
}

unsigned int H264EncoderNode::nPorts(PortType portType) const
{
    return 1; // Both input and output
}

NodeDataType H264EncoderNode::dataType(PortType portType, PortIndex portIndex) const
{
    return GstPipelineData().type();
}

std::shared_ptr<NodeData> H264EncoderNode::outData(PortIndex port)
{
    return _output;
}

void H264EncoderNode::setInData(std::shared_ptr<NodeData> data, PortIndex port)
{
    _input = std::dynamic_pointer_cast<GstPipelineData>(data);
}

void H264EncoderNode::onBitrateChanged()
{
    if (_element) {
        int bitrate = _bitrateEdit->text().toInt();
        g_object_set(_element, "bitrate", bitrate, NULL);
    }
}

// RTMP Sink Node Implementation
RTMPSinkNode::RTMPSinkNode()
{
    _element = gst_element_factory_make("rtmpsink", "rtmp_sink");
    if (_element) {
        _output = std::make_shared<GstPipelineData>(_element);
    }

    // Create UI
    auto layout = new QVBoxLayout(_widget);

    layout->addWidget(new QLabel("RTMP URL:"));
    _urlEdit = new QLineEdit("rtmp://localhost:1935/live/stream");
    layout->addWidget(_urlEdit);

    _streamBtn = new QPushButton("Start Stream");
    layout->addWidget(_streamBtn);

    _statusLabel = new QLabel("Not Streaming");
    layout->addWidget(_statusLabel);

    connect(_urlEdit, &QLineEdit::textChanged, this, &RTMPSinkNode::onUrlChanged);
    connect(_streamBtn, &QPushButton::clicked, this, &RTMPSinkNode::onStreamStart);
}

unsigned int RTMPSinkNode::nPorts(PortType portType) const
{
    return (portType == PortType::In) ? 1 : 0;
}

NodeDataType RTMPSinkNode::dataType(PortType portType, PortIndex portIndex) const
{
    return GstPipelineData().type();
}

std::shared_ptr<NodeData> RTMPSinkNode::outData(PortIndex port)
{
    return nullptr; // Sink has no output
}

void RTMPSinkNode::setInData(std::shared_ptr<NodeData> data, PortIndex port)
{
    _input = std::dynamic_pointer_cast<GstPipelineData>(data);
}

void RTMPSinkNode::onUrlChanged()
{
    if (_element) {
        g_object_set(_element, "location", _urlEdit->text().toStdString().c_str(), NULL);
    }
}

void RTMPSinkNode::onStreamStart()
{
    if (_element) {
        _statusLabel->setText("Streaming...");
        // In a real implementation, you'd start the streaming here
    }
}
