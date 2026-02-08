#pragma once
#pragma once
#include "IDecoder.h"
#include <zbar.h>
using namespace zbar;

class ZBarDecoder final : public IDecoder
{
public:
    DecodeResult decode(const QImage& img) const override;
    const char* name() const override { return "ZBar"; }
};
