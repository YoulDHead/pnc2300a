#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QSettings>
#include <QString>
#include <QByteArray>

class SettingsManager
{
public:
    static SettingsManager& instance();

    void savePortSettings(const QString& portName, int baudRate);
    void loadPortSettings(QString& portName, int& baudRate);

    void saveWindowGeometry(const QByteArray& geometry);
    QByteArray loadWindowGeometry();

    void saveLastDirectory(const QString& path);
    QString loadLastDirectory();

private:
    SettingsManager();
    QSettings m_settings;
};

#endif // SETTINGSMANAGER_H
