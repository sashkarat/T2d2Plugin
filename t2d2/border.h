#ifndef BORDER_H
#define BORDER_H

#include "t2d2.h"

#define T2D2_MAXBORDERS 32

namespace t2d2 {

class Border
{

static unsigned int g_bordersMask;
static Border g_borders[32];

    float m_offset;
    float m_width;
    float m_uOffset;
    float m_vOffset;
    float m_uScale;
    float m_vScale;

public:

    inline static unsigned int  borderMask()                    { return g_bordersMask; }
    inline static void          setBorderMask(unsigned int bm)  { g_bordersMask = bm; }
    inline static unsigned int  maxBorders() { return 32; }
    inline static unsigned int  mask(unsigned int index)        { return (1 << index); }
    inline static bool          borderEnabled(unsigned int index) {return (g_bordersMask & mask(index)) != 0;}
    static Border *             border(unsigned int index);

    Border();

    inline float offset() const;

    inline void setOffset(float offset);

    inline float width() const;

    inline void setWidth(float width);

    inline float uOffset() const;

    inline void setUOffset(float uOffset);

    inline float vOffset() const;

    inline void setVOffset(float vOffset);

    inline float uScale() const;

    inline void setUScale(float uScale);

    inline float vScale() const;

    inline void setVScale(float vScale);
};


class Borders {
    unsigned int m_borderMask;
    Border m_borders[T2D2_MAXBORDERS];
};

}
#endif // BORDER_H
