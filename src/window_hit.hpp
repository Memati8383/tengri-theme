// Kenarlıksız pencerede yeniden boyutlandırma şeritlerinin hesabı. Win32'nin
// kendisine bırakılmayıp burada saf bir fonksiyona çıkarılmasının tek nedeni
// test edilebilir olması: WM_NCHITTEST'i fareyle sürükleyerek doğrulamak
// mümkün değil, bu tabloyu doğrulamak mümkün.
#pragma once
#include <windows.h>

namespace win
{
    inline int HitTestEdge(const RECT& rc, const POINT& pt, int margin)
    {
        if (margin <= 0) return HTCLIENT;

        const bool L = pt.x <  rc.left  + margin;
        const bool R = pt.x >= rc.right  - margin;
        const bool T = pt.y <  rc.top    + margin;
        const bool B = pt.y >= rc.bottom - margin;

        if (T && L) return HTTOPLEFT;
        if (T && R) return HTTOPRIGHT;
        if (B && L) return HTBOTTOMLEFT;
        if (B && R) return HTBOTTOMRIGHT;
        if (L) return HTLEFT;
        if (R) return HTRIGHT;
        if (T) return HTTOP;
        if (B) return HTBOTTOM;
        return HTCLIENT;
    }
}
