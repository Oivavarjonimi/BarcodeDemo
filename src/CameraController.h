// CameraController.h
#include <QObject>
#include <QImage>

class QCamera;
class QMediaCaptureSession;
class QVideoSink;
class QVideoFrame;

class CameraController : public QObject {
    Q_OBJECT
public:
    explicit CameraController(QObject* parent = nullptr);
    bool start();
    QImage lastFrame() const { return m_lastFrame; }

signals:
    void frameReady(const QImage& img);
    void errorMessage(const QString& msg);

private:
    void onVideoFrameChanged(const QVideoFrame& frame);

    QCamera* m_camera = nullptr;
    QMediaCaptureSession* m_session = nullptr;
    QVideoSink* m_sink = nullptr;

    QImage m_lastFrame;
};
