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
#include "stdafx.h"

#define G_MW_MATRIX     0
#define G_MW_NUMLIGHT   2
#define G_MW_CLIP       4
#define G_MW_SEGMENT    6
#define G_MW_FOG        8
#define G_MW_LIGHTCOL  10
#define G_MW_FORCEMTX  12
#define G_MW_PERSPNORM 14

typedef struct
{
    COLORREF fg;
    COLORREF bg;
} UI_COLOR_PAIR; // colors for the cmd list

enum UCODE_ENGINE
{
    UC_UNHANDLED,
    UC_F3D,
    UC_F3DEX,
    UC_F3DEX2
};

enum UCODE_CMD
{
    CMD_UNHANDLED,
    CMD_OTHER,
    
    CMD_G_DL,
    CMD_G_ENDDL,
    CMD_G_VTX,
    CMD_G_TRI1,
    CMD_G_TRI2,
    CMD_G_QUAD,
    CMD_G_MOVEWORD,
    CMD_G_SETTIMG,
    CMD_G_RDPPIPESYNC,
    CMD_G_RDPLOADSYNC,
    CMD_G_RDPTILESYNC,
    CMD_G_RDPFULLSYNC
};

typedef struct
{
    bool bBranch;
    uint32_t address;
    uint32_t segoffset;
} CMD_INFO_G_DL;

typedef struct 
{
    uint8_t index;
    uint16_t offset;
    uint32_t data;
} CMD_INFO_G_MOVEWORD;

typedef struct
{
    uint8_t numv;
    uint8_t vbidx;
    uint32_t segoffset;
} CMD_INFO_G_VTX;

typedef struct
{
    uint8_t v0, v1, v2;
} CMD_INFO_G_TRI1;

class CUCodeCommand
{
    UCODE_ENGINE m_UCodeEngine;
    UCODE_CMD m_OpEnum;

    uint8_t  m_Op;
    uint32_t m_Cmd0, m_Cmd1;
    
    char m_szBuf[256];

public:
    CUCodeCommand(UCODE_ENGINE uCodeClass, uint32_t cmd0, uint32_t cmd1);
    const char* ToString();
    UCODE_CMD GetGenericCommand();

    UI_COLOR_PAIR GetUIColors();

    bool GetInfo_G_DL(CMD_INFO_G_DL& cmdInfo);
    bool GetInfo_G_MOVEWORD(CMD_INFO_G_MOVEWORD& cmdInfo);
    bool GetInfo_G_VTX(CMD_INFO_G_VTX& cmdInfo);
    bool GetInfo_G_TRI1(CMD_INFO_G_TRI1& cmdInfo);

private:
    bool Decode_F3D();

    void d_Unhandled();

    void d_gsDPSetTextureImage();
    void d_gsDPSetTile();
    void d_gsSPDisplayList();
    void d_gsSPEndDisplayList();
    void d_gsSPVertex();
    void d_gsSP1Triangle();
    void d_gsMoveWd();
    void d_gsDPNoOpTag();
    void d_gsSPClearGeometryMode();
    void d_gsSPGeometryMode();
    void d_gsSPOtherMode_lo();
    void d_gsSPOtherMode_hi();
    void d_gsSPMatrix();
    void d_gsDPPipeSync();
    void d_gsDPSetColorImage();
    void d_gsDPWord();
    void d_gsDPSetScissor();
    void d_gsDPSetCombine();
    void d_gsSPTexture();
    void d_gsDPSetDepthImage();
    void d_gsDPSetFillColor();
    void d_gsDPFillRectangle();
    void d_gsMoveMem();
    void d_gsDPTileSync();
    void d_gsDPSetTileSize();
    void d_gsDPLoadSync();
    void d_gsDPLoadBlock();
    void d_gsSPPopMatrixN();
    void d_gsDPSetEnvColor();
    void d_gsDPFullSync();
};

class CDListDecoder
{
    friend class CUCodeCommand;

    UCODE_ENGINE m_UCodeEngine;

public:

    void DetectEngine(uint8_t* ucEngineCode)
    {
        // Method copied from Project64-Video

        uint32_t checksum = 0;

        for (int i = 0; i < 3072; i += sizeof(uint32_t))
        {
            checksum += *(uint32_t*)&ucEngineCode[i];
        }

        switch (checksum)
        {
        case 0x3A1CBAC3:
            m_UCodeEngine = UC_F3D;
            break;
        default:
            MessageBox(NULL, stdstr_f("ucode check %08X", checksum).c_str(), "", MB_OK);
            m_UCodeEngine = UC_UNHANDLED;
            break;
        }
    }

    bool IsEngineHandled()
    {
        return m_UCodeEngine != UC_UNHANDLED;
    }

    const char* GetEngineName()
    {
        switch (m_UCodeEngine)
        {
        case UC_F3D: return "Fast3D";
        }
        return "Unknown";
    }

    CDListDecoder() :
        m_UCodeEngine(UC_UNHANDLED)
    {
    }
    
    UCODE_ENGINE GetEngine()
    {
        return m_UCodeEngine;
    }

    CUCodeCommand DecodeCommand(uint32_t cmd0, uint32_t cmd1)
    {
        return CUCodeCommand(m_UCodeEngine, cmd0, cmd1);
    }
};