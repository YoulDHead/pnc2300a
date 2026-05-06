#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QByteArray>

class SerialManager : public QObject
{
    Q_OBJECT

public:
    static SerialManager& instance();

    bool openConnection(const QString& portName, int baudRate);
    void closeConnection();
    bool isConnected() const;

    bool sendCommand(const QByteArray& command, bool waitForResponse = false);
    bool sendRawData(const QByteArray& data);

    QStringList getAvailablePorts() const;
    QString getCurrentPort() const;

signals:
    void connectionStatusChanged(bool connected);  // изменено: один сигнал с параметром
    void errorOccurred(const QString& error);
    void responseReceived(const QByteArray& response);
    void statusByteReceived(int status);
    void errorCodeReceived(int errorCode);

private slots:
    void onReadyRead();
    void onErrorOccurred(QSerialPort::SerialPortError error);

private:
    SerialManager();
    ~SerialManager();

    QSerialPort m_serial;
    QString m_currentPort;
    QByteArray m_buffer;
};

#endif // SERIALMANAGER_H
