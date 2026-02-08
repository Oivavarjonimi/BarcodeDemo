#pragma once

#include <QMainWindow>
#include <QImage>
#include <QAction>
#include <QPushButton>
#include <QString>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <memory>

#include "IDecoder.h"

enum class DecoderBackend { ZXing, ZBar };

class QLabel;
class QListWidget;
class CameraController;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onFrameReady(const QImage& img);

    // Phone Snap button: 1st press = live, 2nd press = freeze & disconnect
    void onPhoneShot();

    void onCapture();
    void onDecode();
    void onCopy();
    void onSaveImage();

    void onNew();
    void onOpenImage();
    void onSaveDecodedData();
    void onAbout();

private:
    // UI
    void buildUi();
    void buildMenus();
    void setPreviewImage(const QImage& img);

    // Decoding
    void decodeAndAppend(const QImage& img, const QString& sourceLabel);
    QString selectedOrLastDecodedText() const;

    // Settings -> Decoder backend
    void createSettingsMenu();
    void setDecoderBackend(DecoderBackend b, bool persist);
    static DecoderBackend loadBackendSetting();
    static void saveBackendSetting(DecoderBackend b);

    // Phone live preview (MJPEG over HTTP)
    void startPhoneLive();
    void stopPhoneLive(bool keepLastFrame);
    void onPhoneStreamReadyRead();
    void onPhoneStreamFinished();

    void loadPhoneSettings();
    void editPhoneSettings();

private:
    // --- decoder backend state ---
    std::unique_ptr<IDecoder> m_decoder;
    DecoderBackend m_backend = DecoderBackend::ZXing;
    QAction* m_actZXing = nullptr;
    QAction* m_actZBar = nullptr;

    // --- UI widgets ---
    QLabel* m_preview = nullptr;
    QListWidget* m_results = nullptr;

    QPushButton* m_btnPhoneShot = nullptr;
    QPushButton* m_btnCapture = nullptr;
    QPushButton* m_btnDecode = nullptr;
    QPushButton* m_btnCopy = nullptr;
    QPushButton* m_btnSave = nullptr;

    // --- camera controller (PC webcam) ---
    CameraController* m_camera = nullptr;

    // --- images ---
    QImage m_currentImage;   // currently shown image (live frame or loaded image)
    QImage m_snapshotImage;  // captured snapshot for saving (optional)

    // --- phone live state ---
    bool m_liveFromPhone = false;
    QNetworkAccessManager* m_net = nullptr;
    QNetworkReply* m_liveReply = nullptr;
    QByteArray m_mjpegBuffer;
    QString m_droidcamUrl; // loaded from settings

    // --- last decode bookkeeping ---
    QString m_lastDecodedText;
    QString m_lastDecodedFormat;
    qint64 m_lastDecodedMs = 0;
};
