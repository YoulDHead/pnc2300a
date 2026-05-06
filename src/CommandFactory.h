#ifndef COMMANDFACTORY_H
#define COMMANDFACTORY_H

#include <QByteArray>
#include <QString>

class CommandFactory
{
public:
    static QByteArray plotAbsolute(int x, int y);
    static QByteArray plotRelative(int dx, int dy);
    static QByteArray penUp();
    static QByteArray penDown();
    static QByteArray home();
    static QByteArray initialize();

    static QByteArray setVelocityXY(int speed);
    static QByteArray setVelocityZ(int speed);

    static QByteArray motorControl(bool on);

    static QByteArray setZ1Z2(int z1, int z2);
    static QByteArray setZ0(int z0);

    static QByteArray setWindow(int llx, int lly, int urx, int ury);
    static QByteArray setScaling(double xmin, double xmax, double ymin, double ymax);

    static QByteArray setLineType(int pattern);
    static QByteArray setFillType(int type, int spacing = 0, int angle = 0);
    static QByteArray edgeRectangleAbs(int x, int y);
    static QByteArray shadeRectangleAbs(int x, int y);
    static QByteArray circle(int radius);
    static QByteArray arcAbsolute(int x, int y, int angle);

    static QByteArray label(const QString& text);
    static QByteArray setCharacterSize(double w, double h);

    static QByteArray outputStatus();
    static QByteArray outputError();
    static QByteArray outputCurrentPosition();
    static QByteArray outputActualPosition();
    static QByteArray outputP1P2();
    static QByteArray outputWindow();
    static QByteArray outputHardClipLimits();
    static QByteArray outputFactor();
    static QByteArray outputIdentification();

    static QByteArray outputRemainingBuffer();
    static QByteArray abortDeviceControl();
    static QByteArray abortCAMMGL();
    static QByteArray initializeDeviceControl();

    static void setTerminator(char term) { m_terminator = term; }

private:
    static char m_terminator;
    static QByteArray formatCommand(const QString& cmd);
};

#endif // COMMANDFACTORY_H
