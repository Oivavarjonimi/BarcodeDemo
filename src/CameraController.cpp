// CameraController.cpp
#include "CameraController.h"

#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include <QVideoFrame>

CameraController::CameraController(QObject* parent)
    : QObject(parent)
{
    m_camera = new QCamera(this);
    m_session = new QMediaCaptureSession(this);
    m_sink = new QVideoSink(this);

    m_session->setCamera(m_camera);
    m_session->setVideoSink(m_sink);                 // <-- THIS prevents fakevideosink

    connect(m_sink, &QVideoSink::videoFrameChanged,
        this, &CameraController::onVideoFrameChanged);
}

bool CameraController::start()
{
    m_camera->start();
    return true;
}

void CameraController::onVideoFrameChanged(const QVideoFrame& frame)
{
    if (!frame.isValid())
        return;

    QVideoFrame f(frame);
    if (!f.map(QVideoFrame::ReadOnly))
        return;

    const QImage img = f.toImage();                  // Qt 6 helper
    f.unmap();

    if (!img.isNull()) {
        m_lastFrame = img;
        emit frameReady(m_lastFrame);
    }
}
