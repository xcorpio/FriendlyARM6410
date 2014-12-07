#ifndef REMOTECAMERA_H
#define REMOTECAMERA_H

#include <libwebsockets.h>
#include <QThread>

class RemoteCamera : public QThread
{
    Q_OBJECT
public:
    explicit RemoteCamera(QObject *parent = 0);
    static  void drawFb(void* in);
signals:

public slots:
    void openCamera();
    void closeCamera();
    void stopPreview();
    void restorePreview();

    // QThread interface
protected:
    void run();

public:
    struct libwebsocket_context* context;
    struct libwebsocket* wsi;
};

#endif // REMOTECAMERA_H
