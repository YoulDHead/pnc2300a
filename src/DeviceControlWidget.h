#ifndef DEVICECONTROLWIDGET_H
#define DEVICECONTROLWIDGET_H

#include <QWidget>
#include <QGroupBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLabel>

class DeviceControlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceControlWidget(QWidget *parent = nullptr);

public slots:
    void showPortSettings();

private slots:
    void onConnect();
    void onDisconnect();
    void onSendCustomCommand();
    void onClearOutput();
    void onGetStatus();
    void onGetError();
    void onGetPosition();
    void onGetIdentification();
    void onGetP1P2();
    void onGetWindow();
    void onGetHardClip();
    void onGetFactor();
    void onHome();
    void onInitialize();
    void onEmergencyStop();
    void onMotorControl(bool on);
    void onSetVelocityXY();
    void onSetZ1Z2();
    void onSetWindow();
    void onSetScaling();
    void onAbortBuffer();
    void onResetDeviceControl();
    void onConnectionStatusChanged(bool connected);

private:
    void setupUi();
    void createConnectionGroup();
    void createCommandGroup();
    void createOutputGroup();
    void createConfigGroup();
    void createCustomCommandGroup();
    void appendToOutput(const QString& text, const QString& color = "black");

    QGroupBox* m_connectionGroup;
    QComboBox* m_portCombo;
    QComboBox* m_baudCombo;
    QPushButton* m_connectBtn;
    QPushButton* m_disconnectBtn;
    QLabel* m_statusLabel;

    QGroupBox* m_commandGroup;
    QPushButton* m_homeBtn;
    QPushButton* m_initBtn;
    QPushButton* m_emergencyBtn;
    QPushButton* m_getStatusBtn;
    QPushButton* m_getErrorBtn;
    QPushButton* m_getPositionBtn;
    QPushButton* m_getIdentificationBtn;
    QPushButton* m_getP1P2Btn;
    QPushButton* m_getWindowBtn;
    QPushButton* m_getHardClipBtn;
    QPushButton* m_getFactorBtn;
    QPushButton* m_abortBufferBtn;
    QPushButton* m_resetDeviceBtn;

    QGroupBox* m_configGroup;
    QCheckBox* m_motorControlCheck;
    QSpinBox* m_xySpeedSpin;
    QSpinBox* m_zSpeedSpin;
    QSpinBox* m_z1Spin;
    QSpinBox* m_z2Spin;
    QPushButton* m_applyVelocityBtn;
    QPushButton* m_applyZ1Z2Btn;

    QSpinBox* m_windowLLxSpin;
    QSpinBox* m_windowLLySpin;
    QSpinBox* m_windowURxSpin;
    QSpinBox* m_windowURySpin;
    QPushButton* m_applyWindowBtn;

    QDoubleSpinBox* m_scalingXminSpin;
    QDoubleSpinBox* m_scalingXmaxSpin;
    QDoubleSpinBox* m_scalingYminSpin;
    QDoubleSpinBox* m_scalingYmaxSpin;
    QPushButton* m_applyScalingBtn;

    QGroupBox* m_outputGroup;
    QTextEdit* m_outputText;
    QPushButton* m_clearOutputBtn;

    QGroupBox* m_customCommandGroup;
    QLineEdit* m_customCommandEdit;
    QPushButton* m_sendCustomBtn;

    bool m_isConnected;
};

#endif // DEVICECONTROLWIDGET_H
