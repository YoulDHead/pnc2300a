#include "CommandFactory.h"

char CommandFactory::m_terminator = ';';

QByteArray CommandFactory::formatCommand(const QString& cmd)
{
    return (cmd + m_terminator).toLatin1();
}

QByteArray CommandFactory::plotAbsolute(int x, int y)
{
    return formatCommand(QString("PA%1,%2").arg(x).arg(y));
}

QByteArray CommandFactory::plotRelative(int dx, int dy)
{
    return formatCommand(QString("PR%1,%2").arg(dx).arg(dy));
}

QByteArray CommandFactory::penUp()
{
    return formatCommand("PU");
}

QByteArray CommandFactory::penDown()
{
    return formatCommand("PD");
}

QByteArray CommandFactory::home()
{
    return QByteArray("H\r\n");
}

QByteArray CommandFactory::initialize()
{
    return formatCommand("IN");
}

QByteArray CommandFactory::setVelocityXY(int speed)
{
    return formatCommand(QString("VS%1").arg(speed));
}

QByteArray CommandFactory::setVelocityZ(int speed)
{
    return formatCommand(QString("!VZ%1").arg(speed));
}

QByteArray CommandFactory::motorControl(bool on)
{
    return formatCommand(QString("!MC%1").arg(on ? 1 : 0));
}

QByteArray CommandFactory::setZ1Z2(int z1, int z2)
{
    return formatCommand(QString("!PZ%1,%2").arg(z1).arg(z2));
}

QByteArray CommandFactory::setZ0(int z0)
{
    return formatCommand(QString("!ZO%1").arg(z0));
}

QByteArray CommandFactory::setWindow(int llx, int lly, int urx, int ury)
{
    return formatCommand(QString("IW%1,%2,%3,%4").arg(llx).arg(lly).arg(urx).arg(ury));
}

QByteArray CommandFactory::setScaling(double xmin, double xmax, double ymin, double ymax)
{
    return formatCommand(QString("SC%1,%2,%3,%4").arg(xmin).arg(xmax).arg(ymin).arg(ymax));
}

QByteArray CommandFactory::setLineType(int pattern)
{
    return formatCommand(QString("LT%1").arg(pattern));
}

QByteArray CommandFactory::setFillType(int type, int spacing, int angle)
{
    return formatCommand(QString("FT%1,%2,%3").arg(type).arg(spacing).arg(angle));
}

QByteArray CommandFactory::edgeRectangleAbs(int x, int y)
{
    return formatCommand(QString("EA%1,%2").arg(x).arg(y));
}

QByteArray CommandFactory::shadeRectangleAbs(int x, int y)
{
    return formatCommand(QString("RA%1,%2").arg(x).arg(y));
}

QByteArray CommandFactory::circle(int radius)
{
    return formatCommand(QString("CI%1").arg(radius));
}

QByteArray CommandFactory::arcAbsolute(int x, int y, int angle)
{
    return formatCommand(QString("AA%1,%2,%3").arg(x).arg(y).arg(angle));
}

QByteArray CommandFactory::label(const QString& text)
{
    QByteArray cmd = "LB";
    cmd += text.toLatin1();
    cmd += char(0x03);
    return cmd;
}

QByteArray CommandFactory::setCharacterSize(double w, double h)
{
    return formatCommand(QString("SI%1,%2").arg(w).arg(h));
}

QByteArray CommandFactory::outputStatus() { return formatCommand("OS"); }
QByteArray CommandFactory::outputError() { return formatCommand("OE"); }
QByteArray CommandFactory::outputCurrentPosition() { return formatCommand("OC"); }
QByteArray CommandFactory::outputActualPosition() { return formatCommand("OA"); }
QByteArray CommandFactory::outputP1P2() { return formatCommand("OP"); }
QByteArray CommandFactory::outputWindow() { return formatCommand("OW"); }
QByteArray CommandFactory::outputHardClipLimits() { return formatCommand("OH"); }
QByteArray CommandFactory::outputFactor() { return formatCommand("OF"); }
QByteArray CommandFactory::outputIdentification() { return formatCommand("OI"); }

QByteArray CommandFactory::outputRemainingBuffer() { return "\x1B.B"; }
QByteArray CommandFactory::abortDeviceControl() { return "\x1B.J"; }
QByteArray CommandFactory::abortCAMMGL() { return "\x1B.K"; }
QByteArray CommandFactory::initializeDeviceControl() { return "\x1B.R"; }
