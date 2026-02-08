#include "ZBarDecoder.h"
#include <QImage>
#include <QStringList>
#include <zbar.h>

using namespace zbar;

static QString typeName(zbar_symbol_type_t t)
{
    const char* n = zbar_get_symbol_name(t);
    return n ? QString::fromUtf8(n) : QString("UNKNOWN");
}

DecodeResult ZBarDecoder::decode(const QImage& img) const
{
    DecodeResult out;
    if (img.isNull())
        return out;

    QImage gray = img.convertToFormat(QImage::Format_Grayscale8);

    zbar_image_scanner_t* scanner = zbar_image_scanner_create();
    zbar_image_scanner_set_config(scanner, ZBAR_NONE, ZBAR_CFG_ENABLE, 1);

    zbar_image_t* zimg = zbar_image_create();
    zbar_image_set_format(zimg, zbar_fourcc('Y', '8', '0', '0'));
    zbar_image_set_size(zimg, gray.width(), gray.height());

    const int dataSize = gray.bytesPerLine() * gray.height();
    unsigned char* data = (unsigned char*)malloc((size_t)dataSize);
    memcpy(data, gray.constBits(), (size_t)dataSize);
    zbar_image_set_data(zimg, data, (unsigned int)dataSize, zbar_image_free_data);

    const int n = zbar_scan_image(scanner, zimg);
    if (n <= 0) {
        zbar_image_destroy(zimg);
        zbar_image_scanner_destroy(scanner);
        return out;
    }

    QStringList lines;
    const zbar_symbol_set_t* syms = zbar_image_get_symbols(zimg);
    for (const zbar_symbol_t* s = zbar_symbol_set_first_symbol(syms);
        s;
        s = zbar_symbol_next(s))
    {
        const auto t = zbar_symbol_get_type(s);
        const char* txt = zbar_symbol_get_data(s);
        lines << QString("%1: %2").arg(typeName(t),
            txt ? QString::fromUtf8(txt) : QString());
    }

    out.ok = true;
    if (lines.size() == 1) {
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
