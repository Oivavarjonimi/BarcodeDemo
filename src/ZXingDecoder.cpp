#include "ZXingDecoder.h"
#include <QImage>

// ZXing includes (sinun projektissa toimivilla poluilla)
#include <ReadBarcode.h>
#include <ReaderOptions.h>
#include <ImageView.h>
#include <BarcodeFormat.h>

DecodeResult ZXingDecoder::decode(const QImage& img) const
{
    DecodeResult out;
    if (img.isNull())
        return out;

    QImage gray = img.convertToFormat(QImage::Format_Grayscale8);

    ZXing::ReaderOptions opts;
    opts.setTryHarder(true);
    opts.setTryRotate(true);
    opts.setTryInvert(true);

    opts.setFormats(
        ZXing::BarcodeFormat::QRCode |
        ZXing::BarcodeFormat::EAN13 |
        ZXing::BarcodeFormat::EAN8 |
        ZXing::BarcodeFormat::UPCA |
        ZXing::BarcodeFormat::UPCE |
        ZXing::BarcodeFormat::Code128 |
        ZXing::BarcodeFormat::Code39 |
        ZXing::BarcodeFormat::ITF
    );

    ZXing::ImageView view(
        gray.constBits(),
        gray.width(),
        gray.height(),
        ZXing::ImageFormat::Lum,
        gray.bytesPerLine()
    );

    auto results = ZXing::ReadBarcodes(view, opts);
    if (results.empty())
        return out;

    out.ok = true;

    if (results.size() == 1) {
        out.format = QString::fromStdString(ZXing::ToString(results[0].format()));
        out.text = QString::fromStdString(results[0].text());
        return out;
    }

    out.format = "MULTI";
    QStringList lines;
    for (auto& r : results) {
        lines << QString("%1: %2")
            .arg(QString::fromStdString(ZXing::ToString(r.format())))
            .arg(QString::fromStdString(r.text()));
    }
    out.text = lines.join("\n");
    return out;
}
