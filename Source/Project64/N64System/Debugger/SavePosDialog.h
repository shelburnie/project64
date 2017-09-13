#pragma once
#include "stdafx.h"

#include <Project64/Settings/UISettings.h>

template <class T>
class CSavePosDialog
{
    UISettingID m_UISettingID;
    bool m_bInitialized;

    void SaveWindowPosition()
    {
        if (!m_bInitialized)
        {
            return;
        }
        T* pT = static_cast<T*>(this);
        CRect rect;
        pT->GetWindowRect(&rect);
        UISettingsSaveString(m_UISettingID, stdstr_f("%d,%d,%d,%d", rect.left, rect.top, rect.Width(), rect.Height()).c_str());
        //MessageBox(NULL, "pos saved", "test", MB_OK);
    }

    void LoadWindowPosition()
    {
        if (!m_bInitialized)
        {
            return;
        }
        T* pT = static_cast<T*>(this);
        std::string str = UISettingsLoadStringVal(m_UISettingID);
        //MessageBox(NULL, str.c_str(), "loaded", MB_OK);
        int left, top, width, height;
        int nParams = sscanf(str.c_str(), "%d,%d,%d,%d", &left, &top, &width, &height);
        if (nParams == 4)
        {
            pT->SetWindowPos(NULL, left, top, width, height, 0);
            pT->RedrawWindow();
        }
    }

    LRESULT OnPositionChanged(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        SaveWindowPosition();
        bHandled = FALSE;
        return FALSE;
    }

    LRESULT OnShowWindow(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        LoadWindowPosition();
        bHandled = FALSE;
        return FALSE;
    }

public:
    CSavePosDialog():
        m_bInitialized(false)
    {
    }

    void DlgSavePos_Init(UISettingID uiSettingID)
    {
        m_UISettingID = uiSettingID;
        m_bInitialized = true;
    }
    
    BEGIN_MSG_MAP(CSavePosDialog)
        MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
        MESSAGE_HANDLER(WM_EXITSIZEMOVE, OnPositionChanged)
    END_MSG_MAP()
};