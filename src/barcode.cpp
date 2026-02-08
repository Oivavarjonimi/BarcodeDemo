#include <QVideoSink>
#include <QVideoFrame>
#include <QImage>
#include <QDateTime>

// ZXing-cpp
#include <ZXing/ReadBarcode.h>
#include <ZXing/DecodeHints.h>
#include <ZXing/ImageView.h>
#include <ZXing/BarcodeFormat.h>

static QString lastText;
static qint64 lastMs = 0;

void handleFrame(const QVideoFrame& frame)
{
    QVideoFrame f(frame);
    if (!f.isValid()) return;
    if (!f.map(QVideoFrame::ReadOnly)) return;

    QImage img = f.toImage();   // Qt6: QVideoFrame::toImage()
    f.unmap();
    if (img.isNull()) return;

    // Pienennä hieman suorituskyvyn vuoksi (demoissa usein hyvä)
    // img = img.scaledToWidth(960, Qt::SmoothTransformation);

    ZXing::DecodeHints hints;
    hints.setTryHarder(true);
    hints.setFormats(ZXing::BarcodeFormat::Any); // tai rajaa: QRCode | EAN13 | Code128...

    // ZXing haluaa raw view’n:
    // Oletetaan img on RGB32/ARGB32 -> käytetään RGB
    QImage rgb = img.convertToFormat(QImage::Format_RGB888);
    ZXing::ImageView view(rgb.bits(), rgb.width(), rgb.height(), ZXing::ImageFormat::RGB);

    auto result = ZXing::ReadBarcode(view, hints);
    if (!result.isValid()) return;

    const QString text = QString::fromStdString(result.text());

    // Debounce: älä spämmi samaa koodia jatkuvasti
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (text == lastText && (now - lastMs) < 1500) return;
    lastText = text;
    lastMs = now;

    // -> tänne: näytä UI:ssa, kopioi clipboardiin, loki...
    // esimerkiksi: label->setText(text);
}
