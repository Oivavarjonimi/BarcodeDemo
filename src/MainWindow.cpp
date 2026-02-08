#include "MainWindow.h"

#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QGuiApplication>
#include <QDateTime>
#include <QTextStream>
#include <QFile>

#include <QMenuBar>
#include <QActionGroup>
#include <QSettings>
#include <QDebug>
#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QSettings>

#include <QProcess>
#include <QStandardPaths>
#include <QFileInfo>

#include "ZXingDecoder.h"
#include "ZBarDecoder.h"
#include "CameraController.h"

#include <QCryptographicHash>
#include <QBuffer>
#include <QDebug>

static QString imageFingerprint(const QImage& img);


MainWindow::~MainWindow() = default;

static QString timeStamp()
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    m_camera = new CameraController(this);

    m_net = new QNetworkAccessManager(this);

    loadPhoneSettings();

    buildUi();
    buildMenus();

    m_backend = loadBackendSetting();
    setDecoderBackend(m_backend, /*persist=*/false);

    if (statusBar() && m_decoder) {
        statusBar()->showMessage(QString("Decoder backend: %1").arg(m_decoder->name()), 3000);
    }

    statusBar()->showMessage("Starting camera...");
    connect(m_camera, &CameraController::frameReady, this, &MainWindow::onFrameReady);
    connect(m_camera, &CameraController::errorMessage, this, [this](const QString& msg) {
        statusBar()->showMessage(msg, 5000);
        QMessageBox::warning(this, "Camera error", msg);
    });

    if (m_camera->start()) {
        statusBar()->showMessage("Camera running.");
    }
}

void MainWindow::buildUi()
{
    auto* central = new QWidget(this);
    setCentralWidget(central);

    m_preview = new QLabel(this);
    m_preview->setMinimumSize(640, 360);
    m_preview->setAlignment(Qt::AlignCenter);
    m_preview->setText("No image");
    m_preview->setStyleSheet("QLabel { background: #111; color: #ddd; border: 1px solid #333; }");

    m_results = new QListWidget(this);
    m_results->setSelectionMode(QAbstractItemView::SingleSelection);
    m_btnPhoneShot = new QPushButton("Snap (Live)", this);
    m_btnCapture = new QPushButton("PC kuva", this);
    m_btnDecode = new QPushButton("Decode", this);
    m_btnCopy = new QPushButton("Kopioi", this);
    m_btnSave = new QPushButton("Tallenna", this);

    connect(m_btnPhoneShot, &QPushButton::clicked, this, &MainWindow::onPhoneShot);
    connect(m_btnCapture, &QPushButton::clicked, this, &MainWindow::onCapture);
    connect(m_btnDecode, &QPushButton::clicked, this, &MainWindow::onDecode);
    connect(m_btnCopy, &QPushButton::clicked, this, &MainWindow::onCopy);
    connect(m_btnSave, &QPushButton::clicked, this, &MainWindow::onSaveImage);

    auto* rightPanel = new QWidget(this);
    auto* rightLayout = new QVBoxLayout(rightPanel);

    rightLayout->addWidget(new QLabel("Tunnistetut datat:", this));
    rightLayout->addWidget(m_results, 1);

    auto* btnRow = new QHBoxLayout();

    btnRow->addWidget(m_btnPhoneShot);
    btnRow->addWidget(m_btnCapture);
    btnRow->addWidget(m_btnDecode);
    btnRow->addWidget(m_btnCopy);
    btnRow->addWidget(m_btnSave);
    btnRow->addStretch(1);

    rightLayout->addLayout(btnRow);

    auto* splitter = new QSplitter(this);
    splitter->addWidget(m_preview);
    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);

    auto* root = new QVBoxLayout(central);
    root->addWidget(splitter);
}

void MainWindow::buildMenus()
{
    setWindowTitle("Barcode Demo (Qt6 + ZXing-cpp)");

    auto* fileMenu = menuBar()->addMenu("&File");

    // Settings before Help (so order is File - Settings - Help)
    auto* settingsMenu = menuBar()->addMenu("&Settings");
    auto* backendMenu = settingsMenu->addMenu("Decoder backend");

    auto* actPhone = settingsMenu->addAction("Phone camera...");
    connect(actPhone, &QAction::triggered, this, &MainWindow::editPhoneSettings);

    settingsMenu->addSeparator();


    auto* grp = new QActionGroup(this);
    grp->setExclusive(true);

    m_actZXing = backendMenu->addAction("ZXing");
    m_actZXing->setCheckable(true);
    grp->addAction(m_actZXing);

    m_actZBar = backendMenu->addAction("ZBar");
    m_actZBar->setCheckable(true);
    grp->addAction(m_actZBar);

    // Use current member value (will be synced again in setDecoderBackend)
    m_actZXing->setChecked(m_backend == DecoderBackend::ZXing);
    m_actZBar->setChecked(m_backend == DecoderBackend::ZBar);

    connect(m_actZXing, &QAction::triggered, this, [this]() {
        setDecoderBackend(DecoderBackend::ZXing, true);
    });
    connect(m_actZBar, &QAction::triggered, this, [this]() {
        setDecoderBackend(DecoderBackend::ZBar, true);
    });

    auto* helpMenu = menuBar()->addMenu("&Help");

    QAction* actNew = fileMenu->addAction("&New");
    QAction* actOpen = fileMenu->addAction("&Open image...");
    QAction* actSave = fileMenu->addAction("&Save image...");
    QAction* actSaveData = fileMenu->addAction("Save decoded &data...");
    fileMenu->addSeparator();
    QAction* actExit = fileMenu->addAction("E&xit");

    QAction* actAbout = helpMenu->addAction("&About");

    connect(actNew, &QAction::triggered, this, &MainWindow::onNew);
    connect(actOpen, &QAction::triggered, this, &MainWindow::onOpenImage);
    connect(actSave, &QAction::triggered, this, &MainWindow::onSaveImage);
    connect(actSaveData, &QAction::triggered, this, &MainWindow::onSaveDecodedData);
    connect(actExit, &QAction::triggered, this, &QWidget::close);

    connect(actAbout, &QAction::triggered, this, &MainWindow::onAbout);

    statusBar()->showMessage("Ready.");
}
/*
void MainWindow::onFrameReady(const QImage& img)
{
    m_currentImage = img;
    setPreviewImage(m_currentImage);
}
*/

/*
void MainWindow::onFrameReady(const QImage& img)
{
    qDebug() << "frameReady:" << img.size() << "null?" << img.isNull() << "format" << img.format();
    m_currentImage = img;
    setPreviewImage(m_currentImage);
}
*/

void MainWindow::onFrameReady(const QImage& img)
{
    static int n = 0;
    if ((++n % 30) == 0) {
        qDebug() << "frameReady count=" << n << "size=" << img.size() << "null=" << img.isNull();
    }
    m_currentImage = img;
    setPreviewImage(m_currentImage);
}

void MainWindow::setPreviewImage(const QImage& img)
{
    if (img.isNull()) {
        m_preview->setText("No image");
        m_preview->setPixmap(QPixmap());
        return;
    }

    QPixmap px = QPixmap::fromImage(img);
    m_preview->setPixmap(px.scaled(m_preview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

/*void MainWindow::onCapture()
{
    const QImage frame = m_camera->lastFrame();onFrameReady
    if (frame.isNull()) {
        statusBar()->showMessage("No frame available.", 3000);
        return;
    }

    m_snapshotImage = frame;
    decodeAndAppend(m_snapshotImage, "snapshot");
}
*/

void MainWindow::onCapture()
{
    if (m_currentImage.isNull()) {
        statusBar()->showMessage("No frame available.", 3000);
        return;
    }

    m_snapshotImage = m_currentImage;
    decodeAndAppend(m_snapshotImage, "snapshot");
}


void MainWindow::onDecode()
{
    const bool useSnapshot = !m_snapshotImage.isNull();
    const QImage imgToDecode = useSnapshot ? m_snapshotImage : m_currentImage;

    qDebug() << "Decode called:"
        << "useSnapshot=" << useSnapshot
        << "liveFromPhone=" << m_liveFromPhone
        << "current=" << m_currentImage.size() << imageFingerprint(m_currentImage)
        << "snapshot=" << m_snapshotImage.size() << imageFingerprint(m_snapshotImage)
        << "chosen=" << imgToDecode.size() << imageFingerprint(imgToDecode);

    if (imgToDecode.isNull()) {
        statusBar()->showMessage("Decode: no image to decode.", 4000);
        return;
    }

    m_results->clear();

    decodeAndAppend(imgToDecode, useSnapshot ? "Phone snapshot" : "Current image");

    if (m_results->count() == 0) {
        statusBar()->showMessage(
            QString("No barcode found (manual decode) using %1").arg(m_decoder ? m_decoder->name() : "Decoder"),
            4000
        );
    }
    else {
        statusBar()->showMessage(
            QString("Decode OK (%1 result(s)) using %2").arg(m_results->count()).arg(m_decoder ? m_decoder->name() : "Decoder"),
            4000
        );
    }
}



void MainWindow::decodeAndAppend(const QImage& img, const QString& sourceLabel)
{
    if (img.isNull() || !m_decoder)
        return;

    const QString fp = imageFingerprint(img);

    const auto res = m_decoder->decode(img);
    if (!res.ok) {
        statusBar()->showMessage("No barcode found (" + sourceLabel + ", fp=" + fp + ").", 3000);
        return;
    }

    // Debounce only if BOTH decoded text and image fingerprint match within 1.5s
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (res.text == m_lastDecodedText && fp == m_lastDecodedImageFp && (now - m_lastDecodedMs) < 1500) {
        statusBar()->showMessage("Same code ignored (debounce).", 1500);
        return;
    }

    m_lastDecodedText = res.text;
    m_lastDecodedFormat = res.format;
    m_lastDecodedImageFp = fp;
    m_lastDecodedMs = now;

    const QString line = QString("[%1] (%2) %3  {src=%4 fp=%5}")
        .arg(timeStamp())
        .arg(res.format)
        .arg(res.text)
        .arg(sourceLabel)
        .arg(fp);

    m_results->addItem(line);
    m_results->setCurrentRow(m_results->count() - 1);

    statusBar()->showMessage(QString("Decoded: %1 (%2)").arg(res.format, sourceLabel), 3000);
}

QString MainWindow::selectedOrLastDecodedText() const
{
    auto* item = m_results->currentItem();
    if (!item) {
        if (m_results->count() == 0)
            return {};
        item = m_results->item(m_results->count() - 1);
    }

    const QString s = item->text();
    const int idx = s.indexOf(") ");
    if (idx >= 0 && idx + 2 < s.size())
        return s.mid(idx + 2);

    return s;
}

void MainWindow::onCopy()
{
    const QString text = selectedOrLastDecodedText();
    if (text.isEmpty()) {
        statusBar()->showMessage("Nothing to copy.", 3000);
        return;
    }

    QGuiApplication::clipboard()->setText(text);
    statusBar()->showMessage("Copied to clipboard.", 2000);
}

void MainWindow::onSaveImage()
{
    if (m_snapshotImage.isNull()) {
        statusBar()->showMessage("No snapshot to save. Use 'Ota kuva' first.", 4000);
        return;
    }

    const QString path = QFileDialog::getSaveFileName(
        this,
        "Save image",
        "snapshot.png",
        "Images (*.png *.jpg *.jpeg *.bmp)"
    );

    if (path.isEmpty())
        return;

    if (m_snapshotImage.save(path)) {
        statusBar()->showMessage("Saved image: " + path, 4000);
    }
    else {
        statusBar()->showMessage("Failed to save image.", 4000);
    }
}

void MainWindow::onNew()
{
    m_results->clear();
    m_snapshotImage = QImage();
    m_lastDecodedText.clear();
    m_lastDecodedFormat.clear();
    m_lastDecodedMs = 0;
    statusBar()->showMessage("New session.", 2000);
}

void MainWindow::onOpenImage()
{
    const QString path = QFileDialog::getOpenFileName(
        this,
        "Open image",
        QString(),
        "Images (*.png *.jpg *.jpeg *.bmp)"
    );

    if (path.isEmpty())
        return;

    QImage img(path);
    if (img.isNull()) {
        statusBar()->showMessage("Failed to open image.", 4000);
        return;
    }

    // Store as both current and snapshot, so decode + save works
    m_currentImage = img;
    m_snapshotImage = img;
    setPreviewImage(img);

    decodeAndAppend(img, "opened image");
    statusBar()->showMessage("Opened: " + path, 3000);
}

void MainWindow::onSaveDecodedData()
{
    if (m_results->count() == 0) {
        statusBar()->showMessage("No decoded data to save.", 3000);
        return;
    }

    const QString path = QFileDialog::getSaveFileName(
        this,
        "Save decoded data",
        "decoded.txt",
        "Text (*.txt);;CSV (*.csv)"
    );

    if (path.isEmpty())
        return;

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        statusBar()->showMessage("Failed to write file.", 4000);
        return;
    }

    QTextStream out(&f);
    for (int i = 0; i < m_results->count(); ++i) {
        out << m_results->item(i)->text() << "\n";
    }

    statusBar()->showMessage("Saved decoded data: " + path, 4000);
}

void MainWindow::onAbout()
{
    QMessageBox::about(this,
        "About Barcode Demo",
        "Barcode Demo\n\n"
        "Qt6 Widgets + Qt Multimedia + ZXing-cpp\n"
        "Features:\n"
        "- Live camera preview\n"
        "- Snapshot decode (QR, EAN, Code128)\n"
        "- Copy to clipboard\n"
        "- Save snapshot image\n"
        "- Open image and decode\n"
    );
}

DecoderBackend MainWindow::loadBackendSetting()
{
    QSettings s("BarcodeDemo", "BarcodeDemo");
    const QString v = s.value("decoder/backend", "zxing").toString().toLower();
    return (v == "zbar") ? DecoderBackend::ZBar : DecoderBackend::ZXing;
}

void MainWindow::saveBackendSetting(DecoderBackend b)
{
    QSettings s("BarcodeDemo", "BarcodeDemo");
    s.setValue("decoder/backend", (b == DecoderBackend::ZBar) ? "zbar" : "zxing");
}

void MainWindow::setDecoderBackend(DecoderBackend b, bool persist)
{
    m_backend = b;

    switch (b) {
    case DecoderBackend::ZXing:
        m_decoder = std::make_unique<ZXingDecoder>();
        break;
    case DecoderBackend::ZBar:
        m_decoder = std::make_unique<ZBarDecoder>();
        break;
    }

    if (persist)
        saveBackendSetting(b);

    if (statusBar() && m_decoder)
        statusBar()->showMessage(QString("Decoder backend: %1").arg(m_decoder->name()), 2500);

    if (m_actZXing) m_actZXing->setChecked(b == DecoderBackend::ZXing);
    if (m_actZBar)  m_actZBar->setChecked(b == DecoderBackend::ZBar);
}


void MainWindow::onPhoneShot()
{
    // First press: start live stream
    if (!m_liveFromPhone) {
        m_liveFromPhone = true;              // <-- TÄRKEÄ: tila päälle heti
        m_results->clear();
        m_snapshotImage = QImage();          // clear previous snapshot
        m_btnPhoneShot->setText("Freeze");
        startPhoneLive();
        statusBar()->showMessage("Phone live preview started. Press Freeze to capture.", 4000);
        return;
    }

    // Second press: freeze current frame and stop stream
    if (m_currentImage.isNull()) {
        statusBar()->showMessage("Freeze: no phone frame received yet.", 4000);
        return;
    }

    m_snapshotImage = m_currentImage.copy();
    m_btnPhoneShot->setText("Snap");
    stopPhoneLive(/*keepLastFrame=*/true);
    m_liveFromPhone = false;                 // <-- TÄRKEÄ: tila pois

    // Show the frozen frame in UI (optional but recommended)
    setPreviewImage(m_snapshotImage);

    statusBar()->showMessage("Phone frame captured. Press Decode to decode.", 4000);
}



void MainWindow::startPhoneLive()
{
    if (m_liveReply) {
        m_liveReply->abort();
        m_liveReply->deleteLater();
        m_liveReply = nullptr;
    }

    m_mjpegBuffer.clear();

    QNetworkRequest req{ QUrl(m_droidcamUrl) };

    // DroidCam MJPEG: keep connection open
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
        QNetworkRequest::NoLessSafeRedirectPolicy);

    m_liveReply = m_net->get(req);

    connect(m_liveReply, &QNetworkReply::readyRead, this, &MainWindow::onPhoneStreamReadyRead);
    connect(m_liveReply, &QNetworkReply::finished, this, &MainWindow::onPhoneStreamFinished);

    m_liveFromPhone = true;
}

void MainWindow::stopPhoneLive(bool keepLastFrame)
{
    (void)keepLastFrame; // last frame is already in m_currentImage as we update it live

    m_liveFromPhone = false;

    if (m_liveReply) {
        m_liveReply->abort();       // katkaise yhteys
        m_liveReply->deleteLater();
        m_liveReply = nullptr;
    }

    m_mjpegBuffer.clear();
}

void MainWindow::onPhoneStreamFinished()
{
    // Tämä tulee kun abort() tai yhteys katkeaa
    if (m_liveReply && m_liveReply->error() != QNetworkReply::OperationCanceledError) {
        statusBar()->showMessage("Phone stream ended unexpectedly", 4000);
    }
}

void MainWindow::onPhoneStreamReadyRead()
{
    if (!m_liveReply)
        return;

    m_mjpegBuffer += m_liveReply->readAll();

    // Etsi JPEG frame: SOI 0xFFD8 ... EOI 0xFFD9
    while (true) {
        const int soi = m_mjpegBuffer.indexOf("\xFF\xD8", 0);
        if (soi < 0) {
            // Pidä bufferin koko järkevänä
            if (m_mjpegBuffer.size() > 2 * 1024 * 1024)
                m_mjpegBuffer.clear();
            return;
        }

        const int eoi = m_mjpegBuffer.indexOf("\xFF\xD9", soi + 2);
        if (eoi < 0)
            return; // odota lisää dataa

        const int jpegLen = (eoi - soi) + 2;
        const QByteArray jpegData = m_mjpegBuffer.mid(soi, jpegLen);

        // Poista käsitelty data bufferista
        m_mjpegBuffer.remove(0, eoi + 2);

        QImage img;
        if (img.loadFromData(jpegData, "JPG") || img.loadFromData(jpegData, "JPEG")) {

            // --- 3) EHDOTUS: Päivitä DECODEN lähdekuva aina uusimmalla framella ---
            m_currentImage = img;

            // Käytä keskitettyä preview-funktiota (skaalaus ym. yhdessä paikassa)
            setPreviewImage(m_currentImage);

            // (valinnainen) kevyt debug: näkee että framet vaihtuvat
            qDebug() << "Phone frame:" << m_currentImage.size() << "format" << m_currentImage.format();
        }

        // jatka loopissa, jos bufferissa oli useampi frame
    }
}


void MainWindow::loadPhoneSettings()
{
    QSettings s;
    const QString ip = s.value("phone/ip", "192.168.100.171").toString();
    const int port = s.value("phone/port", 4747).toInt();

    m_droidcamUrl = QString("http://%1:%2/video").arg(ip).arg(port);
}

void MainWindow::editPhoneSettings()
{
    QSettings s;

    QDialog dlg(this);
    dlg.setWindowTitle("Phone camera settings");

    auto* ipEdit = new QLineEdit(s.value("phone/ip", "192.168.100.171").toString(), &dlg);

    auto* portSpin = new QSpinBox(&dlg);
    portSpin->setRange(1, 65535);
    portSpin->setValue(s.value("phone/port", 4747).toInt());

    auto* form = new QFormLayout(&dlg);
    form->addRow("IP address:", ipEdit);
    form->addRow("Port:", portSpin);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    form->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() != QDialog::Accepted)
        return;

    const QString ip = ipEdit->text().trimmed();
    const int port = portSpin->value();

    s.setValue("phone/ip", ip);
    s.setValue("phone/port", port);

    m_droidcamUrl = QString("http://%1:%2/video").arg(ip).arg(port);

    // Jos live on päällä, reconnect
    if (m_liveFromPhone) {
        stopPhoneLive(false);
        startPhoneLive();
    }

    statusBar()->showMessage(QString("Phone camera set to %1:%2").arg(ip).arg(port), 4000);
}

static QString imageFingerprint(const QImage& img)
{
    if (img.isNull())
        return "null";

    QByteArray bytes;
    QBuffer buf(&bytes);
    buf.open(QIODevice::WriteOnly);
    img.save(&buf, "PNG");

    const QByteArray hash = QCryptographicHash::hash(bytes, QCryptographicHash::Sha1);
    return QString(hash.toHex().left(12));
}
