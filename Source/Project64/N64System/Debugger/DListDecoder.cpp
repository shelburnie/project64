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
#include "DListDecoder.h"

CUCodeCommand::CUCodeCommand(UCODE_ENGINE uCodeEngine, uint32_t cmd0, uint32_t cmd1):
    m_UCodeEngine(uCodeEngine),
    m_Cmd0(cmd0),
    m_Cmd1(cmd1),
    m_Op(cmd0 >> 24),
    m_OpEnum(CMD_OTHER)
{
    switch (m_UCodeEngine)
    {
    case UC_F3D:
        Decode_F3D();
        break;
    case UC_UNHANDLED:
        sprintf(m_szBuf, "No engine");
        // bad
        break;
    }
}

const char* CUCodeCommand::ToString()
{
    return m_szBuf;
}

UCODE_CMD CUCodeCommand::GetGenericCommand()
{
    return m_OpEnum;
}

UI_COLOR_PAIR CUCodeCommand::GetUIColors()
{
    UI_COLOR_PAIR colors;

    colors.bg = 0xFFFFFFFF;
    colors.fg = 0xFF000000;

    switch (m_OpEnum)
    {
    case CMD_G_DL:
        colors.fg = RGB(0x00, 0x66, 0x00);
        colors.bg = RGB(0xEE, 0xFF, 0xEE);
        break;
    case CMD_G_ENDDL:
        colors.fg = RGB(0x66, 0x00, 0x00);
        colors.bg = RGB(0xFF, 0xEE, 0xEE);
        break;
    case CMD_G_SETTIMG:
        colors.fg = RGB(0x77, 0x66, 0x00);
        colors.bg = RGB(0xFF, 0xEE, 0xDD);
        break;
    case CMD_G_VTX:
        colors.fg = RGB(0x11, 0x00, 0x55);
        colors.bg = RGB(0xBB, 0xCC, 0xEE);
        break;
    case CMD_G_TRI1:
    case CMD_G_TRI2:
    case CMD_G_QUAD:
        colors.fg = RGB(0x22, 0x00, 0x66);
        colors.bg = RGB(0xAA, 0xEE, 0xFF);
        break;
     case CMD_G_RDPPIPESYNC:
     case CMD_G_RDPLOADSYNC:
     case CMD_G_RDPTILESYNC:
     case CMD_G_RDPFULLSYNC:
        colors.fg = RGB(0x44, 0x44, 0x44);
        colors.bg = RGB(0xEE, 0xEE, 0xEE);
        break;
    }

    return colors;
}

bool CUCodeCommand::GetInfo_G_DL(CMD_INFO_G_DL& cmdInfo)
{
    if (m_OpEnum != CMD_G_DL)
    {
        return false;
    }
    cmdInfo.bBranch = (m_Cmd0 & 0x00FF0000) != 0;
    cmdInfo.address = m_Cmd1;
    cmdInfo.segoffset = m_Cmd1;
    return true;
}

bool CUCodeCommand::GetInfo_G_VTX(CMD_INFO_G_VTX& cmdInfo)
{
    if (m_OpEnum != CMD_G_VTX)
    {
        return false;
    }

    if (m_UCodeEngine == UC_F3D)
    {
        cmdInfo.numv = ((m_Cmd0 >> 16) & 0xFF) / 16 + 1;
        cmdInfo.vbidx = ((m_Cmd0 & 0xFFF) / 16) - cmdInfo.numv;
        cmdInfo.segoffset = m_Cmd1;
        return true;
    }
    return false; // layout is different for others
}

bool CUCodeCommand::GetInfo_G_MOVEWORD(CMD_INFO_G_MOVEWORD& cmdInfo)
{
    if (m_OpEnum != CMD_G_MOVEWORD)
    {
        return false;
    }
    
    if (m_UCodeEngine == UC_F3D)
    {
        cmdInfo.index = m_Cmd0 & 0xFF;
        cmdInfo.offset = (m_Cmd0 >> 8) & 0xFFFF;
        cmdInfo.data = m_Cmd1;
        return true;
    }
    
    return false; // layout is different for others
}

bool CUCodeCommand::GetInfo_G_TRI1(CMD_INFO_G_TRI1& cmdInfo)
{
    if (m_UCodeEngine == UC_F3D)
    {
        cmdInfo.v0 = ((m_Cmd1 >> 16) & 0xFF) / 10;
        cmdInfo.v1 = ((m_Cmd1 >> 8) & 0xFF) / 10;
        cmdInfo.v2 = ((m_Cmd1 >> 0) & 0xFF) / 10;
        return true;
    }

    return true; // others have them in cmd0
}

bool CUCodeCommand::Decode_F3D()
{
    switch (m_Op)
    {
    case 0x00: d_gsDPNoOpTag(); break;
    case 0x01: d_gsSPMatrix(); break;
    case 0x03: d_gsMoveMem(); break;
    case 0x04: d_gsSPVertex(); break;
    case 0x06: d_gsSPDisplayList(); break;
    case 0xB4: d_gsDPWord(); break;
    case 0xB6: d_gsSPClearGeometryMode(); break; // actual name?
    case 0xB7: d_gsSPGeometryMode(); break;
    case 0xB8: d_gsSPEndDisplayList(); break;
    case 0xB9: d_gsSPOtherMode_lo(); break;
    case 0xBA: d_gsSPOtherMode_hi(); break;
    case 0xBB: d_gsSPTexture(); break;
    case 0xBC: d_gsMoveWd(); break;
    case 0xBD: d_gsSPPopMatrixN(); break;
    case 0xBF: d_gsSP1Triangle(); break;
    case 0xE6: d_gsDPLoadSync(); break;
    case 0xE7: d_gsDPPipeSync(); break;
    case 0xE8: d_gsDPTileSync(); break;
    case 0xE9: d_gsDPFullSync(); break;
    case 0xED: d_gsDPSetScissor(); break;
    case 0xF2: d_gsDPSetTileSize(); break;
    case 0xF3: d_gsDPLoadBlock(); break;
    case 0xF5: d_gsDPSetTile(); break;
    case 0xF6: d_gsDPFillRectangle(); break;
    case 0xF7: d_gsDPSetFillColor(); break;
    case 0xFB: d_gsDPSetEnvColor(); break;
    case 0xFC: d_gsDPSetCombine(); break;
    case 0xFD: d_gsDPSetTextureImage(); break;
    case 0xFE: d_gsDPSetDepthImage(); break;
    case 0xFF: d_gsDPSetColorImage(); break;
    default:
        d_Unhandled();
        return false;
    }
    return true;
}

void CUCodeCommand::d_Unhandled()
{
    m_OpEnum = CMD_UNHANDLED;
    sprintf(m_szBuf, "??");
}

void CUCodeCommand::d_gsDPSetTextureImage()
{
    m_OpEnum = CMD_G_SETTIMG;

    uint32_t addr = m_Cmd1;
    int width = (m_Cmd0 & 0xFFF) + 1;
    int size = (m_Cmd0 >> 19) & 3;
    int fmt = (m_Cmd0 >> 21) & 7;
    sprintf(m_szBuf, "gsDPSetTextureImage ( fmt: %d, size: %d, w: %d, addr: 0x%08X )", fmt, size, width, addr);
}

void CUCodeCommand::d_gsDPSetTile()
{
    sprintf(m_szBuf, "gsDPSetTile ( )");
}

void CUCodeCommand::d_gsSPDisplayList() // or gsSPBranchList
{
    m_OpEnum = CMD_G_DL;

    uint32_t addr = m_Cmd1;
    if (m_Cmd0 & 0xFF0000)
    {
        sprintf(m_szBuf, "gsSPBranchList ( addr: 0x%08X )", addr);
        return;
    }
    sprintf(m_szBuf, "gsSPDisplayList ( addr: 0x%08X )", addr);
}

void CUCodeCommand::d_gsSPEndDisplayList()
{
    m_OpEnum = CMD_G_ENDDL;

    sprintf(m_szBuf, "gsSPEndDisplayList ( )");
}

void CUCodeCommand::d_gsSPVertex() // fast3d layout
{
    m_OpEnum = CMD_G_VTX;

    // 010nn0aa vvvvvvvv	gsSPVertex(vaddr, numv, vbidx)
    uint32_t addr = m_Cmd1;
    int numv = ((m_Cmd0 >> 16) & 0xFF) / 16 + 1;
    int vbidx = ((m_Cmd0 & 0xFFF) / 16) - numv;
    sprintf(m_szBuf, "gsSPVertex ( vaddr: 0x%08X, numv: %d, vbidx: %d )", addr, numv, vbidx);
}

void CUCodeCommand::d_gsSP1Triangle() // fast3d layout
{
    m_OpEnum = CMD_G_TRI1;

    int v0 = ((m_Cmd1 >> 16) & 0xFF) / 10;
    int v1 = ((m_Cmd1 >> 8) & 0xFF) / 10;
    int v2 = ((m_Cmd1 >> 0) & 0xFF) / 10;
    sprintf(m_szBuf, "gsSP1Triangle ( %d, %d, %d )", v0, v1, v2);
}

void CUCodeCommand::d_gsMoveWd() // fast3d layout
{
    m_OpEnum = CMD_G_MOVEWORD;

    uint8_t index = m_Cmd0 & 0xFF;
    uint16_t offs = (m_Cmd0 >> 8) & 0xFF;

    const char* szIndexName;

    switch (index)
    {
        //taken from cloudmodding f3dzex notes, todo check if fast3d same
    case 0x00: szIndexName = "G_MW_MATRIX"; break;
    case 0x02: szIndexName = "G_MW_NUMLIGHT"; break;
    case 0x04: szIndexName = "G_MW_CLIP"; break;
    case 0x06: szIndexName = "G_MW_SEGMENT"; break;
    case 0x08: szIndexName = "G_MW_FOG"; break;
    case 0x0A: szIndexName = "G_MW_LIGHTCOL"; break;
    case 0x0C: szIndexName = "G_MW_FORCEMTX"; break;
    case 0x0E: szIndexName = "G_MW_PERSPNORM"; break;
    default:
    {
        char name[16];
        sprintf(name, "0x%02X", index);
        szIndexName = name;
    }
    }

    sprintf(m_szBuf, "gsMoveWd ( %s, 0x%02X, 0x%08X )", szIndexName, offs, m_Cmd1);
}

void CUCodeCommand::d_gsDPNoOpTag()
{
    sprintf(m_szBuf, "gsDPNoOpTag ( 0x%08X )", m_Cmd1);
}

void CUCodeCommand::d_gsSPClearGeometryMode()
{
    sprintf(m_szBuf, "gsSPClearGeometryMode ( )");
}

void CUCodeCommand::d_gsSPGeometryMode()
{
    sprintf(m_szBuf, "gsSPGeometryMode ( )");
}

void CUCodeCommand::d_gsSPOtherMode_lo()
{
    sprintf(m_szBuf, "gsSPOtherModeHigh ( ,, )");
}

void CUCodeCommand::d_gsSPOtherMode_hi()
{
    sprintf(m_szBuf, "gsSPOtherModeLow ( ,, )");
}

void CUCodeCommand::d_gsSPMatrix()
{
    uint32_t addr = m_Cmd1;
    sprintf(m_szBuf, "gsSPMatrix ( 0x%08X, params )", addr);
}

void CUCodeCommand::d_gsDPPipeSync()
{
    m_OpEnum = CMD_G_RDPPIPESYNC;
    sprintf(m_szBuf, "gsDPPipeSync ( )");
}
void CUCodeCommand::d_gsDPTileSync()
{
    m_OpEnum = CMD_G_RDPTILESYNC;
    sprintf(m_szBuf, "gsDPTileSync ( )");
}
void CUCodeCommand::d_gsDPLoadSync()
{
    m_OpEnum = CMD_G_RDPLOADSYNC;
    sprintf(m_szBuf, "gsDPLoadSync ( )");
}
void CUCodeCommand::d_gsDPFullSync()
{
    m_OpEnum = CMD_G_RDPFULLSYNC;
    sprintf(m_szBuf, "gsDPFullSync ( )");
}
void CUCodeCommand::d_gsDPSetColorImage()
{
    uint32_t addr = m_Cmd1;
    int width = (m_Cmd0 & 0xFFF) + 1;
    int size = (m_Cmd0 >> 19) & 3;
    int fmt = (m_Cmd0 >> 21) & 7;
    sprintf(m_szBuf, "gsDPSetColorImage ( fmt, siz, width, 0x%08X )", addr);
}

void CUCodeCommand::d_gsDPWord()
{
    sprintf(m_szBuf, "gsDPWord ( 0x%08X )", m_Cmd1);
}

void CUCodeCommand::d_gsDPSetScissor()
{
    const char* szModeName;

    int mode = m_Cmd1 >> 26;

    int ulx = (m_Cmd0 >> 12) & 0xFFF;
    int uly = m_Cmd0 & 0xFFF;
    int lrx = (m_Cmd1 >> 12) & 0xFFF;
    int lry = m_Cmd1 & 0xFFF;

    switch (mode)
    {
    case 0: szModeName = "G_SC_NON_INTERLACE"; break;
    case 2: szModeName = "G_SC_EVEN_INTERLACE"; break;
    case 3: szModeName = "G_SC_ODD_INTERLACE"; break;
    default:
    {
        char name[8];
        sprintf(name, "%02X", mode);
        szModeName = name;
    }
    }

    sprintf(m_szBuf, "gsDPSetScissor ( %s, %d, %d, %d, %d )", szModeName, ulx, uly, lrx, lry);
}

void CUCodeCommand::d_gsDPSetCombine()
{
    // from f3dzex, need check
    int a0 = (m_Cmd0 >> 20) & 0b1111;
    int c0 = (m_Cmd0 >> 15) & 0b11111;
    int Aa0 = (m_Cmd0 >> 12) & 0b111;
    int Ac0 = (m_Cmd0 >> 9) & 0b111;
    int a1 = (m_Cmd0 >> 5) & 0b1111;
    int c1 = (m_Cmd0 & 0b11111);
    int b0 = (m_Cmd1 >> 28) & 0b1111;
    int b1 = (m_Cmd1 >> 24) & 0b1111;
    int Aa1 = (m_Cmd1 >> 21) & 0b111;
    int Ac1 = (m_Cmd1 >> 18) & 0b111;
    int d0 = (m_Cmd1 >> 15) & 0b111;
    int Ab0 = (m_Cmd1 >> 12) & 0b111;
    int Ad0 = (m_Cmd1 >> 9) & 0b111;
    int d1 = (m_Cmd1 >> 6) & 0b111;
    int Ab1 = (m_Cmd1 >> 3) & 0b111;
    int Ad1 = (m_Cmd1 & 0b111);

    sprintf(m_szBuf, "gsDPSetCombineLERP ( %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d )",
        a0, c0, Aa0, Ac0, a1, c1, b0, b1, Aa1, Ac1, d0, Ab0, Ad0, d1, Ab1, Ad1);
}

void CUCodeCommand::d_gsSPTexture()
{
    int level = (m_Cmd0 >> 11) & 0b111;
    int tile = (m_Cmd0 >> 8) & 0b111;
    int on = m_Cmd0 & 0b111;
    int scaleS = (m_Cmd1 >> 16) & 0xFFFF;
    int scaleT = m_Cmd1 & 0xFFFF;

    sprintf(m_szBuf, "gsSPTexture ( %d, %d, %d, %d, %d )", scaleS, scaleT, level, tile, on);
}

void CUCodeCommand::d_gsDPSetDepthImage()
{
    sprintf(m_szBuf, "gsDPSetDepthImage ( imgaddr )");
}

void CUCodeCommand::d_gsDPSetFillColor()
{
    sprintf(m_szBuf, "gsDPSetFillColor ( color )");
}

void CUCodeCommand::d_gsDPFillRectangle()
{
    sprintf(m_szBuf, "gsDPFillRectangle ( ulx, uly, lrx, lry )");
}

void CUCodeCommand::d_gsMoveMem()
{
    sprintf(m_szBuf, "gsMoveMem ( size, index, offset, address )");
}

void CUCodeCommand::d_gsDPSetTileSize()
{
    sprintf(m_szBuf, "gsDPSetTileSize ( tile, uls, ult, lrs, lrt )");
}

void CUCodeCommand::d_gsDPLoadBlock()
{
    sprintf(m_szBuf, "gsDPLoadBlock ( tile, uls, ult, texels, dxt )");
}

void CUCodeCommand::d_gsSPPopMatrixN()
{
    sprintf(m_szBuf, "gsSPPopMatrixN ( which, num )");
}

void CUCodeCommand::d_gsDPSetEnvColor()
{
    sprintf(m_szBuf, "gsDPSetEnvColor ( R, G, B, A )");
}
