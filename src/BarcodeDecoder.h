#pragma once
#include <QString>
#include <QImage>
#include "IDecoder.h"   // tuo DecodeResult

/*
struct DecodeResult
{
    bool ok = false;
    QString text;
    QString format;
};
*/

class BarcodeDecoder
{
public:
    DecodeResult decode(const QImage& img) const;
};