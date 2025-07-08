#pragma once

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeData>
#include <QtCore/QObject>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QGroupBox>
#include <gst/gst.h>

using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDelegateModel;
using QtNodes::PortIndex;
using QtNodes::PortType;

// Simple data type for our pipeline connections
class GstPipelineData : public NodeData
{
public:
    GstPipelineData() = default;
    GstPipelineData(const QString& info) : _info(info) {}

    NodeDataType type() const override {
        return NodeDataType {"GstPipeline", "GStreamer Pipeline"};
    }

    QString info() const { return _info; }

private:
    QString _info = "GStreamer Element";
};

// RTSP Source Node - Perfect for your camera work
class RTSPSourceNode : public NodeDelegateModel
{
    Q_OBJECT

public:
    RTSPSourceNode() {
        _widget = new QWidget();
        auto layout = new QVBoxLayout(_widget);

        // RTSP URL input
        layout->addWidget(new QLabel("RTSP URL:"));
        _urlEdit = new QLineEdit("rtsp://192.168.1.100:554/stream");
        layout->addWidget(_urlEdit);

        // Connection controls
        _connectBtn = new QPushButton("Connect");
        layout->addWidget(_connectBtn);

        _statusLabel = new QLabel("Disconnected");
        layout->addWidget(_statusLabel);

        // Create output data
        _output = std::make_shared<GstPipelineData>("RTSP Source");

        connect(_connectBtn, &QPushButton::clicked, this, &RTSPSourceNode::onConnect);
    }

    QString caption() const override { return "RTSP Source"; }
    QString name() const override { return "RTSPSourceNode"; }

    unsigned int nPorts(PortType portType) const override {
        return (portType == PortType::Out) ? 1 : 0;
    }

    NodeDataType dataType(PortType portType, PortIndex portIndex) const override {
        return GstPipelineData().type();
    }

    std::shared_ptr<NodeData> outData(PortIndex port) override {
        return _output;
    }

    void setInData(std::shared_ptr<NodeData>, PortIndex) override {}

    QWidget* embeddedWidget() override { return _widget; }

private slots:
    void onConnect() {
        QString url = _urlEdit->text();
        if (url.isEmpty()) {
            _statusLabel->setText("Invalid URL");
            return;
        }

        _statusLabel->setText("Connecting to: " + url);
        // Here you would create your GStreamer pipeline
        // gst_parse_launch(url.toStdString().c_str(), nullptr);
    }

private:
    QWidget* _widget;
    QLineEdit* _urlEdit;
    QPushButton* _connectBtn;
    QLabel* _statusLabel;
    std::shared_ptr<GstPipelineData> _output;
};

// V4L2 Camera Source Node
class CameraSourceNode : public NodeDelegateModel
{
    Q_OBJECT

public:
    CameraSourceNode() {
        _widget = new QWidget();
        auto layout = new QVBoxLayout(_widget);

        // Device selection
        layout->addWidget(new QLabel("Camera Device:"));
        _deviceCombo = new QComboBox();
        _deviceCombo->addItems({"/dev/video0", "/dev/video1", "/dev/video2", "/dev/video3"});
        layout->addWidget(_deviceCombo);

        // Resolution selection
        layout->addWidget(new QLabel("Resolution:"));
        _resCombo = new QComboBox();
        _resCombo->addItems({"640x480", "1280x720", "1920x1080", "3840x2160"});
        _resCombo->setCurrentText("1280x720");
        layout->addWidget(_resCombo);

        // FPS selection
        layout->addWidget(new QLabel("FPS:"));
        _fpsSpinBox = new QSpinBox();
        _fpsSpinBox->setRange(1, 60);
        _fpsSpinBox->setValue(30);
        layout->addWidget(_fpsSpinBox);

        _startBtn = new QPushButton("Start Camera");
        layout->addWidget(_startBtn);

        _statusLabel = new QLabel("Camera Stopped");
        layout->addWidget(_statusLabel);

        _output = std::make_shared<GstPipelineData>("Camera Source");

        connect(_startBtn, &QPushButton::clicked, this, &CameraSourceNode::onStart);
    }

    QString caption() const override { return "Camera (V4L2)"; }
    QString name() const override { return "CameraSourceNode"; }

    unsigned int nPorts(PortType portType) const override {
        return (portType == PortType::Out) ? 1 : 0;
    }

    NodeDataType dataType(PortType portType, PortIndex portIndex) const override {
        return GstPipelineData().type();
    }

    std::shared_ptr<NodeData> outData(PortIndex port) override {
        return _output;
    }

    void setInData(std::shared_ptr<NodeData>, PortIndex) override {}

    QWidget* embeddedWidget() override { return _widget; }

private slots:
    void onStart() {
        QString device = _deviceCombo->currentText();
        QString resolution = _resCombo->currentText();
        int fps = _fpsSpinBox->value();

        _statusLabel->setText(QString("Camera: %1 @ %2, %3fps").arg(device, resolution).arg(fps));

        // Here you would create your V4L2 pipeline:
        // QString pipeline = QString("v4l2src device=%1 ! video/x-raw,width=%2,height=%3,framerate=%4/1")
        //                   .arg(device).arg(width).arg(height).arg(fps);
    }

private:
    QWidget* _widget;
    QComboBox* _deviceCombo;
    QComboBox* _resCombo;
    QSpinBox* _fpsSpinBox;
    QPushButton* _startBtn;
    QLabel* _statusLabel;
    std::shared_ptr<GstPipelineData> _output;
};

// H.264 Encoder Node
class H264EncoderNode : public NodeDelegateModel
{
    Q_OBJECT

public:
    H264EncoderNode() {
        _widget = new QWidget();
        auto layout = new QVBoxLayout(_widget);

        // Encoder settings
        layout->addWidget(new QLabel("Bitrate (kbps):"));
        _bitrateSpinBox = new QSpinBox();
        _bitrateSpinBox->setRange(100, 50000);
        _bitrateSpinBox->setValue(2000);
        layout->addWidget(_bitrateSpinBox);

        layout->addWidget(new QLabel("Preset:"));
        _presetCombo = new QComboBox();
        _presetCombo->addItems({"ultrafast", "superfast", "veryfast", "faster", "fast", "medium", "slow"});
        _presetCombo->setCurrentText("fast");
        layout->addWidget(_presetCombo);

        layout->addWidget(new QLabel("Profile:"));
        _profileCombo = new QComboBox();
        _profileCombo->addItems({"baseline", "main", "high"});
        _profileCombo->setCurrentText("main");
        layout->addWidget(_profileCombo);

        _statusLabel = new QLabel("Ready to encode");
        layout->addWidget(_statusLabel);

        _output = std::make_shared<GstPipelineData>("H264 Encoded");

        connect(_bitrateSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &H264EncoderNode::updateStatus);
        connect(_presetCombo, QOverload<const QString&>::of(&QComboBox::currentTextChanged), this, &H264EncoderNode::updateStatus);
    }

    QString caption() const override { return "H.264 Encoder"; }
    QString name() const override { return "H264EncoderNode"; }

    unsigned int nPorts(PortType portType) const override { return 1; }

    NodeDataType dataType(PortType portType, PortIndex portIndex) const override {
        return GstPipelineData().type();
    }

    std::shared_ptr<NodeData> outData(PortIndex port) override {
        return _output;
    }

    void setInData(std::shared_ptr<NodeData> data, PortIndex port) override {
        _input = std::dynamic_pointer_cast<GstPipelineData>(data);
        updateStatus();
    }

    QWidget* embeddedWidget() override { return _widget; }

private slots:
    void updateStatus() {
        int bitrate = _bitrateSpinBox->value();
        QString preset = _presetCombo->currentText();
        QString profile = _profileCombo->currentText();

        _statusLabel->setText(QString("Encoding: %1kbps, %2, %3").arg(bitrate).arg(preset).arg(profile));

        // Here you would configure your x264enc element:
        // g_object_set(encoder, "bitrate", bitrate, "speed-preset", preset, "profile", profile, NULL);
    }

private:
    QWidget* _widget;
    QSpinBox* _bitrateSpinBox;
    QComboBox* _presetCombo;
    QComboBox* _profileCombo;
    QLabel* _statusLabel;
    std::shared_ptr<GstPipelineData> _input;
    std::shared_ptr<GstPipelineData> _output;
};

// RTMP Streaming Node
class RTMPSinkNode : public NodeDelegateModel
{
    Q_OBJECT

public:
    RTMPSinkNode() {
        _widget = new QWidget();
        auto layout = new QVBoxLayout(_widget);

        // RTMP URL
        layout->addWidget(new QLabel("RTMP URL:"));
        _urlEdit = new QLineEdit("rtmp://localhost:1935/live/stream");
        layout->addWidget(_urlEdit);

        // Stream key (for platforms like Twitch, YouTube)
        layout->addWidget(new QLabel("Stream Key (optional):"));
        _keyEdit = new QLineEdit();
        _keyEdit->setEchoMode(QLineEdit::Password);
        layout->addWidget(_keyEdit);

        _startBtn = new QPushButton("Start Streaming");
        layout->addWidget(_startBtn);

        _statusLabel = new QLabel("Not streaming");
        layout->addWidget(_statusLabel);

        connect(_startBtn, &QPushButton::clicked, this, &RTMPSinkNode::onStreamToggle);
    }

    QString caption() const override { return "RTMP Stream"; }
    QString name() const override { return "RTMPSinkNode"; }

    unsigned int nPorts(PortType portType) const override {
        return (portType == PortType::In) ? 1 : 0;
    }

    NodeDataType dataType(PortType portType, PortIndex portIndex) const override {
        return GstPipelineData().type();
    }

    std::shared_ptr<NodeData> outData(PortIndex port) override {
        return nullptr;
    }

    void setInData(std::shared_ptr<NodeData> data, PortIndex port) override {
        _input = std::dynamic_pointer_cast<GstPipelineData>(data);
    }

    QWidget* embeddedWidget() override { return _widget; }

private slots:
    void onStreamToggle() {
        if (_startBtn->text() == "Start Streaming") {
            QString url = _urlEdit->text();
            QString key = _keyEdit->text();

            if (!key.isEmpty()) {
                url += "/" + key;
            }

            _statusLabel->setText("Streaming to: " + url);
            _startBtn->setText("Stop Streaming");

            // Here you would start your RTMP stream:
            // QString pipeline = QString("... ! rtmpsink location=%1").arg(url);
        } else {
            _statusLabel->setText("Stream stopped");
            _startBtn->setText("Start Streaming");
        }
    }

private:
    QWidget* _widget;
    QLineEdit* _urlEdit;
    QLineEdit* _keyEdit;
    QPushButton* _startBtn;
    QLabel* _statusLabel;
    std::shared_ptr<GstPipelineData> _input;
};
