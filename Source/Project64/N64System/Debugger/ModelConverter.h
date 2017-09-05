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

#pragma once

#include <stdafx.h>

typedef struct
{
    int16_t x, y, z;
    unsigned : 16;
    int16_t tex_u, tex_v;
    union
    {
        uint8_t r, g, b;
        struct
        {
            int8_t normal_x, normal_y, normal_z;
        };
    };
    uint8_t alpha;
} VERTEX;

//typedef S15_16 RSPMAT4[4][4];
//
//typedef union {
//    int32_t bin;
//    struct {
//        int16_t intpart;
//        uint16_t fracpart;
//    };
//} S15_16;
//
//typedef struct
//{
//    S15_16 x, y, z, w;
//} VEC4;

/*
float FixedToFloat(int16_t intpart, uint16_t fracpart)
{
    bool sign = intpart < 0;
    float result = (float)intpart;
    if(!sign)
    {
        for(int i = 0; i < 16; i++)
        {
            int bit = fracpart & (0x8000 >> i);
            if(bit != 0)
            {
                result += (double)1 / (1 << (i + 1));
            }
        }
        return result;
    }

    for(int i = 0; i < 16; i++)
    {
        int bit = fracpart & (0x8000 >> i);
        if(bit != 0)
        {
            result -= (double)1 / (1 << (i + 1));
        }
    }

    return result;
}

S15_16 FixedMultiply(S15_16 a, S15_16 b)
{
    S15_16 result;

    int64_t c = a.n32 * b.n32;
    result.bin = (c >> 16) & 0xFFFFFFFF;

    return result;
}

S15_16 Mat4Get(RCPMAT4& mat4, int row, int col)
{
    S15_16 n;
    n.intpart = mat4.intpart[row][col];
    n.fracpart = mat4.fracpart[row][col];

    return n;
}

VEC4 TransformVec4(VEC4 vec4, RCPMAT4 mat4)
{
    VEC4 vec4t;

    vec4t.x = 
}

*/

typedef struct
{
    VERTEX vertex;
    int convertedIndex;
    bool bUsed;
} CVT_VERTEX;

typedef struct
{
    int cv0, cv1, cv2; // converted indeces
} CVT_TRIANGLE;

class CModelConverter
{
    // Acts like an rcp vertex buffer
    //  but vertices can be marked with a final index
    CVT_VERTEX m_TempVertexBuffer[32];

    // Intermediate model data
    vector<CVT_VERTEX> m_ConvertedVertices;
    vector<CVT_TRIANGLE> m_ConvertedTriangles;
    
public:
    bool LoadTemporaryVertices(uint32_t pAddr, uint8_t vbidx, uint8_t numv);
    int  AddVertexIfNew(uint8_t vbidx);
    void AddTriangle(int cv0, int cv1, int cv2);
    void ExportWavefrontOBJ();
    void ExportColladaDAE();
};
