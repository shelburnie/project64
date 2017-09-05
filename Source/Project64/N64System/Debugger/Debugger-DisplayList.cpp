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
#include "DebuggerUI.h"

CDebugDisplayList::CDebugDisplayList(CDebuggerUI * debugger) :
    CDebugDialog<CDebugDisplayList>(debugger),
    m_RAMSnapshot(NULL),
    m_bFullRefreshPending(false)
{
}

CDebugDisplayList::~CDebugDisplayList(void)
{

}

LRESULT	CDebugDisplayList::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    DlgResize_Init(false, true);

    m_CommandList.Attach(GetDlgItem(IDC_CMD_LIST));
    m_CommandList.ModifyStyle(LVS_OWNERDRAWFIXED, 0, 0);
    m_CommandList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);
    m_CommandList.AddColumn("Address", 0);
    m_CommandList.AddColumn("Command", 1);
    m_CommandList.AddColumn("Command", 1);
    m_CommandList.SetColumnWidth(0, 70);
    m_CommandList.SetColumnWidth(1, 120);
    m_CommandList.SetColumnWidth(2, 400);

    m_SegmentList.Attach(GetDlgItem(IDC_SEGMENTS_LIST));
    m_SegmentList.ModifyStyle(LVS_OWNERDRAWFIXED, 0, 0);
    m_SegmentList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);
    m_SegmentList.AddColumn("No", 0);
    m_SegmentList.AddColumn("Address", 1);
    m_SegmentList.SetColumnWidth(0, 30);
    m_SegmentList.SetColumnWidth(1, 70);
    
    for (int i = 0; i < 16; i++)
    {
        m_SegmentList.AddItem(i, 0, stdstr_f("%02X", i).c_str());
        m_SegmentList.AddItem(i, 1, "00000000");
    }

    WindowCreated();
    return TRUE;
}

void CDebugDisplayList::CreateRAMSnapshot()
{
    if (g_MMU == NULL)
    {
        return;
    }

    uint8_t* ram = g_MMU->Rdram();
    uint32_t size = g_MMU->RdramSize();

    DeleteRAMSnapshot();
    m_RAMSnapshot = new uint8_t[size];
    memcpy(m_RAMSnapshot, ram, size);
}

void CDebugDisplayList::DeleteRAMSnapshot()
{
    if (m_RAMSnapshot != NULL)
    {
        delete[] m_RAMSnapshot;
        m_RAMSnapshot = NULL;
    }
}

LRESULT CDebugDisplayList::OnDestroy(void)
{
    DeleteRAMSnapshot();
    return 0;
}

LRESULT CDebugDisplayList::OnClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    switch (wID)
    {
    case IDCANCEL:
        EndDialog(0);
        break;
    case IDC_REFRESH_BTN:
        m_bFullRefreshPending = true;
        break;
    }
    return FALSE;
}

void CDebugDisplayList::Refresh()
{
    if (g_MMU == NULL || !IsWindowVisible())
    {
        return;
    }
    
    CreateRAMSnapshot();

    uint32_t ucEngineCodeAddr, ucEngineCodeSize, dlistAddr, dlistSize;
    
    g_MMU->LW_VAddr(0xA4000FD0, ucEngineCodeAddr);
    g_MMU->LW_VAddr(0xA4000FF0, dlistAddr);
    g_MMU->LW_VAddr(0xA4000FF4, dlistSize);
    
    uint8_t* ucEngineCode = g_MMU->Rdram() + ucEngineCodeAddr;

    m_Decoder.DetectEngine(ucEngineCode);

    if (!m_Decoder.IsEngineHandled())
    {
        return;
    }

    SetWindowTextA(stdstr_f("Display List - %s", m_Decoder.GetEngineName()).c_str());

    m_ListColors.clear();
    
    if (m_bFullRefreshPending)
    {
        m_CommandList.SetRedraw(FALSE);
        m_CommandList.DeleteAllItems();
    }

    /* Use stack to decide when the master list is finished
        and for indentation */

    uint32_t stack[16];
    int stackIndex = 0;

    uint32_t curAddr = dlistAddr;

    /* Decode the microcode into the command list, push face/vertex
        data to conversion-ready buffers during the pass */
    char szTree[1024];
    int tree_idx = 0;

    tree_idx += sprintf(szTree + tree_idx, "%08X ", curAddr);

    for(int i = 0;; i++)
    {
        uint32_t cmd0, cmd1;
        g_MMU->LW_PAddr(curAddr + 0, cmd0);
        g_MMU->LW_PAddr(curAddr + 4, cmd1);
        uint8_t operation = cmd0 >> 24;
        CUCodeCommand command = m_Decoder.DecodeCommand(cmd0, cmd1);
        UCODE_CMD genCmd = command.GetGenericCommand();

        if (m_bFullRefreshPending)
        {
            m_CommandList.AddItem(i, 0, stdstr_f("%08X", curAddr).c_str());
            m_CommandList.AddItem(i, 1, stdstr_f("%08X %08X", cmd0, cmd1).c_str());
            m_CommandList.AddItem(i, 2, stdstr_f("%*s%s", stackIndex * 4, "", command.ToString()).c_str());
            m_ListColors.push_back(command.GetUIColors());
        }
        
        curAddr += 8;
        
        if (genCmd == CMD_G_DL)
        {
            // Push current address to stack, jump to target address
            CMD_INFO_G_DL cmdInfo;
            command.GetInfo_G_DL(cmdInfo);

            tree_idx += sprintf(szTree + tree_idx, "%08X ", cmdInfo.segoffset);

            if (stackIndex < 16 && !cmdInfo.bBranch)
            {
                stack[stackIndex] = curAddr;
                stackIndex++;
            }
            curAddr = SegmentedToPhysical(cmdInfo.segoffset);
            continue;
        }

        if (genCmd == CMD_G_ENDDL)
        {
            // Pop stack and return
            if (stackIndex == 0)
            {
                break;
            }
            stackIndex--;
            curAddr = stack[stackIndex];

            tree_idx += sprintf(szTree + tree_idx, "\r\n%*s", (stackIndex+1)*9, "");

            continue;
        }

        if (genCmd == CMD_G_MOVEWORD)
        {
            // If index is G_MW_SEGMENT, update the segment address in the list
            CMD_INFO_G_MOVEWORD cmdInfo;
            command.GetInfo_G_MOVEWORD(cmdInfo);

            if (cmdInfo.index == G_MW_SEGMENT)
            {
                uint8_t segno = (cmdInfo.offset / 4) & 0x0F;
                uint32_t address = cmdInfo.data;
                
                if (m_RDPSegments[segno] != address)
                {
                    m_RDPSegments[segno] = address;
                    m_SegmentList.SetItemText(segno, 1, stdstr_f("%08X", address).c_str());
                }
            }
            continue;
        }
        
        if (m_bFullRefreshPending)
        {
            if (genCmd == CMD_G_VTX)
            {
                CMD_INFO_G_VTX cmdInfo;
                command.GetInfo_G_VTX(cmdInfo);
                uint32_t pAddr = SegmentedToPhysical(cmdInfo.segoffset);
                m_ModelConverter.LoadTemporaryVertices(pAddr, cmdInfo.vbidx, cmdInfo.numv);
                continue;
            }

            if (genCmd == CMD_G_TRI1)
            {
                CMD_INFO_G_TRI1 cmdInfo;
                command.GetInfo_G_TRI1(cmdInfo);
                m_ModelConverter.AddTriangle(cmdInfo.v0, cmdInfo.v1, cmdInfo.v2);
            }
        }
    }

    if (m_bFullRefreshPending)
    {
        //MessageBox(tree);
        m_CommandList.SetRedraw(TRUE);
        m_ModelConverter.ExportWavefrontOBJ();
        m_bFullRefreshPending = false;
    }
}

LRESULT	CDebugDisplayList::OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (wParam == IDC_SEGMENTS_LIST || wParam == IDC_CMD_LIST)
    {
        MEASUREITEMSTRUCT* lpMeasureItem = (MEASUREITEMSTRUCT*)lParam;
        lpMeasureItem->itemHeight = 13;
    }
    return FALSE;
}


LRESULT CDebugDisplayList::OnCustomDrawList(NMHDR* pNMHDR)
{
    NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
    DWORD drawStage = pLVCD->nmcd.dwDrawStage;

    HDC hDC = pLVCD->nmcd.hdc;

    switch (drawStage)
    {
    case CDDS_PREPAINT:
        return (CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT);
    case CDDS_ITEMPREPAINT:
        return CDRF_NOTIFYSUBITEMDRAW;
    case (CDDS_ITEMPREPAINT | CDDS_SUBITEM):
        break;
    default:
        return CDRF_DODEFAULT;
    }

    DWORD nItem = pLVCD->nmcd.dwItemSpec;
    DWORD nSubItem = pLVCD->iSubItem;

    if (nSubItem == 0)
    {
        pLVCD->clrTextBk = RGB(0xEE, 0xEE, 0xEE);
        pLVCD->clrText = RGB(0x44, 0x44, 0x44);
    }
    else
    {
        UI_COLOR_PAIR colors = m_ListColors[nItem];
        pLVCD->clrTextBk = colors.bg;
        pLVCD->clrText = colors.fg;
    }

    return CDRF_DODEFAULT;
}

uint32_t CDebugDisplayList::SegmentedToPhysical(uint32_t segmentOffset)
{
    uint8_t segno = segmentOffset >> 24;
    uint32_t offset = segmentOffset & 0x00FFFFFF;
    
    return m_RDPSegments[segno] + offset;
}

uint32_t CDebugDisplayList::SegmentedToVirtual(uint32_t segmentOffset)
{
    return 0x80000000 | SegmentedToPhysical(segmentOffset);
}