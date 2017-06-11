#include "border.h"

unsigned int t2d2::Border::g_bordersMask = 0xFFFFFFFF;
t2d2::Border t2d2::Border::g_borders[32];

float t2d2::Border::offset() const
{
    return m_offset;
}

void t2d2::Border::setOffset(float offset)
{
    m_offset = offset;
}

float t2d2::Border::width() const
{
    return m_width;
}

void t2d2::Border::setWidth(float width)
{
    m_width = width;
}

float t2d2::Border::uOffset() const
{
    return m_uOffset;
}

void t2d2::Border::setUOffset(float uOffset)
{
    m_uOffset = uOffset;
}

float t2d2::Border::vOffset() const
{
    return m_vOffset;
}

void t2d2::Border::setVOffset(float vOffset)
{
    m_vOffset = vOffset;
}

float t2d2::Border::uScale() const
{
    return m_uScale;
}

void t2d2::Border::setUScale(float uScale)
{
    m_uScale = uScale;
}

float t2d2::Border::vScale() const
{
    return m_vScale;
}

void t2d2::Border::setVScale(float vScale)
{
    m_vScale = vScale;
}

t2d2::Border *t2d2::Border::border(unsigned int index)
{
    return g_borders + index;
}

t2d2::Border::Border() :
    m_offset(0),
    m_width(1),
    m_uOffset(0),
    m_vOffset(0),
    m_uScale(1),
    m_vScale(1)
{
}
