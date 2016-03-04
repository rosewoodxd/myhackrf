#ifndef RECEIVER_H
#define RECEIVER_H

#include <QObject>

#include "packet.pb.h"
#include "zhelpers.h"

class Receiver : public QObject
{
    Q_OBJECT

public:
    Receiver();
    bool initialize();
    void setIP(QString ipAddress);
    void tune(u_int64_t fc_hz);
    void setSampleRate(double fs_hz);

public slots:
    void receivePackets();
    void stop();

signals:
    void newPacket(Packet pkt);
    void newParameters(double fc_hz, double fs_hz, QString data_target);

private:
    void updateParameters();

    bool isRunning;
    bool isInitialized;
    QString data_port;
    QString comm_port;
    QString data_target;
    QString comm_target;

    zmq::context_t * comm_context;
    zmq::socket_t * comm_socket;

    u_int64_t fc_hz;    // center freq
    double    fs_hz;    // sample rate
    u_int32_t lna_gain; // gain
};

#endif // RECEIVER_H
