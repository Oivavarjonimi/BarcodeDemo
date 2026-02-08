#ifdef USE_ZBAR

#include "BarcodeDecoder.h"

#include <QImage>
#include <QString>
#include <QDebug>

// ZBar C API
#include <zbar.h>

static QString zbarTypeToString(zbar_symbol_type_t t)
{
    // ZBar returns enums like ZBAR_QRCODE, ZBAR_EAN13, ZBAR_CODE128...
    const char* n = zbar_get_symbol_name(t);
    return n ? QString::fromUtf8(n) : QString("UNKNOWN");
}

DecodeResult BarcodeDecoder::decode(const QImage& img) const
{
    DecodeResult out;
    if (img.isNull())
        return out;

    // ZBar likes 8-bit grayscale ("Y800")
    QImage gray = img.convertToFormat(QImage::Format_Grayscale8);

    // Create scanner
    zbar_image_scanner_t* scanner = zbar_image_scanner_create();

    // Enable everything (you can selectively enable types if you want)
    zbar_image_scanner_set_config(scanner, ZBAR_NONE, ZBAR_CFG_ENABLE, 1);

    // Wrap the image
    zbar_image_t* zimg = zbar_image_create();
    zbar_image_set_format(zimg, zbar_fourcc('Y', '8', '0', '0'));
    zbar_image_set_size(zimg, gray.width(), gray.height());

    // Provide data (copy to be safe)
    const int dataSize = gray.bytesPerLine() * gray.height();
    unsigned char* data = (unsigned char*)malloc((size_t)dataSize);
    memcpy(data, gray.constBits(), (size_t)dataSize);

    // Let ZBar free() it when done
    zbar_image_set_data(zimg, data, (unsigned int)dataSize, zbar_image_free_data);

    // Scan
    int n = zbar_scan_image(scanner, zimg);

    if (n <= 0) {
        zbar_image_destroy(zimg);
        zbar_image_scanner_destroy(scanner);
        return out;
    }

    // If multiple symbols, concatenate
    QStringList lines;

    const zbar_symbol_set_t* syms = zbar_image_get_symbols(zimg);
    for (const zbar_symbol_t* sym = zbar_symbol_set_first_symbol(syms);
        sym;
        sym = zbar_symbol_next(sym))
    {
        zbar_symbol_type_t type = zbar_symbol_get_type(sym);
        const char* txt = zbar_symbol_get_data(sym);

        const QString typeStr = zbarTypeToString(type);
        const QString textStr = txt ? QString::fromUtf8(txt) : QString();

        lines << QString("%1: %2").arg(typeStr, textStr);
    }

    out.ok = true;
    if (lines.size() == 1) {
        // split "TYPE: text"
        const QString line = lines[0];
        const int idx = line.indexOf(": ");
        out.format = (idx > 0) ? line.left(idx) : "UNKNOWN";
        out.text = (idx > 0) ? line.mid(idx + 2) : line;
    }
    else {
        out.format = "MULTI";
        out.text = lines.join("\n");
    }

    zbar_image_destroy(zimg);
    zbar_image_scanner_destroy(scanner);
    return out;
}

#endif // USE_ZBAR
