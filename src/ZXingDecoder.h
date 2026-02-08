#pragma once
#pragma once
#include "IDecoder.h"

class ZXingDecoder final : public IDecoder
{
public:
    DecodeResult decode(const QImage& img) const override;
    const char* name() const override { return "ZXing"; }
};
