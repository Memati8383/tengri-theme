// ============================ KENDİ UYGULAMAN ==============================
// Bu dosya başlangıç şablonudur: TengriApp.exe buradan doğar. Tema çekirdeğine
// (src\gui\*), kabuğa (src\app\shell.*) ve src\main.cpp'ye dokunmana gerek yok —
// uygulamanın tamamı bu dosyada ve aşağıdaki üç yerde yaşar:
//
//   1) kPages tablosu        : kenar çubuğundaki satırlar ve hangi fonksiyonun
//                              hangi sayfayı çizdiği
//   2) Draw* fonksiyonları    : sayfa başına bir fonksiyon; kartları buraya yazarsın
//   3) content() doldur      : marka metni, durum kartı, ilk kare (Begin) ve her
//                              kare (Tick) çağrıları
//
// Bileşenlerin tam listesi için referans vitrine bak: src\demo.cpp
// ===========================================================================
#include "shell.hpp"
#include "../gui/theme.hpp"
#include "../gui/fx.hpp"
#include "../gui/widgets.hpp"
#include "../gui/icons.hpp"
#include <cmath>
#include <cstdio>

using theme::px;

namespace my
{
    namespace
    {
        // ---- uygulama durumu ------------------------------------------------
        // Şablonda hepsi yerel değişken: gerçek uygulamada burayı bir yapıya
        // toplayıp kayıt defteri/dosya yazma adımını ekle.
        char   g_name[64]   = "";
        float  g_workload   = 35.0f;   // 0..100: slider ve ölçü aynı birimi kullansın
        int    g_profile    = 1;
        bool   g_autostart  = true;
        bool   g_telemetry  = false;
        bool   g_confirm    = false;
        float  g_history[64];
        const char* kProfiles[] = { "Hızlı", "Dengeli", "Derin" };

        void InitHistory()
        {
            for (int i = 0; i < 64; ++i)
                g_history[i] = 0.5f + 0.28f * sinf(i * 0.24f);
        }

        // ---- 1. sayfa: Panel -------------------------------------------------
        void DrawPanel(float w)
        {
            const float inner = w - px(36);

            ui::BeginCard("##card_run", w, "Durum", "Bir değer, bir ölçü ve iki eylem.");
            // Kart yüzü üst pencerenin listesinde, içeriğin arkasında çizildiği için
            // draw list her BeginCard'dan SONRA alınır.
            ImDrawList* dl = ImGui::GetWindowDrawList();
            const ImVec2 rp = ImGui::GetCursorScreenPos();
            const float  rr = px(30);
            ImGui::Dummy(ImVec2(rr * 2.0f, rr * 2.0f));
            ui::Ring(dl, rp + ImVec2(rr, rr), rr, px(6), g_workload * 0.01f);
            char pct[12];
            snprintf(pct, sizeof(pct), "%%%d", (int)g_workload);
            ui::Text(dl, theme::fonts.bold, theme::size::PageTitle,
                     rp + ImVec2(rr, rr) - ui::TextSize(theme::fonts.bold, theme::size::PageTitle, pct) * 0.5f,
                     theme::Gray(theme::ink::Primary), pct);
            ImGui::SameLine(0.0f, px(24));
            ui::Slider("İş yükü", &g_workload, 0.0f, 100.0f, "%.0f%%", inner - rr * 2.0f - px(48));
            ImGui::Dummy(ImVec2(0, px(10)));
            const ImVec2 bw = ImVec2((inner - px(20)) * 0.5f, px(40));
            if (ui::Button("Çalıştır", bw, ui::ButtonStyle::Primary, Icon::Bolt))
                ui::Notify(ui::Toast::Success, "Çalıştırıldı", kProfiles[g_profile]);
            ImGui::SameLine(0.0f, px(20));
            if (ui::Button("Kaydet", bw, ui::ButtonStyle::Secondary, Icon::Check))
                ui::Notify(ui::Toast::Info, "Kaydedildi", "Ayarlar bu oturumda tutuluyor.");
            ui::EndCard();

            ImGui::Dummy(ImVec2(0, px(16)));

            ui::BeginCard("##card_history", w, "Geçmiş", "ui::Graph tek tek değer değil, dizi çizer.");
            dl = ImGui::GetWindowDrawList();
            const ImVec2 gp = ImGui::GetCursorScreenPos();
            const ImVec2 gs = ImVec2(inner, px(120));
            ui::Graph(dl, gp, gp + gs, g_history, 64, 0.0f, 1.0f, 1.0f);
            ImGui::Dummy(gs);
            ui::EndCard();

            ImGui::Dummy(ImVec2(0, px(16)));

            ui::BeginCard("##card_task", w, "Görev", "Etiketli giriş ve iki seçim satırı.");
            ui::InputField("##name", "Görev adı", g_name, sizeof(g_name), Icon::Search, nullptr, inner);
            ImGui::Dummy(ImVec2(0, px(8)));
            ui::ToggleCard("Otomatik başlat", "Oturum açılınca çalışır", &g_autostart, inner);
            ui::ToggleCard("Telemetri", "Kapalı kalması önerilir", &g_telemetry, inner);
            ui::EndCard();
        }

        // ---- 2. sayfa: Ayarlar -----------------------------------------------
        void DrawSettings(float w)
        {
            const float inner = w - px(36);

            ui::BeginCard("##card_fx", w, "Görünüm", "Efekt ayarları doğrudan fx::settings üzerinden yazılır.");
            ui::ToggleCard("Parçacıklar", "Düşen ışık noktaları", &fx::settings.particles, inner, false);
            ui::ToggleCard("Işıma", "Kart üstü parlama", &fx::settings.glow, inner, false);
            ui::ToggleCard("Süpürme", "Periyodik ışık süpürmesi", &fx::settings.sweep, inner, false);
            ImGui::Dummy(ImVec2(0, px(6)));
            ui::Slider("Hız", &fx::settings.speed, 0.2f, 3.0f, "x%.1f", inner);
            ui::Segmented("##profile", kProfiles, 3, &g_profile, inner);
            ui::EndCard();

            ImGui::Dummy(ImVec2(0, px(16)));

            ui::BeginCard("##card_reset", w, "Sıfırla", "Onay modalı: ui::Button + ImGui::BeginPopupModal.");
            const ImVec2 bw = ImVec2(inner * 0.5f - px(10), px(38));
            if (ui::Button("Tüm ayarları sıfırla", bw * 2.0f, ui::ButtonStyle::Ghost, Icon::Refresh))
                g_confirm = true;
            ImGui::Dummy(ImVec2(0, px(8)));
            ui::CheckRow("Bildirimler", "ui::Notify çıktı üretsin", "toaster",
                         &ui::notificationsEnabled, inner);

            // Tema kök düzen için WindowPadding'i (0,0) bırakıyor; kendi yerel
            // ImGui pencereni/popupunu açtığında boşluğu sen veriyorsun, yoksa
            // metin kartın kenarına yapışır.
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(px(22), px(20)));
            if (g_confirm) ImGui::OpenPopup("##reset_modal");
            if (ImGui::BeginPopupModal("##reset_modal", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                // AlwaysAutoResize: modalın genişliğini içeriği belirler. Kart
                // genişliğini (inner) buraya taşirsan modal ekrani kaplar.
                const ImVec2 mb = ImVec2(px(150), px(38));
                ui::Label(theme::fonts.bold, theme::size::Title, theme::ink::Primary, "Emin misin?");
                ImGui::Dummy(ImVec2(0, px(2)));
                ui::Label(theme::fonts.regular, theme::size::Body, theme::ink::Secondary,
                          "Bu şablonda durum yalnızca bellekte.");
                ImGui::Dummy(ImVec2(0, px(10)));
                if (ui::Button("Vazgeç", mb, ui::ButtonStyle::Secondary))
                {
                    g_confirm = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine(0.0f, px(20));
                if (ui::Button("Sıfırla", mb, ui::ButtonStyle::Primary, Icon::Alert))
                {
                    g_confirm   = false;
                    g_workload  = 35.0f;
                    g_profile   = 1;
                    g_autostart = true;
                    ui::Notify(ui::Toast::Warning, "Sıfırlandı", "Varsayılan değerler geri geldi.");
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
            ImGui::PopStyleVar();
            ui::EndCard();
        }

        // ---- 3. sayfa: Hakkında ----------------------------------------------
        void DrawAbout(float w)
        {
            const float inner = w - px(36);
            ui::BeginCard("##card_about", w, "Bu şablon", "Üç dosya: kabuk, tema, içerik.");
            ui::TextSpaced(ImGui::GetWindowDrawList(), theme::fonts.bold, theme::size::Heading,
                           ImGui::GetCursorScreenPos(), theme::Gray(theme::ink::Primary),
                           "TENGRİ", px(theme::track::Wide));
            ImGui::Dummy(ImVec2(0, px(26)));
            ui::Label(theme::fonts.regular, theme::size::Body, theme::ink::Secondary,
                      "src\\app\\shell.cpp  —  pencere, kenar çubuğu, üst bar");
            ui::Label(theme::fonts.regular, theme::size::Body, theme::ink::Secondary,
                      "src\\gui\\*  —  tema jetonları ve bileşenler");
            ui::Label(theme::fonts.regular, theme::size::Body, theme::ink::Secondary,
                      "src\\app\\my_app.cpp  —  senin uygulaman");
            ImGui::Dummy(ImVec2(0, px(10)));
            ui::SectionLabel("İPUCU");
            ui::Label(theme::fonts.regular, theme::size::Caption, theme::ink::Tertiary,
                      "Yeni sayfa: kPages'e bir satır ekle, fonksiyonu yaz, bitti.");
            ImGui::Dummy(ImVec2(0, px(8)));
            ui::ProgressBar("##ratio", g_workload * 0.01f, ImVec2(inner, px(10)));
            ui::EndCard();
        }
    }

    // ---- kenar çubuğu = sayfa tablosu ---------------------------------------
    const app::Page kPages[] = {
        { "Panel",    Icon::Dashboard, DrawPanel    },
        { "Ayarlar",  Icon::Settings,  DrawSettings },
        { "Hakkında", Icon::Info,      DrawAbout    },
    };
    constexpr int kPageCount = (int)(sizeof(kPages) / sizeof(kPages[0]));

    const char* const kStatus[] = { "Başlangıç şablonu", "Kenarlıksız pencere" };

    void Begin(const app::Options& opt)
    {
        InitHistory();
        if (opt.modal)
        {
            g_confirm = true;
            // Modal Ayarlar sayfasında çiziliyor; popup'u açan sayfa görünmezse
            // ImGui onu göstermez, o yüzden sayfa da oraya taşınıyor.
            if (opt.start_page < 0) app::SetPage(1);
        }
    }

    void Tick(double now)
    {
        // Geçmiş kare hızıyla değil, sabit aralıkla örneklenir: 60 fps'te her kare
        // kaydırırsan 64 nokta bir saniyede tükenir ve çizgi düzleşir. Gerçek
        // uygulamada burada bir ölçüm ya da iş kuyruğu okunur.
        static double s_last = 0.0;
        if (now - s_last < 0.12) return;
        s_last = now;
        for (int i = 0; i < 63; ++i) g_history[i] = g_history[i + 1];
        g_history[63] = 0.5f + 0.3f * sinf((float)now * 1.4f) * (0.4f + g_workload * 0.01f);
    }
}

namespace app
{
    // main.cpp bu fonksiyonu linklenen modülden alır: şablon için bu dosya,
    // vitrin için src\demo.cpp.
    const Content& content()
    {
        static const Content c = []
        {
            Content x;
            x.brand        = "TENGRİ APP";
            x.caption      = "başlangıç şablonu";
            x.pages        = my::kPages;
            x.page_count   = my::kPageCount;
            x.nav_label    = "UYGULAMA";
            x.status_lines = my::kStatus;
            x.status_count = 2;
            x.status_size  = true;
            x.begin        = my::Begin;
            x.tick         = my::Tick;
            return x;
        }();
        return c;
    }
}
