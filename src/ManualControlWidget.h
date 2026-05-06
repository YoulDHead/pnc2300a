#ifndef MANUALCONTROLWIDGET_H
#define MANUALCONTROLWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QByteArray>

class QPushButton;
class QLabel;
class QGroupBox;
class QSpinBox;
class QCheckBox;
class QLineEdit;
class QTextEdit;

class ManualControlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ManualControlWidget(QWidget *parent = nullptr);

private slots:
    void onJogXLeftPressed();
    void onJogXRightPressed();
    void onJogYUpPressed();
    void onJogYDownPressed();
    void onJogZUpPressed();
    void onJogZDownPressed();
    void onJogStop();
    void onJogTimer();

    void onSpindleTest();
    void onSpindleStop();

    void onGoHome();
    void onGoZ0();
    void onGoZ1();

    void onSendGCode();
    void onLoadGCodeFile();
    void onStopExecution();

    void onUpdatePosition();
    void onPositionResponse(const QByteArray& response);

    void onConnectionStatusChanged(bool connected);

private:
    void setupUi();
    void createJogPanel();
    void createSpindlePanel();
    void createPositionPanel();
    void createGCodePanel();
    void updateJogButtons(bool enabled);
    void updatePositionDisplay(int x, int y, int z, bool toolDown);
    void sendJogCommand(int dx, int dy, int dz);
    void appendToOutput(const QString& text, const QString& color = "black");

    QByteArray translateGCodeToCAMMGL(const QString& gcode, bool absoluteMode, int feedrate);
    QByteArray parseMoveCommand(const QString& gcode, bool rapid, bool absolute);
    QByteArray parseArcCommand(const QString& gcode, bool ccw);

    // Jog controls
    QGroupBox* m_jogPanel;
    QPushButton* m_jogXLeftBtn;
    QPushButton* m_jogXRightBtn;
    QPushButton* m_jogYUpBtn;
    QPushButton* m_jogYDownBtn;
    QPushButton* m_jogZUpBtn;
    QPushButton* m_jogZDownBtn;
    QPushButton* m_jogStopBtn;

    QSpinBox* m_jogStepSpin;
    QCheckBox* m_continuousJogCheck;
    QTimer* m_jogTimer;
    int m_jogDx;
    int m_jogDy;
    int m_jogDz;

    // Spindle controls
    QGroupBox* m_spindlePanel;
    QPushButton* m_spindleTestBtn;
    QPushButton* m_spindleStopBtn;
    QSpinBox* m_spindleSpeedSpin;
    QLabel* m_spindleStatusLabel;

    // Position display
    QGroupBox* m_positionPanel;
    QLabel* m_posXLabel;
    QLabel* m_posYLabel;
    QLabel* m_posZLabel;
    QLabel* m_toolStatusLabel;
    QPushButton* m_updatePosBtn;
    QPushButton* m_homeBtn;
    QPushButton* m_z0Btn;
    QPushButton* m_z1Btn;

    // G-code panel
    QGroupBox* m_gCodePanel;
    QTextEdit* m_gCodeEdit;
    QPushButton* m_sendGCodeBtn;
    QPushButton* m_loadFileBtn;
    QPushButton* m_stopBtn;
    QCheckBox* m_absoluteModeCheck;
    QSpinBox* m_feedrateSpin;

    // State
    bool m_isConnected;
    int m_currentX;
    int m_currentY;
    int m_currentZ;
    bool m_toolDown;
    bool m_isExecuting;
};

#endif // MANUALCONTROLWIDGET_H
