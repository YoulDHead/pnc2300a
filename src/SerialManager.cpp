#include "SerialManager.h"
#include <QDebug>

SerialManager::SerialManager() = default;

SerialManager::~SerialManager()
{
    closeConnection();
}

SerialManager& SerialManager::instance()
{
    static SerialManager instance;
    return instance;
}

bool SerialManager::openConnection(const QString& portName, int baudRate)
{
    if (m_serial.isOpen()) {
        m_serial.close();
    }

    m_serial.setPortName(portName);
    m_serial.setBaudRate(baudRate);
    m_serial.setDataBits(QSerialPort::Data8);
    m_serial.setParity(QSerialPort::NoParity);
    m_serial.setStopBits(QSerialPort::OneStop);
    m_serial.setFlowControl(QSerialPort::HardwareControl);

    if (m_serial.open(QIODevice::ReadWrite)) {
        m_currentPort = portName;
        QObject::connect(&m_serial, &QSerialPort::readyRead, this, &SerialManager::onReadyRead);
        QObject::connect(&m_serial, &QSerialPort::errorOccurred, this, &SerialManager::onErrorOccurred);
        emit connectionStatusChanged(true);  // изменено
        return true;
    }

    emit errorOccurred(m_serial.errorString());
    return false;
}

void SerialManager::closeConnection()
{
    if (m_serial.isOpen()) {
        m_serial.close();
    }
    m_currentPort.clear();
    emit connectionStatusChanged(false);  // изменено
}

bool SerialManager::isConnected() const
{
    return m_serial.isOpen();
}

bool SerialManager::sendCommand(const QByteArray& command, bool waitForResponse)
{
    if (!isConnected()) {
        emit errorOccurred("Not connected to device");
        return false;
    }

    qDebug() << "Sending:" << command;

    if (m_serial.write(command) == -1) {
        emit errorOccurred("Failed to write to serial port");
        return false;
    }

    if (!m_serial.waitForBytesWritten(2000)) {
        emit errorOccurred("Timeout writing to serial port");
        return false;
    }

    if (waitForResponse) {
        if (m_serial.waitForReadyRead(3000)) {
            onReadyRead();
        } else {
            emit errorOccurred("Timeout waiting for response");
            return false;
        }
    }

    return true;
}

bool SerialManager::sendRawData(const QByteArray& data)
{
    return sendCommand(data, false);
}

void SerialManager::onReadyRead()
{
    m_buffer.append(m_serial.readAll());

    if (m_buffer.contains('\n') || m_buffer.contains('\r')) {
        emit responseReceived(m_buffer);

        QString resp = QString::fromLatin1(m_buffer).trimmed();
        if (resp.startsWith("OS")) {
            bool ok;
            int status = resp.mid(2).trimmed().toInt(&ok);
            if (ok) emit statusByteReceived(status);
        } else if (resp.startsWith("OE")) {
            bool ok;
            int code = resp.mid(2).trimmed().toInt(&ok);
            if (ok) emit errorCodeReceived(code);
        }

        m_buffer.clear();
    }
}

void SerialManager::onErrorOccurred(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError) {
        emit errorOccurred(m_serial.errorString());
    }
}

QStringList SerialManager::getAvailablePorts() const
{
    QStringList ports;
    const auto availablePorts = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo& port : availablePorts) {
        ports << port.portName();
    }
    return ports;
}

QString SerialManager::getCurrentPort() const
{
    return m_currentPort;
}
