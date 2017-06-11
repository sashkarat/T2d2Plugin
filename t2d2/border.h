#ifndef BORDER_H
#define BORDER_H

#include "t2d2.h"

#define T2D2_MAXBORDERS 32

namespace t2d2 {

class Border
{
    float m_offset;
    float m_width;
    float m_uOffset;
    float m_vOffset;
    float m_uScale;
    float m_vScale;

public:

    Border();

    inline float offset() const  { return m_offset; }

    inline void setOffset(float offset) { m_offset = offset; }

    inline float width() const {return m_width; }

    inline void setWidth(float width) { m_width = width; }

    inline float uOffset() const { return m_uOffset; }

    inline void setUOffset(float uOffset) { m_uOffset = uOffset; }

    inline float vOffset() const { return m_vOffset; }

    inline void setVOffset(float vOffset) { m_vOffset = vOffset; }

    inline float uScale() const {return m_uScale; }

    inline void setUScale(float uScale) { m_uScale = uScale; }

    inline float vScale() const { return m_vScale; }

    inline void setVScale(float vScale) { m_vScale = vScale; }
};


class Borders {
    unsigned int m_bordersMask;
    Border m_borders[T2D2_MAXBORDERS];
public:
    Borders() : m_bordersMask(0x00) {}
    inline static unsigned int  mask(unsigned int index)            { return (1 << index); }
    inline unsigned int         borderMask()                        { return m_bordersMask;}
    inline void                 setBordersMask(unsigned int mask)   {m_bordersMask = mask;}
    inline bool                 borderEnabled(unsigned int index)   {return (m_bordersMask & mask(index)) != 0;}
    inline Border*              border(unsigned int index)          {return m_borders + index; }
};

}
#endif // BORDER_H
