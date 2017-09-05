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

#include "DListDecoder.h"
#include "ModelConverter.h"

class CDebugDisplayList :
    public CDebugDialog<CDebugDisplayList>,
    public CDialogResize<CDebugDisplayList>
{
public:
    enum { IDD = IDD_Debugger_DisplayList };

    CDebugDisplayList(CDebuggerUI * debugger);
    virtual ~CDebugDisplayList(void);

    void Refresh();

private:
    
    CDListDecoder m_Decoder;
    CModelConverter m_ModelConverter;

    uint32_t m_RDPSegments[16];
    
    bool m_bFullRefreshPending;

    uint8_t* m_RAMSnapshot;
    void CreateRAMSnapshot();
    void DeleteRAMSnapshot();

    CListViewCtrl m_CommandList;
    vector<UI_COLOR_PAIR> m_ListColors;

    CListViewCtrl m_SegmentList;

    uint32_t SegmentedToPhysical(uint32_t segmentOffset);
    uint32_t SegmentedToVirtual(uint32_t segmentOffset);
    
    LRESULT	OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnDestroy(void);
    LRESULT	OnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT	OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnCustomDrawList(NMHDR* pNMHDR);

    BEGIN_MSG_MAP_EX(CDebugDisplayList)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_CODE_HANDLER(BN_CLICKED, OnClicked)
        MSG_WM_DESTROY(OnDestroy)
        MESSAGE_HANDLER(WM_MEASUREITEM, OnMeasureItem)
        NOTIFY_HANDLER_EX(IDC_CMD_LIST, NM_CUSTOMDRAW, OnCustomDrawList)
        CHAIN_MSG_MAP(CDialogResize<CDebugDisplayList>)
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(CDebugDisplayList)
        DLGRESIZE_CONTROL(IDC_CMD_LIST, DLSZ_SIZE_Y | DLSZ_SIZE_X)
        DLGRESIZE_CONTROL(IDC_SEGMENTS_LIST, DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(IDC_SEGMENTS_BOX, DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(IDC_INFO_BOX, DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(IDC_ADDR_BOX, DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(IDC_REFRESH_BTN, DLSZ_MOVE_X)
    END_DLGRESIZE_MAP()
};