/****************************************************************************
*                                                                           *
* Project64 - A Nintendo 64 emulator.                                      *
* http://www.pj64-emu.com/                                                  *
* Copyright (C) 2012 Project64. All rights reserved.                        *
*                                                                           *
* License:                                                                  *
* GNU/GPLv2 http://www.gnu.org/licenses/gpl-2.0.html                        *
*                                                                           *
****************************************************************************/

#include "stdafx.h"
#include "ModelConverter.h"

bool CModelConverter::LoadTemporaryVertices(uint32_t pAddr, uint8_t vbidx, uint8_t numv)
{
    for (int i = vbidx; i < numv; i++)
    {
        uint32_t addr = pAddr + i * 16;
        CVT_VERTEX* cvertex = &m_TempVertexBuffer[i];
        VERTEX* vertex = &cvertex->vertex;

        g_MMU->LH_PAddr(addr + 0x00, (uint16_t&)vertex->x);
        g_MMU->LH_PAddr(addr + 0x02, (uint16_t&)vertex->y);
        g_MMU->LH_PAddr(addr + 0x04, (uint16_t&)vertex->z);

        g_MMU->LH_PAddr(addr + 0x08, (uint16_t&)vertex->tex_u);
        g_MMU->LH_PAddr(addr + 0x0A, (uint16_t&)vertex->tex_u);

        g_MMU->LB_PAddr(addr + 0x0C, vertex->r);
        g_MMU->LB_PAddr(addr + 0x0D, vertex->g);
        g_MMU->LB_PAddr(addr + 0x0E, vertex->b);
        g_MMU->LB_PAddr(addr + 0x0F, vertex->alpha);

        cvertex->bUsed = false;
    }

    return true;
}

int CModelConverter::AddVertexIfNew(uint8_t vbidx)
{
    CVT_VERTEX* cvertex = &m_TempVertexBuffer[vbidx];

    if (cvertex->bUsed == false)
    {
        cvertex->convertedIndex = m_ConvertedVertices.size();
        cvertex->bUsed = true;
        m_ConvertedVertices.push_back(*cvertex);
    }

    return cvertex->convertedIndex;
}

void CModelConverter::AddTriangle(int v0, int v1, int v2)
{
    CVT_TRIANGLE triangle;

    triangle.cv0 = AddVertexIfNew(v0);
    triangle.cv1 = AddVertexIfNew(v1);
    triangle.cv2 = AddVertexIfNew(v2);

    m_ConvertedTriangles.push_back(triangle);
}

void CModelConverter::ExportWavefrontOBJ()
{
    FILE* meme = fopen("test-convert.obj", "wb");
    fprintf(meme, "o 1\r\n\r\n");

    for (int i = 0; i < m_ConvertedVertices.size(); i++)
    {
        int16_t x = m_ConvertedVertices[i].vertex.x;
        int16_t y = m_ConvertedVertices[i].vertex.y;
        int16_t z = m_ConvertedVertices[i].vertex.z;
        fprintf(meme, "v %d %d %d\r\n", x, y, z);
    }

    fprintf(meme, "\r\nusemtl Default\r\n\r\n");

    for (int i = 0; i < m_ConvertedTriangles.size(); i++)
    {
        int v0 = m_ConvertedTriangles[i].cv0;
        int v1 = m_ConvertedTriangles[i].cv1;
        int v2 = m_ConvertedTriangles[i].cv2;
        fprintf(meme, "f %d %d %d\r\n", v0 + 1, v1 + 1, v2 + 1);
    }

    fclose(meme);
}