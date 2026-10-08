#pragma once
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui.h"
#include "../gui/icons.hpp"
#include <windows.h>

// Uygulama kabuğu: kenarlıksız pencerenin içindeki kalıp arayüzü (üst bar, kenar
// çubuğu, kayan sekme göstergesi, kaydırılabilir içerik, bildirimler, pencere
// düğmeleri ve sürükleyerek taşıma) bir kez burada yaşar. Bir uygulamanın yapması
// gereken tek şey aşağıdaki Content sözleşmesini doldurmak.
namespace app
{
    // Bir kenar çubuğu satırı = bir sayfa. draw(), ##content alt penceresinin
    // içinde çağrılır; width o alt pencerenin genişliğidir (kartlar bu genişliği
    // ui::BeginCard(id, width, ...) ile olduğu gibi kullanır).
    struct Page
    {
        const char* label = "";
        Icon        icon  = Icon::None;
        void      (*draw)(float width) = nullptr;
    };

    // Komut satından gelen başlangıç durumları. Üçü de tıklama gerektirmeden
    // belirli bir hali ekrana getirir (ekran görüntüsü alma ve doküman için).
    struct Options
    {
        int  start_page = -1;    // -1: Content::first_page
        int  toast      = -1;    // ui::Toast sırası (0 Başarılı, 1 Bilgi, 2 Uyarı, 3 Hata)
        bool modal      = false; // içeriğin kendi modalı; bayrağı okumak içeriğe kalmış
    };

    // Kabuğun bildiği tek şey bu yapı. Alanların hepsinin bir varsayılanı var,
    // böylece şablon yalnızca ihtiyacı olan satırları doldurur.
    struct Content
    {
        const char* brand   = "TENGRİ"; // üst bardaki marka metni
        const char* caption = nullptr;  // üst bardaki ikincil etiket

        const Page* pages      = nullptr;
        int         page_count = 0;
        int         first_page = 0;

        const char* nav_label  = "GEZİNME";   // kenar çubuğu bölüm etiketi
        // Durum kartı satırları (en fazla 3; kart sabit yükseklikte). nullptr ile
        // status_count = 0 olursa kart hiç çizilmez.
        const char* const* status_lines = nullptr;
        int          status_count       = 0;
        bool         status_size        = false; // son satır olarak canlı "W x H px"

        // İlk kareden hemen önce, bir kez: başlangıç sekmesini seçmek, bildirim
        // sıraya almak için. ImGui bağlamı açıktır, pencere henüz çizilmemiştir.
        void (*begin)(const Options& opt) = nullptr;
        // Her karede, sayfa çizilmeden önce: kendi animasyon/veri güncellemeleri.
        void (*tick)(double now) = nullptr;
    };

    // main.cpp bu serbest fonksiyonu linklenen içerik modülünden alır
    // (src\app\my_app.cpp ya da referans vitrin için src\demo.cpp).
    const Content& content();

    void Init(HWND hwnd, float corner_radius, const Content& c, const Options& opt);
    void Frame();

    // Sayfa fonksiyonlarının kabukla konuşabildiği tek iki çağrı.
    int   CurrentPage();
    void  SetPage(int index);
    HWND  Hwnd();
    float Corner();
}
