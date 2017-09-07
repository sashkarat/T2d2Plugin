#include "border.h"

t2d2::Border::Border() :
    m_offset(0),
    m_width(1),
    m_zOffset(0),
    m_uOffset(0),
    m_vOffset(0),
    m_uScale(1),
    m_vScale(1),
    m_subMeshIndex(0)
{
}

void t2d2::Border::saveToFile(t2d2::Border *b, std::ofstream &fs)
{
    static float fdata[8];

    fdata[0] = b->m_offset;
    fdata[1] = b->m_width;
    fdata[2] = b->m_zOffset;
    fdata[3] = b->m_uOffset;
    fdata[4] = b->m_vOffset;
    fdata[5] = b->m_uScale;
    fdata[6] = b->m_vScale;
    fdata[7] = static_cast<float>(b->m_subMeshIndex);

    fs.write((char*)fdata, sizeof(float) * 8);
}

void t2d2::Border::loadFromFile(t2d2::Border *b, std::ifstream &fs)
{
    static float fdata[8];

    fs.read((char*)fdata, sizeof(float)*8);

    b->m_offset = fdata[0];
    b->m_width = fdata[1];
    b->m_zOffset = fdata[2];
    b->m_uOffset = fdata[3];
    b->m_vOffset = fdata[4];
    b->m_uScale = fdata[5];
    b->m_vScale = fdata[6];
    b->m_subMeshIndex = static_cast<int>(fdata[7]);
}

void t2d2::Borders::saveToFile(t2d2::Borders *bs, std::ofstream &fs)
{
    fs.write((char*)&(bs->m_bordersMask), sizeof(int));

    for(int i = 0; i < T2D2_MAXBORDERS; i++)
        t2d2::Border::saveToFile(bs->m_borders + i, fs);
}

void t2d2::Borders::loadFromFile(t2d2::Borders *bs, std::ifstream &fs)
{
    static unsigned int bm;
    fs.read((char*)&bm, sizeof(int));
    bs->m_bordersMask = bm;

    for(int i = 0; i < T2D2_MAXBORDERS; i++)
        t2d2::Border::loadFromFile(bs->m_borders + i, fs);
}
