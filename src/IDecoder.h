#pragma once
#pragma once
#include <QImage>
#include <QString>

struct DecodeResult
{
    bool ok = false;
    QString format;
    QString text;
};

class IDecoder
{
public:
    virtual ~IDecoder() = default;
    virtual DecodeResult decode(const QImage& img) const = 0;
    virtual const char* name() const = 0;
};
