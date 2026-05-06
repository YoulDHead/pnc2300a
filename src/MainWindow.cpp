#include "MainWindow.h"
#include "DeviceControlWidget.h"
#include "ManualControlWidget.h"
#include "SerialManager.h"
#include "SettingsManager.h"
#include "CommandFactory.h"

#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QTimer>
#include <QTabWidget>
#include <QLabel>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_isConnected(false)
{
    setupUi();
    setupToolBar();
    setupStatusBar();
    createMenus();
    loadSettings();

    auto& serial = SerialManager::instance();
    // Исправлено: подключаем к одному сигналу с параметром bool
    QObject::connect(&serial, &SerialManager::connectionStatusChanged,
                     this, &MainWindow::onConnectionStatusChanged);
    QObject::connect(&serial, &SerialManager::errorOccurred,
                     this, &MainWindow::onSerialError);
    QObject::connect(&serial, &SerialManager::responseReceived,
                     this, &MainWindow::onResponseReceived);

    setWindowTitle("CAMM-Grinder - PNC-2300A Controller");
    resize(1000, 700);
}

MainWindow::~MainWindow()
{
    saveSettings();
    if (m_isConnected) {
        SerialManager::instance().closeConnection();
    }
}

void MainWindow::setupUi()
{
    auto* tabWidget = new QTabWidget(this);

    m_deviceControlWidget = new DeviceControlWidget(this);
    m_manualControlWidget = new ManualControlWidget(this);

    tabWidget->addTab(m_deviceControlWidget, tr("Device Control"));
    tabWidget->addTab(m_manualControlWidget, tr("Manual Control"));

    setCentralWidget(tabWidget);
}

void MainWindow::setupToolBar()
{
    auto* toolBar = addToolBar(tr("Main"));
    toolBar->setMovable(false);

    m_connectAction = new QAction(tr("Connect"), this);
    m_connectAction->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
    connect(m_connectAction, &QAction::triggered, this, &MainWindow::onConnect);

    m_disconnectAction = new QAction(tr("Disconnect"), this);
    m_disconnectAction->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
    m_disconnectAction->setEnabled(false);
    connect(m_disconnectAction, &QAction::triggered, this, &MainWindow::onDisconnect);

    toolBar->addSeparator();

    m_homeAction = new QAction(tr("Home"), this);
    m_homeAction->setIcon(style()->standardIcon(QStyle::SP_DialogResetButton));
    m_homeAction->setEnabled(false);
    connect(m_homeAction, &QAction::triggered, [this]() {
        if (m_isConnected) {
            SerialManager::instance().sendCommand(CommandFactory::home());
        }
    });

    m_emergencyStopAction = new QAction(tr("Emergency Stop"), this);
    m_emergencyStopAction->setIcon(style()->standardIcon(QStyle::SP_BrowserStop));
    m_emergencyStopAction->setEnabled(false);
    connect(m_emergencyStopAction, &QAction::triggered, [this]() {
        QMessageBox::warning(this, tr("Emergency Stop"),
                             tr("Press the EMERGENCY STOP switch on the device.\n"
                                "To resume, release the switch and power cycle the device."));
    });

    toolBar->addAction(m_connectAction);
    toolBar->addAction(m_disconnectAction);
    toolBar->addSeparator();
    toolBar->addAction(m_homeAction);
    toolBar->addAction(m_emergencyStopAction);
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel(tr("Ready"));
    m_connectionLabel = new QLabel(tr("Disconnected"));
    m_connectionLabel->setStyleSheet("QLabel { color: red; }");

    statusBar()->addWidget(m_statusLabel);
    statusBar()->addPermanentWidget(m_connectionLabel);
}

void MainWindow::createMenus()
{
    auto* fileMenu = menuBar()->addMenu(tr("&File"));

    auto* exitAction = fileMenu->addAction(tr("E&xit"));
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    auto* helpMenu = menuBar()->addMenu(tr("&Help"));

    auto* aboutAction = helpMenu->addAction(tr("&About"));
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, tr("About CAMM-Grinder"),
                           tr("CAMM-Grinder - Controller for Roland PNC-2300A\n\n"
                              "Protocol: CAMM-GL I / Mode 2\n"
                              "Interface: RS-232C Serial\n\n"
                              "Supported commands: PA, PR, PU, PD, VS, !VZ, !MC, !PZ, IN, H, OS, OE, OC, OA, OP, OW, OH, OF, OI\n"
                              "Device Control: ESC.B, ESC.J, ESC.K, ESC.R"));
    });
}

void MainWindow::onConnect()
{
    m_deviceControlWidget->showPortSettings();
}

void MainWindow::onDisconnect()
{
    SerialManager::instance().closeConnection();
}

void MainWindow::onConnectionStatusChanged(bool connected)
{
    m_isConnected = connected;

    if (connected) {
        m_connectionLabel->setText(tr("Connected to %1").arg(SerialManager::instance().getCurrentPort()));
        m_connectionLabel->setStyleSheet("QLabel { color: green; }");
        m_connectAction->setEnabled(false);
        m_disconnectAction->setEnabled(true);
        m_homeAction->setEnabled(true);
        m_emergencyStopAction->setEnabled(true);
        m_statusLabel->setText(tr("Connected. Sending initialization..."));

        QTimer::singleShot(500, [this]() {
            SerialManager::instance().sendCommand(CommandFactory::initialize());
            m_statusLabel->setText(tr("Ready"));
        });
    } else {
        m_connectionLabel->setText(tr("Disconnected"));
        m_connectionLabel->setStyleSheet("QLabel { color: red; }");
        m_connectAction->setEnabled(true);
        m_disconnectAction->setEnabled(false);
        m_homeAction->setEnabled(false);
        m_emergencyStopAction->setEnabled(false);
        m_statusLabel->setText(tr("Disconnected"));
    }
}

void MainWindow::onSerialError(const QString& error)
{
    m_statusLabel->setText(tr("Error: %1").arg(error));
    QMessageBox::warning(this, tr("Serial Error"), error);
}

void MainWindow::onResponseReceived(const QByteArray& response)
{
    m_statusLabel->setText(tr("Response: %1").arg(QString::fromLatin1(response).trimmed()));
    QTimer::singleShot(2000, [this]() {
        if (m_statusLabel->text().startsWith("Response:")) {
            m_statusLabel->setText(tr("Ready"));
        }
    });
}

void MainWindow::loadSettings()
{
    auto geometry = SettingsManager::instance().loadWindowGeometry();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
}

void MainWindow::saveSettings()
{
    SettingsManager::instance().saveWindowGeometry(saveGeometry());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();
    event->accept();
}
