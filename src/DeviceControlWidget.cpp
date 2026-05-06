#include "DeviceControlWidget.h"
#include "SerialManager.h"
#include "CommandFactory.h"
#include "SettingsManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QDateTime>

DeviceControlWidget::DeviceControlWidget(QWidget *parent)
    : QWidget(parent)
    , m_isConnected(false)
{
    setupUi();

    auto& serial = SerialManager::instance();
    connect(&serial, &SerialManager::connectionStatusChanged,
            this, &DeviceControlWidget::onConnectionStatusChanged);
    connect(&serial, &SerialManager::responseReceived, [this](const QByteArray& resp) {
        appendToOutput(QString::fromLatin1(resp), "blue");
    });
    connect(&serial, &SerialManager::statusByteReceived, [this](int status) {
        appendToOutput(QString("Status byte received: %1 (0x%2)").arg(status).arg(status, 2, 16, QChar('0')), "green");
    });
    connect(&serial, &SerialManager::errorCodeReceived, [this](int code) {
        appendToOutput(QString("Error code: %1").arg(code), "red");
    });
}

void DeviceControlWidget::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);

    createConnectionGroup();
    createCommandGroup();
    createConfigGroup();
    createOutputGroup();
    createCustomCommandGroup();

    mainLayout->addWidget(m_connectionGroup);
    mainLayout->addWidget(m_commandGroup);
    mainLayout->addWidget(m_configGroup);
    mainLayout->addWidget(m_outputGroup);
    mainLayout->addWidget(m_customCommandGroup);
}

void DeviceControlWidget::createConnectionGroup()
{
    m_connectionGroup = new QGroupBox(tr("Connection"), this);
    auto* layout = new QHBoxLayout(m_connectionGroup);

    auto* portLabel = new QLabel(tr("Port:"), this);
    m_portCombo = new QComboBox(this);
    m_portCombo->setEditable(true);

    auto* baudLabel = new QLabel(tr("Baudrate:"), this);
    m_baudCombo = new QComboBox(this);
    m_baudCombo->addItems({"9600", "4800"});
    m_baudCombo->setCurrentText("9600");

    m_connectBtn = new QPushButton(tr("Connect"), this);
    m_disconnectBtn = new QPushButton(tr("Disconnect"), this);
    m_disconnectBtn->setEnabled(false);
    m_statusLabel = new QLabel(tr("Not connected"), this);
    m_statusLabel->setStyleSheet("QLabel { color: red; }");

    connect(m_connectBtn, &QPushButton::clicked, this, &DeviceControlWidget::onConnect);
    connect(m_disconnectBtn, &QPushButton::clicked, this, &DeviceControlWidget::onDisconnect);

    QString savedPort;
    int savedBaud;
    SettingsManager::instance().loadPortSettings(savedPort, savedBaud);
    if (!savedPort.isEmpty()) {
        m_portCombo->setCurrentText(savedPort);
    }
    m_baudCombo->setCurrentText(QString::number(savedBaud));

    const auto ports = SerialManager::instance().getAvailablePorts();
    for (const QString& port : ports) {
        m_portCombo->addItem(port);
    }

    layout->addWidget(portLabel);
    layout->addWidget(m_portCombo);
    layout->addWidget(baudLabel);
    layout->addWidget(m_baudCombo);
    layout->addWidget(m_connectBtn);
    layout->addWidget(m_disconnectBtn);
    layout->addWidget(m_statusLabel);
    layout->addStretch();
}

void DeviceControlWidget::createCommandGroup()
{
    m_commandGroup = new QGroupBox(tr("Commands"), this);
    auto* layout = new QGridLayout(m_commandGroup);

    m_homeBtn = new QPushButton(tr("Home (H)"), this);
    m_initBtn = new QPushButton(tr("Initialize (IN)"), this);
    m_emergencyBtn = new QPushButton(tr("Emergency Stop"), this);
    m_getStatusBtn = new QPushButton(tr("Get Status (OS)"), this);
    m_getErrorBtn = new QPushButton(tr("Get Error (OE)"), this);
    m_getPositionBtn = new QPushButton(tr("Get Position (OC)"), this);
    m_getIdentificationBtn = new QPushButton(tr("Get ID (OI)"), this);
    m_getP1P2Btn = new QPushButton(tr("Get P1/P2 (OP)"), this);
    m_getWindowBtn = new QPushButton(tr("Get Window (OW)"), this);
    m_getHardClipBtn = new QPushButton(tr("Get Hard Clip (OH)"), this);
    m_getFactorBtn = new QPushButton(tr("Get Factor (OF)"), this);
    m_abortBufferBtn = new QPushButton(tr("Abort Buffer (ESC.K)"), this);
    m_resetDeviceBtn = new QPushButton(tr("Reset Device (ESC.R)"), this);

    connect(m_homeBtn, &QPushButton::clicked, this, &DeviceControlWidget::onHome);
    connect(m_initBtn, &QPushButton::clicked, this, &DeviceControlWidget::onInitialize);
    connect(m_emergencyBtn, &QPushButton::clicked, this, &DeviceControlWidget::onEmergencyStop);
    connect(m_getStatusBtn, &QPushButton::clicked, this, &DeviceControlWidget::onGetStatus);
    connect(m_getErrorBtn, &QPushButton::clicked, this, &DeviceControlWidget::onGetError);
    connect(m_getPositionBtn, &QPushButton::clicked, this, &DeviceControlWidget::onGetPosition);
    connect(m_getIdentificationBtn, &QPushButton::clicked, this, &DeviceControlWidget::onGetIdentification);
    connect(m_getP1P2Btn, &QPushButton::clicked, this, &DeviceControlWidget::onGetP1P2);
    connect(m_getWindowBtn, &QPushButton::clicked, this, &DeviceControlWidget::onGetWindow);
    connect(m_getHardClipBtn, &QPushButton::clicked, this, &DeviceControlWidget::onGetHardClip);
    connect(m_getFactorBtn, &QPushButton::clicked, this, &DeviceControlWidget::onGetFactor);
    connect(m_abortBufferBtn, &QPushButton::clicked, this, &DeviceControlWidget::onAbortBuffer);
    connect(m_resetDeviceBtn, &QPushButton::clicked, this, &DeviceControlWidget::onResetDeviceControl);

    int row = 0;
    layout->addWidget(m_homeBtn, row, 0);
    layout->addWidget(m_initBtn, row, 1);
    layout->addWidget(m_emergencyBtn, row, 2);
    layout->addWidget(m_getStatusBtn, row, 3);
    row++;
    layout->addWidget(m_getErrorBtn, row, 0);
    layout->addWidget(m_getPositionBtn, row, 1);
    layout->addWidget(m_getIdentificationBtn, row, 2);
    layout->addWidget(m_getP1P2Btn, row, 3);
    row++;
    layout->addWidget(m_getWindowBtn, row, 0);
    layout->addWidget(m_getHardClipBtn, row, 1);
    layout->addWidget(m_getFactorBtn, row, 2);
    layout->addWidget(m_abortBufferBtn, row, 3);
    row++;
    layout->addWidget(m_resetDeviceBtn, row, 0);
}

void DeviceControlWidget::createOutputGroup()
{
    m_outputGroup = new QGroupBox(tr("Output / Response"), this);
    auto* layout = new QVBoxLayout(m_outputGroup);

    m_outputText = new QTextEdit(this);
    m_outputText->setReadOnly(true);
    m_outputText->setFont(QFont("Consolas", 9));

    m_clearOutputBtn = new QPushButton(tr("Clear Output"), this);
    connect(m_clearOutputBtn, &QPushButton::clicked, this, &DeviceControlWidget::onClearOutput);

    layout->addWidget(m_outputText);
    layout->addWidget(m_clearOutputBtn);
}

void DeviceControlWidget::createConfigGroup()
{
    m_configGroup = new QGroupBox(tr("Configuration"), this);
    auto* layout = new QGridLayout(m_configGroup);

    m_motorControlCheck = new QCheckBox(tr("Spindle Motor ON"), this);
    connect(m_motorControlCheck, &QCheckBox::toggled, this, &DeviceControlWidget::onMotorControl);

    auto* xySpeedLabel = new QLabel(tr("XY Speed (mm/sec):"), this);
    m_xySpeedSpin = new QSpinBox(this);
    m_xySpeedSpin->setRange(0, 60);
    m_xySpeedSpin->setValue(10);
    m_applyVelocityBtn = new QPushButton(tr("Apply"), this);
    connect(m_applyVelocityBtn, &QPushButton::clicked, this, &DeviceControlWidget::onSetVelocityXY);

    auto* zSpeedLabel = new QLabel(tr("Z Speed (mm/sec):"), this);
    m_zSpeedSpin = new QSpinBox(this);
    m_zSpeedSpin->setRange(0, 30);
    m_zSpeedSpin->setValue(5);

    auto* z1Label = new QLabel(tr("Z1 (cut depth, -3000..0):"), this);
    m_z1Spin = new QSpinBox(this);
    m_z1Spin->setRange(-3000, 0);
    m_z1Spin->setValue(-500);
    auto* z2Label = new QLabel(tr("Z2 (lift height, 0..3000):"), this);
    m_z2Spin = new QSpinBox(this);
    m_z2Spin->setRange(0, 3000);
    m_z2Spin->setValue(500);
    m_applyZ1Z2Btn = new QPushButton(tr("Apply Z1/Z2"), this);
    connect(m_applyZ1Z2Btn, &QPushButton::clicked, this, &DeviceControlWidget::onSetZ1Z2);

    auto* windowLabel = new QLabel(tr("Window (LLx,LLy,URx,URy):"), this);
    m_windowLLxSpin = new QSpinBox(this);
    m_windowLLxSpin->setRange(0, 30500);
    m_windowLLxSpin->setValue(0);
    m_windowLLySpin = new QSpinBox(this);
    m_windowLLySpin->setRange(0, 23000);
    m_windowLLySpin->setValue(0);
    m_windowURxSpin = new QSpinBox(this);
    m_windowURxSpin->setRange(0, 30500);
    m_windowURxSpin->setValue(30500);
    m_windowURySpin = new QSpinBox(this);
    m_windowURySpin->setRange(0, 23000);
    m_windowURySpin->setValue(23000);
    m_applyWindowBtn = new QPushButton(tr("Apply Window"), this);
    connect(m_applyWindowBtn, &QPushButton::clicked, this, &DeviceControlWidget::onSetWindow);

    auto* scalingLabel = new QLabel(tr("Scaling (Xmin,Xmax,Ymin,Ymax):"), this);
    m_scalingXminSpin = new QDoubleSpinBox(this);
    m_scalingXminSpin->setRange(-1000, 1000);
    m_scalingXminSpin->setValue(0);
    m_scalingXmaxSpin = new QDoubleSpinBox(this);
    m_scalingXmaxSpin->setRange(-1000, 1000);
    m_scalingXmaxSpin->setValue(1);
    m_scalingYminSpin = new QDoubleSpinBox(this);
    m_scalingYminSpin->setRange(-1000, 1000);
    m_scalingYminSpin->setValue(0);
    m_scalingYmaxSpin = new QDoubleSpinBox(this);
    m_scalingYmaxSpin->setRange(-1000, 1000);
    m_scalingYmaxSpin->setValue(1);
    m_applyScalingBtn = new QPushButton(tr("Apply Scaling"), this);
    connect(m_applyScalingBtn, &QPushButton::clicked, this, &DeviceControlWidget::onSetScaling);

    int row = 0;
    layout->addWidget(m_motorControlCheck, row, 0, 1, 3);
    row++;
    layout->addWidget(xySpeedLabel, row, 0);
    layout->addWidget(m_xySpeedSpin, row, 1);
    layout->addWidget(m_applyVelocityBtn, row, 2);
    row++;
    layout->addWidget(zSpeedLabel, row, 0);
    layout->addWidget(m_zSpeedSpin, row, 1);
    row++;
    layout->addWidget(z1Label, row, 0);
    layout->addWidget(m_z1Spin, row, 1);
    layout->addWidget(m_applyZ1Z2Btn, row, 2);
    row++;
    layout->addWidget(z2Label, row, 0);
    layout->addWidget(m_z2Spin, row, 1);
    row++;
    layout->addWidget(windowLabel, row, 0, 1, 3);
    row++;
    layout->addWidget(m_windowLLxSpin, row, 0);
    layout->addWidget(m_windowLLySpin, row, 1);
    row++;
    layout->addWidget(m_windowURxSpin, row, 0);
    layout->addWidget(m_windowURySpin, row, 1);
    row++;
    layout->addWidget(m_applyWindowBtn, row, 0, 1, 2);
    row++;
    layout->addWidget(scalingLabel, row, 0, 1, 3);
    row++;
    layout->addWidget(m_scalingXminSpin, row, 0);
    layout->addWidget(m_scalingXmaxSpin, row, 1);
    layout->addWidget(m_scalingYminSpin, row, 2);
    row++;
    layout->addWidget(m_scalingYmaxSpin, row, 1);
    row++;
    layout->addWidget(m_applyScalingBtn, row, 0, 1, 2);
}

void DeviceControlWidget::createCustomCommandGroup()
{
    m_customCommandGroup = new QGroupBox(tr("Custom Command"), this);
    auto* layout = new QHBoxLayout(m_customCommandGroup);

    m_customCommandEdit = new QLineEdit(this);
    m_customCommandEdit->setPlaceholderText(tr("Enter CAMM-GL command (e.g., 'PA5000,5000;')"));
    m_sendCustomBtn = new QPushButton(tr("Send"), this);

    connect(m_sendCustomBtn, &QPushButton::clicked, this, &DeviceControlWidget::onSendCustomCommand);
    connect(m_customCommandEdit, &QLineEdit::returnPressed, this, &DeviceControlWidget::onSendCustomCommand);

    layout->addWidget(m_customCommandEdit);
    layout->addWidget(m_sendCustomBtn);
}

void DeviceControlWidget::showPortSettings()
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Serial Port Settings"));
    auto* layout = new QVBoxLayout(&dialog);

    auto* portCombo = new QComboBox(&dialog);
    const auto ports = SerialManager::instance().getAvailablePorts();
    for (const QString& port : ports) {
        portCombo->addItem(port);
    }
    portCombo->setCurrentText(m_portCombo->currentText());

    auto* baudCombo = new QComboBox(&dialog);
    baudCombo->addItems({"9600", "4800"});
    baudCombo->setCurrentText(m_baudCombo->currentText());

    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    layout->addWidget(new QLabel(tr("Port:")));
    layout->addWidget(portCombo);
    layout->addWidget(new QLabel(tr("Baudrate:")));
    layout->addWidget(baudCombo);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, [&]() {
        m_portCombo->setCurrentText(portCombo->currentText());
        m_baudCombo->setCurrentText(baudCombo->currentText());
        dialog.accept();
        onConnect();
    });
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    dialog.exec();
}

void DeviceControlWidget::onConnect()
{
    const QString port = m_portCombo->currentText();
    const int baud = m_baudCombo->currentText().toInt();

    if (SerialManager::instance().openConnection(port, baud)) {
        SettingsManager::instance().savePortSettings(port, baud);
        appendToOutput(tr("Connecting to %1 at %2 baud...").arg(port).arg(baud), "blue");
    } else {
        appendToOutput(tr("Failed to connect to %1").arg(port), "red");
    }
}

void DeviceControlWidget::onDisconnect()
{
    SerialManager::instance().closeConnection();
    appendToOutput(tr("Disconnected"), "orange");
}

void DeviceControlWidget::onConnectionStatusChanged(bool connected)
{
    m_isConnected = connected;

    if (connected) {
        m_connectBtn->setEnabled(false);
        m_disconnectBtn->setEnabled(true);
        m_statusLabel->setText(tr("Connected"));
        m_statusLabel->setStyleSheet("QLabel { color: green; }");
        appendToOutput(tr("Connected to %1").arg(SerialManager::instance().getCurrentPort()), "green");
    } else {
        m_connectBtn->setEnabled(true);
        m_disconnectBtn->setEnabled(false);
        m_statusLabel->setText(tr("Not connected"));
        m_statusLabel->setStyleSheet("QLabel { color: red; }");
    }
}

void DeviceControlWidget::onSendCustomCommand()
{
    if (!m_isConnected) {
        appendToOutput(tr("Not connected"), "red");
        return;
    }

    QString cmd = m_customCommandEdit->text().trimmed();
    if (cmd.isEmpty()) return;

    if (!cmd.endsWith(';') && !cmd.endsWith("\r\n")) {
        cmd += ';';
    }

    SerialManager::instance().sendCommand(cmd.toLatin1());
    appendToOutput(tr(">> %1").arg(cmd), "purple");
    m_customCommandEdit->clear();
}

void DeviceControlWidget::onClearOutput()
{
    m_outputText->clear();
}

void DeviceControlWidget::appendToOutput(const QString& text, const QString& color)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    m_outputText->append(QString("<font color=\"%1\">[%2] %3</font>").arg(color, timestamp, text));
}

void DeviceControlWidget::onGetStatus()
{
    if (!m_isConnected) return;
    SerialManager::instance().sendCommand(CommandFactory::outputStatus(), true);
    appendToOutput(">> OS;", "purple");
}

void DeviceControlWidget::onGetError()
{
    if (!m_isConnected) return;
    SerialManager::instance().sendCommand(CommandFactory::outputError(), true);
    appendToOutput(">> OE;", "purple");
}

void DeviceControlWidget::onGetPosition()
{
    if (!m_isConnected) return;
    SerialManager::instance().sendCommand(CommandFactory::outputCurrentPosition(), true);
    appendToOutput(">> OC;", "purple");
}

void DeviceControlWidget::onGetIdentification()
{
    if (!m_isConnected) return;
    SerialManager::instance().sendCommand(CommandFactory::outputIdentification(), true);
    appendToOutput(">> OI;", "purple");
}

void DeviceControlWidget::onGetP1P2()
{
    if (!m_isConnected) return;
    SerialManager::instance().sendCommand(CommandFactory::outputP1P2(), true);
    appendToOutput(">> OP;", "purple");
}

void DeviceControlWidget::onGetWindow()
{
    if (!m_isConnected) return;
    SerialManager::instance().sendCommand(CommandFactory::outputWindow(), true);
    appendToOutput(">> OW;", "purple");
}

void DeviceControlWidget::onGetHardClip()
{
    if (!m_isConnected) return;
    SerialManager::instance().sendCommand(CommandFactory::outputHardClipLimits(), true);
    appendToOutput(">> OH;", "purple");
}

void DeviceControlWidget::onGetFactor()
{
    if (!m_isConnected) return;
    SerialManager::instance().sendCommand(CommandFactory::outputFactor(), true);
    appendToOutput(">> OF;", "purple");
}

void DeviceControlWidget::onHome()
{
    if (!m_isConnected) return;
    SerialManager::instance().sendCommand(CommandFactory::home());
    appendToOutput(">> H (Home)", "purple");
}

void DeviceControlWidget::onInitialize()
{
    if (!m_isConnected) return;
    SerialManager::instance().sendCommand(CommandFactory::initialize());
    appendToOutput(">> IN;", "purple");
}

void DeviceControlWidget::onEmergencyStop()
{
    QMessageBox::warning(this, tr("Emergency Stop"),
                         tr("Press the EMERGENCY STOP switch on the device.\n"
                            "To resume, release the switch and power cycle the device."));
}

void DeviceControlWidget::onMotorControl(bool on)
{
    if (!m_isConnected) return;
    SerialManager::instance().sendCommand(CommandFactory::motorControl(on));
    appendToOutput(tr(">> !MC %1;").arg(on ? 1 : 0), "purple");
}

void DeviceControlWidget::onSetVelocityXY()
{
    if (!m_isConnected) return;
    int speed = m_xySpeedSpin->value();
    SerialManager::instance().sendCommand(CommandFactory::setVelocityXY(speed));
    appendToOutput(tr(">> VS %1;").arg(speed), "purple");
}

void DeviceControlWidget::onSetZ1Z2()
{
    if (!m_isConnected) return;
    int z1 = m_z1Spin->value();
    int z2 = m_z2Spin->value();
    SerialManager::instance().sendCommand(CommandFactory::setZ1Z2(z1, z2));
    appendToOutput(tr(">> !PZ %1,%2;").arg(z1).arg(z2), "purple");
}

void DeviceControlWidget::onSetWindow()
{
    if (!m_isConnected) return;
    int llx = m_windowLLxSpin->value();
    int lly = m_windowLLySpin->value();
    int urx = m_windowURxSpin->value();
    int ury = m_windowURySpin->value();
    SerialManager::instance().sendCommand(CommandFactory::setWindow(llx, lly, urx, ury));
    appendToOutput(tr(">> IW %1,%2,%3,%4;").arg(llx).arg(lly).arg(urx).arg(ury), "purple");
}

void DeviceControlWidget::onSetScaling()
{
    if (!m_isConnected) return;
    double xmin = m_scalingXminSpin->value();
    double xmax = m_scalingXmaxSpin->value();
    double ymin = m_scalingYminSpin->value();
    double ymax = m_scalingYmaxSpin->value();
    SerialManager::instance().sendCommand(CommandFactory::setScaling(xmin, xmax, ymin, ymax));
    appendToOutput(tr(">> SC %1,%2,%3,%4;").arg(xmin).arg(xmax).arg(ymin).arg(ymax), "purple");
}

void DeviceControlWidget::onAbortBuffer()
{
    if (!m_isConnected) return;
    SerialManager::instance().sendCommand(CommandFactory::abortCAMMGL());
    appendToOutput(">> ESC.K (Abort CAMM-GL, clear buffer)", "purple");
}

void DeviceControlWidget::onResetDeviceControl()
{
    if (!m_isConnected) return;
    SerialManager::instance().sendCommand(CommandFactory::initializeDeviceControl());
    appendToOutput(">> ESC.R (Reset device control settings)", "purple");
}
