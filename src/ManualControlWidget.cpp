#include "ManualControlWidget.h"
#include "SerialManager.h"
#include "CommandFactory.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QThread>
#include <QRegularExpression>
#include <QDateTime>

ManualControlWidget::ManualControlWidget(QWidget *parent)
    : QWidget(parent)
    , m_isConnected(false)
    , m_currentX(0)
    , m_currentY(0)
    , m_currentZ(0)
    , m_toolDown(false)
    , m_isExecuting(false)
    , m_jogDx(0)
    , m_jogDy(0)
    , m_jogDz(0)
{
    setupUi();

    m_jogTimer = new QTimer(this);
    m_jogTimer->setInterval(50);
    connect(m_jogTimer, &QTimer::timeout, this, &ManualControlWidget::onJogTimer);

    auto& serial = SerialManager::instance();
    connect(&serial, &SerialManager::connectionStatusChanged,
            this, &ManualControlWidget::onConnectionStatusChanged);
    connect(&serial, &SerialManager::responseReceived,
            this, &ManualControlWidget::onPositionResponse);
}

void ManualControlWidget::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);

    createJogPanel();
    createSpindlePanel();
    createPositionPanel();
    createGCodePanel();

    mainLayout->addWidget(m_jogPanel);
    mainLayout->addWidget(m_spindlePanel);
    mainLayout->addWidget(m_positionPanel);
    mainLayout->addWidget(m_gCodePanel);

    updateJogButtons(false);
}

void ManualControlWidget::createJogPanel()
{
    m_jogPanel = new QGroupBox(tr("Manual Jog Control"), this);
    auto* layout = new QVBoxLayout(m_jogPanel);

    auto* buttonLayout = new QGridLayout();

    m_jogYUpBtn = new QPushButton(tr("▲ Y+"), this);
    m_jogYUpBtn->setFixedSize(80, 60);
    m_jogXLeftBtn = new QPushButton(tr("◄ X-"), this);
    m_jogXLeftBtn->setFixedSize(80, 60);
    m_jogStopBtn = new QPushButton(tr("STOP"), this);
    m_jogStopBtn->setFixedSize(80, 60);
    m_jogStopBtn->setStyleSheet("QPushButton { background-color: red; color: white; font-weight: bold; }");
    m_jogXRightBtn = new QPushButton(tr("X+ ►"), this);
    m_jogXRightBtn->setFixedSize(80, 60);
    m_jogYDownBtn = new QPushButton(tr("▼ Y-"), this);
    m_jogYDownBtn->setFixedSize(80, 60);

    m_jogZUpBtn = new QPushButton(tr("Z+ ▲"), this);
    m_jogZUpBtn->setFixedSize(80, 60);
    m_jogZDownBtn = new QPushButton(tr("Z- ▼"), this);
    m_jogZDownBtn->setFixedSize(80, 60);

    buttonLayout->addWidget(m_jogYUpBtn, 0, 1);
    buttonLayout->addWidget(m_jogXLeftBtn, 1, 0);
    buttonLayout->addWidget(m_jogStopBtn, 1, 1);
    buttonLayout->addWidget(m_jogXRightBtn, 1, 2);
    buttonLayout->addWidget(m_jogYDownBtn, 2, 1);
    buttonLayout->addWidget(m_jogZUpBtn, 3, 0);
    buttonLayout->addWidget(m_jogZDownBtn, 3, 2);

    auto* settingsLayout = new QHBoxLayout();
    settingsLayout->addWidget(new QLabel(tr("Step (steps):"), this));
    m_jogStepSpin = new QSpinBox(this);
    m_jogStepSpin->setRange(1, 10000);
    m_jogStepSpin->setValue(100);
    m_jogStepSpin->setSuffix(tr(" steps"));
    settingsLayout->addWidget(m_jogStepSpin);

    m_continuousJogCheck = new QCheckBox(tr("Continuous (hold for movement)"), this);
    m_continuousJogCheck->setChecked(true);
    settingsLayout->addWidget(m_continuousJogCheck);
    settingsLayout->addStretch();

    layout->addLayout(buttonLayout);
    layout->addLayout(settingsLayout);

    connect(m_jogXLeftBtn, &QPushButton::pressed, this, &ManualControlWidget::onJogXLeftPressed);
    connect(m_jogXLeftBtn, &QPushButton::released, this, &ManualControlWidget::onJogStop);
    connect(m_jogXRightBtn, &QPushButton::pressed, this, &ManualControlWidget::onJogXRightPressed);
    connect(m_jogXRightBtn, &QPushButton::released, this, &ManualControlWidget::onJogStop);
    connect(m_jogYUpBtn, &QPushButton::pressed, this, &ManualControlWidget::onJogYUpPressed);
    connect(m_jogYUpBtn, &QPushButton::released, this, &ManualControlWidget::onJogStop);
    connect(m_jogYDownBtn, &QPushButton::pressed, this, &ManualControlWidget::onJogYDownPressed);
    connect(m_jogYDownBtn, &QPushButton::released, this, &ManualControlWidget::onJogStop);
    connect(m_jogZUpBtn, &QPushButton::pressed, this, &ManualControlWidget::onJogZUpPressed);
    connect(m_jogZUpBtn, &QPushButton::released, this, &ManualControlWidget::onJogStop);
    connect(m_jogZDownBtn, &QPushButton::pressed, this, &ManualControlWidget::onJogZDownPressed);
    connect(m_jogZDownBtn, &QPushButton::released, this, &ManualControlWidget::onJogStop);
    connect(m_jogStopBtn, &QPushButton::clicked, this, &ManualControlWidget::onJogStop);
}

void ManualControlWidget::createSpindlePanel()
{
    m_spindlePanel = new QGroupBox(tr("Spindle Control"), this);
    auto* layout = new QHBoxLayout(m_spindlePanel);

    m_spindleTestBtn = new QPushButton(tr("SPINDLE TEST ON"), this);
    m_spindleTestBtn->setStyleSheet("QPushButton { background-color: green; color: white; }");
    m_spindleStopBtn = new QPushButton(tr("SPINDLE OFF"), this);
    m_spindleStopBtn->setStyleSheet("QPushButton { background-color: red; color: white; }");

    layout->addWidget(new QLabel(tr("Speed (RPM):"), this));
    m_spindleSpeedSpin = new QSpinBox(this);
    m_spindleSpeedSpin->setRange(5000, 15000);
    m_spindleSpeedSpin->setValue(10000);
    m_spindleSpeedSpin->setSingleStep(500);
    m_spindleSpeedSpin->setSuffix(tr(" RPM"));
    layout->addWidget(m_spindleSpeedSpin);

    layout->addWidget(m_spindleTestBtn);
    layout->addWidget(m_spindleStopBtn);

    m_spindleStatusLabel = new QLabel(tr("Spindle: OFF"), this);
    m_spindleStatusLabel->setStyleSheet("QLabel { color: red; }");
    layout->addWidget(m_spindleStatusLabel);
    layout->addStretch();

    connect(m_spindleTestBtn, &QPushButton::clicked, this, &ManualControlWidget::onSpindleTest);
    connect(m_spindleStopBtn, &QPushButton::clicked, this, &ManualControlWidget::onSpindleStop);
}

void ManualControlWidget::createPositionPanel()
{
    m_positionPanel = new QGroupBox(tr("Position & Status"), this);
    auto* layout = new QGridLayout(m_positionPanel);

    layout->addWidget(new QLabel(tr("X:"), this), 0, 0);
    m_posXLabel = new QLabel(tr("0 steps (0.00 mm)"), this);
    m_posXLabel->setFont(QFont("Consolas", 10));
    layout->addWidget(m_posXLabel, 0, 1);

    layout->addWidget(new QLabel(tr("Y:"), this), 1, 0);
    m_posYLabel = new QLabel(tr("0 steps (0.00 mm)"), this);
    m_posYLabel->setFont(QFont("Consolas", 10));
    layout->addWidget(m_posYLabel, 1, 1);

    layout->addWidget(new QLabel(tr("Z:"), this), 2, 0);
    m_posZLabel = new QLabel(tr("0 steps (0.00 mm)"), this);
    m_posZLabel->setFont(QFont("Consolas", 10));
    layout->addWidget(m_posZLabel, 2, 1);

    layout->addWidget(new QLabel(tr("Tool:"), this), 3, 0);
    m_toolStatusLabel = new QLabel(tr("UP"), this);
    m_toolStatusLabel->setFont(QFont("Consolas", 10));
    m_toolStatusLabel->setStyleSheet("QLabel { color: blue; }");
    layout->addWidget(m_toolStatusLabel, 3, 1);

    auto* buttonLayout = new QHBoxLayout();
    m_updatePosBtn = new QPushButton(tr("Update Position"), this);
    m_homeBtn = new QPushButton(tr("Go Home (XY)"), this);
    m_z0Btn = new QPushButton(tr("Go Z0"), this);
    m_z1Btn = new QPushButton(tr("Go Z1 (Cut depth)"), this);

    buttonLayout->addWidget(m_updatePosBtn);
    buttonLayout->addWidget(m_homeBtn);
    buttonLayout->addWidget(m_z0Btn);
    buttonLayout->addWidget(m_z1Btn);

    layout->addLayout(buttonLayout, 4, 0, 1, 2);

    connect(m_updatePosBtn, &QPushButton::clicked, this, &ManualControlWidget::onUpdatePosition);
    connect(m_homeBtn, &QPushButton::clicked, this, &ManualControlWidget::onGoHome);
    connect(m_z0Btn, &QPushButton::clicked, this, &ManualControlWidget::onGoZ0);
    connect(m_z1Btn, &QPushButton::clicked, this, &ManualControlWidget::onGoZ1);
}

void ManualControlWidget::createGCodePanel()
{
    m_gCodePanel = new QGroupBox(tr("G-Code / Custom Commands"), this);
    auto* layout = new QVBoxLayout(m_gCodePanel);

    auto* settingsLayout = new QHBoxLayout();
    m_absoluteModeCheck = new QCheckBox(tr("Absolute Mode (G90)"), this);
    m_absoluteModeCheck->setChecked(true);
    settingsLayout->addWidget(m_absoluteModeCheck);

    settingsLayout->addWidget(new QLabel(tr("Feedrate (mm/sec):"), this));
    m_feedrateSpin = new QSpinBox(this);
    m_feedrateSpin->setRange(1, 60);
    m_feedrateSpin->setValue(10);
    settingsLayout->addWidget(m_feedrateSpin);
    settingsLayout->addStretch();
    layout->addLayout(settingsLayout);

    m_gCodeEdit = new QTextEdit(this);
    m_gCodeEdit->setPlaceholderText(tr(
        "Enter commands here. One per line.\n"
        "Supports:\n"
        "  G00 X100 Y200  - Rapid positioning\n"
        "  G01 X100 Y200  - Linear interpolation\n"
        "  G90            - Absolute mode\n"
        "  G91            - Relative mode\n"
        "  M03            - Spindle ON\n"
        "  M05            - Spindle OFF\n"
        "  M00            - Stop\n"
        "  F10            - Set feedrate to 10 mm/sec\n"
        "\nOr direct CAMM-GL commands:\n"
        "  PA5000,5000;   - Move to absolute position\n"
        "  PD5000,5000;   - Draw to position\n"
        "  PU;            - Pen up\n"
        "  PD;            - Pen down\n"
        "  CI1000;        - Draw circle radius 1000\n"
        ));
    m_gCodeEdit->setFont(QFont("Consolas", 10));
    layout->addWidget(m_gCodeEdit);

    auto* buttonLayout = new QHBoxLayout();
    m_loadFileBtn = new QPushButton(tr("Load G-Code File..."), this);
    m_sendGCodeBtn = new QPushButton(tr("Send Commands"), this);
    m_stopBtn = new QPushButton(tr("STOP"), this);
    m_stopBtn->setStyleSheet("QPushButton { background-color: red; color: white; }");

    buttonLayout->addWidget(m_loadFileBtn);
    buttonLayout->addWidget(m_sendGCodeBtn);
    buttonLayout->addWidget(m_stopBtn);
    layout->addLayout(buttonLayout);

    connect(m_loadFileBtn, &QPushButton::clicked, this, &ManualControlWidget::onLoadGCodeFile);
    connect(m_sendGCodeBtn, &QPushButton::clicked, this, &ManualControlWidget::onSendGCode);
    connect(m_stopBtn, &QPushButton::clicked, this, &ManualControlWidget::onStopExecution);
}

void ManualControlWidget::onJogXLeftPressed() { sendJogCommand(-m_jogStepSpin->value(), 0, 0); }
void ManualControlWidget::onJogXRightPressed() { sendJogCommand(m_jogStepSpin->value(), 0, 0); }
void ManualControlWidget::onJogYUpPressed() { sendJogCommand(0, m_jogStepSpin->value(), 0); }
void ManualControlWidget::onJogYDownPressed() { sendJogCommand(0, -m_jogStepSpin->value(), 0); }
void ManualControlWidget::onJogZUpPressed() { sendJogCommand(0, 0, m_jogStepSpin->value()); }
void ManualControlWidget::onJogZDownPressed() { sendJogCommand(0, 0, -m_jogStepSpin->value()); }

void ManualControlWidget::sendJogCommand(int dx, int dy, int dz)
{
    if (!m_isConnected) {
        QMessageBox::warning(this, tr("Not Connected"), tr("Please connect to the device first."));
        return;
    }

    if (m_continuousJogCheck->isChecked()) {
        m_jogDx = dx;
        m_jogDy = dy;
        m_jogDz = dz;
        if (!m_jogTimer->isActive()) {
            m_jogTimer->start();
        }
    } else {
        if (dx != 0 || dy != 0) {
            SerialManager::instance().sendCommand(CommandFactory::plotRelative(dx, dy));
        }
        if (dz != 0) {
            appendToOutput(tr("Z jog: %1 steps (use manual panel for precise Z control)").arg(dz), "orange");
        }
    }
}

void ManualControlWidget::onJogStop()
{
    m_jogTimer->stop();
    m_jogDx = m_jogDy = m_jogDz = 0;
}

void ManualControlWidget::onJogTimer()
{
    if (!m_isConnected) return;

    if (m_jogDx != 0 || m_jogDy != 0) {
        SerialManager::instance().sendCommand(CommandFactory::plotRelative(m_jogDx, m_jogDy));
    }
}

void ManualControlWidget::onSpindleTest()
{
    if (!m_isConnected) return;
    SerialManager::instance().sendCommand(CommandFactory::motorControl(true));
    m_spindleStatusLabel->setText(tr("Spindle: ON"));
    m_spindleStatusLabel->setStyleSheet("QLabel { color: green; }");
    appendToOutput("Spindle ON", "green");
}

void ManualControlWidget::onSpindleStop()
{
    if (!m_isConnected) return;
    SerialManager::instance().sendCommand(CommandFactory::motorControl(false));
    m_spindleStatusLabel->setText(tr("Spindle: OFF"));
    m_spindleStatusLabel->setStyleSheet("QLabel { color: red; }");
    appendToOutput("Spindle OFF", "red");
}

void ManualControlWidget::onGoHome()
{
    if (!m_isConnected) return;
    SerialManager::instance().sendCommand(CommandFactory::home());
    appendToOutput("Go Home", "purple");
}

void ManualControlWidget::onGoZ0()
{
    if (!m_isConnected) return;
    appendToOutput("Go Z0 - use manual panel or software settings", "orange");
}

void ManualControlWidget::onGoZ1()
{
    if (!m_isConnected) return;
    appendToOutput("Go Z1 - use manual panel or software settings", "orange");
}

void ManualControlWidget::onUpdatePosition()
{
    if (!m_isConnected) return;
    SerialManager::instance().sendCommand(CommandFactory::outputCurrentPosition(), true);
}

void ManualControlWidget::onPositionResponse(const QByteArray& response)
{
    QString resp = QString::fromLatin1(response).trimmed();
    auto parts = resp.split(',');
    if (parts.size() >= 3) {
        bool okX, okY, okT;
        int x = parts[0].toInt(&okX);
        int y = parts[1].toInt(&okY);
        int t = parts[2].toInt(&okT);

        if (okX && okY) {
            m_currentX = x;
            m_currentY = y;
            updatePositionDisplay(x, y, m_currentZ, t == 1);
        }
        if (okT) {
            m_toolDown = (t == 1);
            m_toolStatusLabel->setText(m_toolDown ? tr("DOWN (engraving)") : tr("UP"));
            m_toolStatusLabel->setStyleSheet(m_toolDown ?
                                                 "QLabel { color: red; }" : "QLabel { color: blue; }");
        }
    }
}

void ManualControlWidget::updatePositionDisplay(int x, int y, int z, bool toolDown)
{
    Q_UNUSED(toolDown);
    double x_mm = x * 0.01;
    double y_mm = y * 0.01;
    double z_mm = z * 0.01;

    m_posXLabel->setText(QString("%1 steps (%2 mm)").arg(x).arg(x_mm, 0, 'f', 2));
    m_posYLabel->setText(QString("%1 steps (%2 mm)").arg(y).arg(y_mm, 0, 'f', 2));
    m_posZLabel->setText(QString("%1 steps (%2 mm)").arg(z).arg(z_mm, 0, 'f', 2));
}

void ManualControlWidget::appendToOutput(const QString& text, const QString& color)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    // Output to status bar - can be extended to a log widget
    qDebug() << "[ManualControl]" << text;
}

void ManualControlWidget::onSendGCode()
{
    if (!m_isConnected) {
        QMessageBox::warning(this, tr("Not Connected"), tr("Please connect to the device first."));
        return;
    }

    QString text = m_gCodeEdit->toPlainText();
    if (text.trimmed().isEmpty()) return;

    m_isExecuting = true;
    m_sendGCodeBtn->setEnabled(false);
    m_stopBtn->setEnabled(true);

    QStringList lines = text.split('\n', Qt::SkipEmptyParts);
    bool absoluteMode = m_absoluteModeCheck->isChecked();
    int feedrate = m_feedrateSpin->value();

    SerialManager::instance().sendCommand(CommandFactory::initialize());
    QThread::msleep(100);
    SerialManager::instance().sendCommand(CommandFactory::setVelocityXY(feedrate));
    QThread::msleep(50);

    if (absoluteMode) {
        SerialManager::instance().sendCommand(CommandFactory::plotAbsolute(0, 0));
    } else {
        SerialManager::instance().sendCommand(CommandFactory::plotRelative(0, 0));
    }

    for (const QString& line : lines) {
        if (!m_isExecuting) break;

        QString cmd = line.trimmed().toUpper();
        if (cmd.isEmpty() || cmd.startsWith('(') || cmd.startsWith(';')) continue;

        QByteArray cammCmd = translateGCodeToCAMMGL(cmd, absoluteMode, feedrate);
        if (!cammCmd.isEmpty()) {
            SerialManager::instance().sendCommand(cammCmd);
            QThread::msleep(10);
        }
    }

    m_isExecuting = false;
    m_sendGCodeBtn->setEnabled(true);
    m_stopBtn->setEnabled(false);

    QMessageBox::information(this, tr("Complete"), tr("Command execution finished."));
}

void ManualControlWidget::onLoadGCodeFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open G-Code File"),
                                                    QString(),
                                                    tr("G-Code Files (*.nc *.ngc *.gcode *.txt);;All Files (*)"));

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            m_gCodeEdit->setPlainText(stream.readAll());
            file.close();
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Cannot open file: %1").arg(file.errorString()));
        }
    }
}

void ManualControlWidget::onStopExecution()
{
    if (m_isExecuting) {
        m_isExecuting = false;
        SerialManager::instance().sendCommand(CommandFactory::abortCAMMGL());
        SerialManager::instance().sendCommand(CommandFactory::home());
        QMessageBox::information(this, tr("Stopped"), tr("Command execution stopped."));
    }
}

QByteArray ManualControlWidget::translateGCodeToCAMMGL(const QString& gcode, bool absoluteMode, int feedrate)
{
    Q_UNUSED(feedrate);

    if (gcode.startsWith("G00") || gcode.startsWith("G0")) {
        return parseMoveCommand(gcode, true, absoluteMode);
    }
    else if (gcode.startsWith("G01") || gcode.startsWith("G1")) {
        return parseMoveCommand(gcode, false, absoluteMode);
    }
    else if (gcode.startsWith("G02") || gcode.startsWith("G2")) {
        return parseArcCommand(gcode, false);
    }
    else if (gcode.startsWith("G03") || gcode.startsWith("G3")) {
        return parseArcCommand(gcode, true);
    }
    else if (gcode == "G90") {
        return CommandFactory::plotAbsolute(0, 0);
    }
    else if (gcode == "G91") {
        return CommandFactory::plotRelative(0, 0);
    }
    else if (gcode == "M03") {
        return CommandFactory::motorControl(true);
    }
    else if (gcode == "M05") {
        return CommandFactory::motorControl(false);
    }
    else if (gcode == "M00") {
        return "!NR;";
    }
    else if (gcode.startsWith("F")) {
        int newFeedrate = gcode.mid(1).toInt();
        if (newFeedrate > 0) {
            return CommandFactory::setVelocityXY(newFeedrate);
        }
    }
    else if (!gcode.isEmpty() && (gcode.contains(';') || gcode.contains("PA") || gcode.contains("PR") ||
                                  gcode.contains("PU") || gcode.contains("PD") || gcode.contains("CI") ||
                                  gcode.contains("AA") || gcode.contains("EA") || gcode.contains("RA") ||
                                  gcode.contains("LB") || gcode.contains("VS") || gcode.contains("!VZ") ||
                                  gcode.contains("!MC") || gcode.contains("!PZ") || gcode.contains("IW") ||
                                  gcode.contains("SC") || gcode.contains("LT") || gcode.contains("FT"))) {
        QByteArray cmd = gcode.toLatin1();
        if (!cmd.endsWith(';')) cmd += ';';
        return cmd;
    }

    return QByteArray();
}

QByteArray ManualControlWidget::parseMoveCommand(const QString& gcode, bool rapid, bool absolute)
{
    int x = -1, y = -1, z = -1;

    QRegularExpression xRegex("X([+-]?[0-9.]+)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression yRegex("Y([+-]?[0-9.]+)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression zRegex("Z([+-]?[0-9.]+)", QRegularExpression::CaseInsensitiveOption);

    QRegularExpressionMatch match;

    match = xRegex.match(gcode);
    if (match.hasMatch()) {
        double val = match.captured(1).toDouble();
        x = qRound(val / 0.01);
    }

    match = yRegex.match(gcode);
    if (match.hasMatch()) {
        double val = match.captured(1).toDouble();
        y = qRound(val / 0.01);
    }

    match = zRegex.match(gcode);
    if (match.hasMatch()) {
        double val = match.captured(1).toDouble();
        z = qRound(val / 0.01);
    }

    QByteArray result;

    if (z != -1) {
        if (z < 0) {
            result += CommandFactory::setZ1Z2(z, 0);
        } else {
            result += CommandFactory::setZ0(z);
        }
        result += '\n';
    }

    if (x != -1 || y != -1) {
        if (absolute) {
            int absX = (x != -1) ? x : m_currentX;
            int absY = (y != -1) ? y : m_currentY;
            if (rapid) {
                result += CommandFactory::plotAbsolute(absX, absY);
            } else {
                result += CommandFactory::penDown();
                result += CommandFactory::plotAbsolute(absX, absY);
                result += CommandFactory::penUp();
            }
        } else {
            int relX = (x != -1) ? x : 0;
            int relY = (y != -1) ? y : 0;
            if (rapid) {
                result += CommandFactory::plotRelative(relX, relY);
            } else {
                result += CommandFactory::penDown();
                result += CommandFactory::plotRelative(relX, relY);
                result += CommandFactory::penUp();
            }
        }
    }

    return result;
}

QByteArray ManualControlWidget::parseArcCommand(const QString& gcode, bool ccw)
{
    Q_UNUSED(ccw);

    double r = -1;

    QRegularExpression rRegex("R([+-]?[0-9.]+)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match;

    match = rRegex.match(gcode);
    if (match.hasMatch()) {
        r = match.captured(1).toDouble() / 0.01;
    }

    if (r > 0) {
        return CommandFactory::circle(r);
    }

    return QByteArray();
}

void ManualControlWidget::onConnectionStatusChanged(bool connected)
{
    m_isConnected = connected;
    updateJogButtons(connected);

    if (connected) {
        onUpdatePosition();
        appendToOutput("Connected to PNC-2300A", "green");
    } else {
        appendToOutput("Disconnected from PNC-2300A", "red");
    }
}

void ManualControlWidget::updateJogButtons(bool enabled)
{
    m_jogXLeftBtn->setEnabled(enabled);
    m_jogXRightBtn->setEnabled(enabled);
    m_jogYUpBtn->setEnabled(enabled);
    m_jogYDownBtn->setEnabled(enabled);
    m_jogZUpBtn->setEnabled(enabled);
    m_jogZDownBtn->setEnabled(enabled);
    m_jogStopBtn->setEnabled(enabled);
    m_spindleTestBtn->setEnabled(enabled);
    m_spindleStopBtn->setEnabled(enabled);
    m_updatePosBtn->setEnabled(enabled);
    m_homeBtn->setEnabled(enabled);
    m_z0Btn->setEnabled(enabled);
    m_z1Btn->setEnabled(enabled);
    m_sendGCodeBtn->setEnabled(enabled);
    m_loadFileBtn->setEnabled(enabled);
}
