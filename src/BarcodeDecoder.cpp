#include "BarcodeDecoder.h"

// ZXing-cpp
#include <ReadBarcode.h>      // jos sinulla nämä ovat "flat"-muodossa
#include <ReaderOptions.h>
#include <ImageView.h>
#include <BarcodeFormat.h>
// Debug
#include <QDebug>
#include <QDir>
#include <QStringList>


static QString formatToString(ZXing::BarcodeFormat f)
{
    using ZXing::BarcodeFormat;
    switch (f) {
    case BarcodeFormat::QRCode:  return "QR";
    case BarcodeFormat::EAN13:   return "EAN-13";
    case BarcodeFormat::EAN8:    return "EAN-8";
    case BarcodeFormat::UPCA:    return "UPC-A";
    case BarcodeFormat::UPCE:    return "UPC-E";
    case BarcodeFormat::Code128: return "Code-128";
    case BarcodeFormat::Code39:  return "Code-39";
    case BarcodeFormat::ITF:     return "ITF";
    default: return "Unknown";
    }
}

DecodeResult BarcodeDecoder::decode(const QImage& img) const
{
    DecodeResult out;

    if (img.isNull()) {
        qDebug() << "Decode: image is null";
        return out;
    }

    // --- Input debug ---
    qDebug() << "Decode input:"
        << "size=" << img.size()
        << "format=" << img.format()
        << "bytesPerLine=" << img.bytesPerLine();

    // --- Convert to grayscale (robust) ---
    QImage gray = img.convertToFormat(QImage::Format_Grayscale8);

    qDebug() << "Grayscale image:"
        << "size=" << gray.size()
        << "format=" << gray.format()
        << "bytesPerLine=" << gray.bytesPerLine();

    // --- Save debug image ---
    const QString outDir = QDir::currentPath() + "/debug";
    QDir().mkpath(outDir);

    const QString grayPath = outDir + "/input_gray.png";
    if (!gray.save(grayPath)) {
        qDebug() << "WARNING: failed to save grayscale debug image to:" << grayPath;
    }
    else {
        qDebug() << "Saved grayscale debug image to:" << grayPath;
    }

    // --- ZXing reader options ---
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

    // --- Create ZXing ImageView (IMPORTANT: stride = bytesPerLine) ---
    ZXing::ImageView view(
        gray.constBits(),
        gray.width(),
        gray.height(),
        ZXing::ImageFormat::Lum,
        gray.bytesPerLine()
    );

    // --- Decode ALL barcodes in image ---
    auto results = ZXing::ReadBarcodes(view, opts);

    qDebug() << "ZXing results count:" << (int)results.size();
    for (int i = 0; i < (int)results.size(); ++i) {
        qDebug() << "Result" << i
            << "format=" << (int)results[i].format()
            << "text=" << QString::fromStdString(results[i].text());
    }

    if (results.empty()) {
        qDebug() << "Decode: no barcodes found";
        return out;
    }

    out.ok = true;

    if (results.size() == 1) {
        out.format = QString::fromStdString(ZXing::ToString(results[0].format()));
        out.text = QString::fromStdString(results[0].text());
        return out;
    }

    out.format = "MULTI";
    QStringList lines;
    lines.reserve((int)results.size());

    for (auto& r : results) {
        lines << QString("%1: %2")
            .arg(QString::fromStdString(ZXing::ToString(r.format())))
            .arg(QString::fromStdString(r.text()));
    }

    out.text = lines.join("\n");
    return out;
}

