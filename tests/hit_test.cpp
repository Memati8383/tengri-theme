// win::HitTestEdge için başlıksız, UI'sız, yan etkisiz tablo testi.
// Derle ve çalıştır: tests\run_tests.bat  — sıfır dönüş kodu = geçti.
#include "../src/window_hit.hpp"
#include <cstdio>

static int g_failed = 0;

static void Check(const RECT& rc, LONG x, LONG y, int margin, int expect, const char* what)
{
    const POINT pt{ x, y };
    const int got = win::HitTestEdge(rc, pt, margin);
    if (got != expect)
    {
        ++g_failed;
        std::printf("FAIL  %-28s (%d,%d) m=%d -> %d, beklenen %d\n", what, x, y, margin, got, expect);
    }
    else
    {
        std::printf("ok    %-28s (%d,%d) m=%d\n", what, x, y, margin);
    }
}

int main()
{
    const RECT rc{ 100, 100, 1100, 900 };   // sol üst 100,100 / sağ alt 1100,900
    const int  m = 10;

    // köşeler: şeridin ilk pikseli
    Check(rc, 100, 100, m, HTTOPLEFT,     "sol ust");
    Check(rc, 109, 109, m, HTTOPLEFT,     "sol ust, serit ici");
    Check(rc, 1099, 100, m, HTTOPRIGHT,   "sag ust");
    Check(rc, 1099, 899, m, HTBOTTOMRIGHT,"sag alt");
    Check(rc, 100, 899, m, HTBOTTOMLEFT,  "sol alt");

    // kenarlar: köşe bandinin bir piksel dışı, iç sınırı
    Check(rc, 110, 100, m, HTTOP,         "ust kenar");
    Check(rc, 109, 110, m, HTLEFT,        "sol kenar");
    Check(rc, 1090, 500, m, HTRIGHT,      "sag kenar, ilk piksel");
    Check(rc, 500, 890, m, HTBOTTOM,      "alt kenar, ilk piksel");

    // iç: hiçbir şeride değmiyor
    Check(rc, 500, 500, m, HTCLIENT,      "merkez");
    Check(rc, 1089, 500, m, HTCLIENT,     "sag serit disina bir piksel");
    Check(rc, 500, 889, m, HTCLIENT,      "alt serit disina bir piksel");

    // pencere dışındaki noktalar en yakın kenara çözümlenir: köşe tutamayı
    // bu sayede toleranslı. Sabitlenmezse sessizce değişebilir.
    Check(rc, 50, 50, m, HTTOPLEFT,       "disari: sol ust");
    Check(rc, 2000, 500, m, HTRIGHT,      "disari: sag");

    // yapıbozum: şerit yoksa her yer istem alanı, şerit yarım pencereden
    // büyükse köşeler kazanır.
    Check(rc, 500, 500, 0, HTCLIENT,      "margin=0");
    Check(rc, 100, 100, 0, HTCLIENT,      "margin=0, kose");
    Check(rc, 600, 500, 900, HTTOPLEFT,   "margin > yarim pencere");

    std::printf(g_failed ? "\n%d test basarisiz\n" : "\ntum testler gecti\n", g_failed);
    return g_failed ? 1 : 0;
}
