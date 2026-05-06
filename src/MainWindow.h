#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class DeviceControlWidget;
class ManualControlWidget;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onConnect();
    void onDisconnect();
    void onConnectionStatusChanged(bool connected);
    void onSerialError(const QString& error);
    void onResponseReceived(const QByteArray& response);

private:
    void setupUi();
    void setupToolBar();
    void setupStatusBar();
    void createMenus();
    void loadSettings();
    void saveSettings();

    DeviceControlWidget* m_deviceControlWidget;
    ManualControlWidget* m_manualControlWidget;

    QAction* m_connectAction;
    QAction* m_disconnectAction;
    QAction* m_homeAction;
    QAction* m_emergencyStopAction;

    QLabel* m_statusLabel;
    QLabel* m_connectionLabel;

    bool m_isConnected;
};

#endif // MAINWINDOW_H
