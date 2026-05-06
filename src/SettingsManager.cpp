#include "SettingsManager.h"
#include <QCoreApplication>

SettingsManager::SettingsManager()
    : m_settings(QSettings::IniFormat, QSettings::UserScope,
                 "CNC Tools", "CAMM-Grinder")
{
}

SettingsManager& SettingsManager::instance()
{
    static SettingsManager instance;
    return instance;
}

void SettingsManager::savePortSettings(const QString& portName, int baudRate)
{
    m_settings.setValue("Serial/PortName", portName);
    m_settings.setValue("Serial/BaudRate", baudRate);
    m_settings.sync();
}

void SettingsManager::loadPortSettings(QString& portName, int& baudRate)
{
    portName = m_settings.value("Serial/PortName", "").toString();
    baudRate = m_settings.value("Serial/BaudRate", 9600).toInt();
}

void SettingsManager::saveWindowGeometry(const QByteArray& geometry)
{
    m_settings.setValue("Window/Geometry", geometry);
    m_settings.sync();
}

QByteArray SettingsManager::loadWindowGeometry()
{
    return m_settings.value("Window/Geometry").toByteArray();
}

void SettingsManager::saveLastDirectory(const QString& path)
{
    m_settings.setValue("LastDirectory", path);
    m_settings.sync();
}

QString SettingsManager::loadLastDirectory()
{
    return m_settings.value("LastDirectory", QCoreApplication::applicationDirPath()).toString();
}
