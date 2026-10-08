// Tema vitrini: src\app\shell.hpp sözleşmesini dolduran beş sayfa. Pencere, kenar
// çubuğu ve üst bar kabukta yaşıyor — bu dosyada yalnızca içerik var.
// Kendi uygulamanı yazmak için buna bakabilir, src\app\my_app.cpp dosyasını
// düzenlemeye başlayabilirsin.
#include "app/shell.hpp"
#include "gui/theme.hpp"
#include "gui/fx.hpp"
#include "gui/widgets.hpp"
#include "gui/icons.hpp"
#include "gui/logo.hpp"
#include "imgui_internal.h"
#include <windows.h>
#include <cmath>
#include <cstdio>

using theme::px;

namespace demo
{
    namespace
    {
        // vitrin durumunun tamamı: hiçbir değer gerçek bir iş yapmıyor, yalnızca
        // bileşenlerin etkileşimini gösteriyor.
        bool   g_tweaks[4]   = { true, false, true, false };
        bool   g_opts[3]     = { true, true, false };
        char   g_search[64]  = {};
        char   g_key[64]     = {};
        bool   g_reveal      = false;
        float  g_speed       = 1.0f;
        float  g_gauge       = 0.62f;
        int    g_mode        = 1;
        int    g_amount      = 45;
        float  g_history[64] = {};
        float  g_progress    = 0.0f;
        bool   g_working     = false;
        bool   g_switch      = true;
        double g_workStart   = 0.0;
        const char* kModes[] = { "Dengeli", "Performans", "Sessiz" };
        const char* kPrimNames[] = { "RadialGradient", "GradientQuad", "Shine" };

        // ---- yerel ImGui bilesenleri icin durum -----------------------------
        bool   g_native_check  = true;
        int    g_native_radio  = 0;
        int    g_native_combo  = 1;
        int    g_native_list   = 2;
        bool   g_native_tree   = true;
        bool   g_native_modal  = false;
        bool   g_native_disabled = false;
        float  g_native_slider = 0.4f;
        int    g_native_row    = 0;
        char   g_native_text[64] = "yerel giriş alanı";
        const char* kComboItems[] = { "Kapalı", "Düşük", "Orta", "Yüksek" };

        void UpdateFakeData(double now)
        {
            // grafik için düz bir dalga: kaydırma değeri ilerledikçe çizgi sağdan dolar
            static float scroll = 0.0f;
            scroll += 0.008f;
            for (int i = 0; i < 64; ++i)
            {
                const float t = (float)i * 0.18f + scroll;
                g_history[i] = 0.5f + 0.34f * sinf(t) + 0.12f * sinf(t * 2.7f);
            }
            if (g_working)
            {
                g_progress = (float)((now - g_workStart) / 2.4);
                if (g_progress >= 1.0f)
                {
                    g_progress = 1.0f;
                    g_working  = false;
                    ui::Notify(ui::Toast::Success, "İşlem bitti", "Vitrin görevi tamamlandı.");
                }
            }
        }

        void DrawComponents(float w)
        {
            ui::BeginCard("##card_buttons", w, "Düğmeler", "Primary / Secondary / Ghost, simge ve yükleniyor hâli.");
            const ImVec2 bw = ImVec2((w - px(36) - px(20)) * 0.5f, px(42));
            if (ui::Button("Tara", bw, ui::ButtonStyle::Primary, Icon::Search))
                ui::Notify(ui::Toast::Info, "Tarama", "Bu bir vitrin bildirimi.");
            ImGui::SameLine(0.0f, px(20));
            if (ui::Button("Geri al", bw, ui::ButtonStyle::Secondary, Icon::Refresh))
                ui::Notify(ui::Toast::Warning, "Geri alınacak", "Onay bekleniyor.");
            ImGui::Dummy(ImVec2(0, px(8)));
            const ImVec2 bw2 = ImVec2(bw.x * 0.5f, px(38));
            if (ui::Button("İşlem başlat", bw2, ui::ButtonStyle::Ghost, Icon::Bolt))
            {
                g_working   = true;
                g_progress  = 0.0f;
                g_workStart = ImGui::GetTime();
            }
            ImGui::SameLine(0.0f, px(20));
            ui::Button("Çalışıyor", bw2, ui::ButtonStyle::Primary, Icon::None, g_working);
            ui::EndCard();

            ImGui::Dummy(ImVec2(0, px(16)));

            const float inner = w - px(36);
            ui::BeginCard("##card_checks", w, "Seçim satırları", "ToggleCard kart olarak, CheckRow düz satır çizilir.");
            const char* tweakNames[]  = { "Game Bar", "Telemetri", "Superfetch", "Hızlı Erişim"};
            const char* tweakDescs[]  = { "Arka plan kaydı kapatılır", "Olay eşleştirici devre dışı",
                                          "Ön yükleme durdurulur", "İndeks taraması kapatılır"};
            for (int i = 0; i < 4; ++i)
                ui::ToggleCard(tweakNames[i], tweakDescs[i], &g_tweaks[i], inner);
            ImGui::Dummy(ImVec2(0, px(8)));
            ui::SectionLabel("ONAYLAR");
            const char* optNames[] = { "Yedek al", "Geri sayım", "Otomatik başlat"};
            for (int i = 0; i < 3; ++i)
                ui::CheckRow(optNames[i], "Ayar sayfasından da değiştirilebilir", "HKCU", &g_opts[i], inner);
            ui::EndCard();

            ImGui::Dummy(ImVec2(0, px(16)));

            ui::BeginCard("##card_inputs", w, "Girdi ve kaydırma", "Maskeli alan, etiketli slider ve segment seçici.");
            ui::InputField("##search", "Ara", g_search, sizeof(g_search), Icon::Search, nullptr, inner);
            ImGui::Dummy(ImVec2(0, px(6)));
            ui::InputField("##key", "Anahtar", g_key, sizeof(g_key), Icon::Key, &g_reveal, inner,
                           g_reveal ? 0 : ImGuiInputTextFlags_Password);
            ImGui::Dummy(ImVec2(0, px(10)));
            ui::Slider("Parlaklık", &g_gauge, 0.0f, 1.0f, "%.0f%%", inner);
            ui::SliderInt("Düzen", &g_amount, 0, 100, inner);
            ImGui::Dummy(ImVec2(0, px(6)));
            ui::Segmented("##mode", kModes, 3, &g_mode, inner);
            ui::EndCard();
        }

        void DrawCharts(float w)
        {
            const float inner = w - px(36);

            ui::BeginCard("##card_graph", w, "Geçmiş", "Çizgi dolar, kaydırma değeriyle sağdan ilerler.");
            // Draw list BeginCard'dan SONRA alinir: kart yuzeyi ust pencerenin
            // listesinde ve iceriğin arkasinda kalir.
            ImDrawList* dl = ImGui::GetWindowDrawList();
            const ImVec2 gp = ImGui::GetCursorScreenPos();
            const ImVec2 gs = ImVec2(inner, px(118));
            ui::Graph(dl, gp, gp + gs, g_history, 64, 0.0f, 1.0f, 1.0f);
            ImGui::Dummy(gs);
            ui::EndCard();

            ImGui::Dummy(ImVec2(0, px(16)));

            ui::BeginCard("##card_ring", w, "Ölçek", "Ring yüzde, ProgressBar ısı, Spinner durumu çizer.");
            dl = ImGui::GetWindowDrawList();   // her kart kendi alt penceresinin listesini kullanir
            const ImVec2 rp = ImGui::GetCursorScreenPos();
            const float  rr = px(34);
            ImGui::Dummy(ImVec2(rr * 2.0f, rr * 2.0f));
            ui::Ring(dl, rp + ImVec2(rr, rr), rr, px(6), g_progress);
            char pct[16];
            snprintf(pct, sizeof(pct), "%%%d", (int)(g_progress * 100.0f));
            ui::Text(dl, theme::fonts.bold, theme::size::PageTitle,
                     rp + ImVec2(rr, rr) - ui::TextSize(theme::fonts.bold, theme::size::PageTitle, pct) * 0.5f,
                     theme::Gray(theme::ink::Primary), pct);
            ImGui::SameLine(0.0f, px(24));
            const ImVec2 cp = ImGui::GetCursorScreenPos();
            ui::ProgressBar("##bar", g_progress, ImVec2(inner - rr * 2.0f - px(48), px(10)));
            ImGui::SetCursorScreenPos(cp + ImVec2(0, px(28)));
            ui::Spinner(dl, ImGui::GetCursorScreenPos() + ImVec2(px(8), px(8)), px(8), px(2),
                        theme::Gray(theme::ink::Secondary));
            ImGui::SetCursorScreenPos(cp + ImVec2(px(26), px(30)));
            ui::Label(theme::fonts.regular, theme::size::Caption, theme::ink::Tertiary, "İşlem sürüyor");
            ImGui::Dummy(ImVec2(0, px(24)));
            ui::EndCard();

            ImGui::Dummy(ImVec2(0, px(16)));

            ui::BeginCard("##card_switch", w, "Switch",
                          "DrawSwitch tek başına çizilir; Key aynı öğenin hover ve değerini ayrı yumuşatır.");
            dl = ImGui::GetWindowDrawList();
            const ImVec2 sp = ImGui::GetCursorScreenPos();
            if (ImGui::InvisibleButton("##sw_live", ImVec2(px(38), px(21))))
                g_switch = !g_switch;
            const ImGuiID swId  = ImGui::GetItemID();
            const float   swHov = ui::Anim(ui::Key(swId, "hover"), ImGui::IsItemHovered() ? 1.0f : 0.0f);
            const float   swOn  = ui::Anim(ui::Key(swId, "state"), g_switch ? 1.0f : 0.0f);
            ui::DrawSwitch(dl, sp, swOn, swHov);
            ui::Text(dl, theme::fonts.regular, theme::size::Body, sp + ImVec2(px(50), px(3)),
                     theme::Gray(theme::ink::Secondary),
                     g_switch ? "Açık — anahtara tıkla" : "Kapalı — anahtara tıkla");

            // `on` float olduğu için ara değer de geçerlidir: geçiş anının kendisi.
            for (int i = 0; i < 3; ++i)
            {
                const float  t = (float)i * 0.5f;
                const ImVec2 p = sp + ImVec2(px(250) + i * (px(38) + px(34)), 0);
                ui::DrawSwitch(dl, p, t, 0.0f);
                char lab[24];
                snprintf(lab, sizeof(lab), "on = %.1f", t);
                ui::Text(dl, theme::fonts.regular, theme::size::Meta, p + ImVec2(px(2), px(26)),
                         theme::Gray(theme::ink::Tertiary), lab);
            }
            ImGui::Dummy(ImVec2(inner, px(48)));
            ui::EndCard();

            ImGui::Dummy(ImVec2(0, px(16)));

            ui::BeginCard("##card_logo", w, "Marka resmi",
                          "Aynı doku üç biçimde: Draw gerer, DrawFitted sığdırır, Tex ham çizime verir.");
            dl = ImGui::GetWindowDrawList();
            const ImVec2 bp = ImGui::GetCursorScreenPos();
            const float  bw2 = inner / 3.0f;
            const float  bh  = px(62);
            const ImU32  tint = theme::White(1.0f);
            const ImVec2 a0 = bp + ImVec2(px(6), 0),      a1 = bp + ImVec2(bw2 - px(6), bh);
            const ImVec2 b0 = bp + ImVec2(bw2 + px(6), 0), b1 = bp + ImVec2(2 * bw2 - px(6), bh);
            const ImVec2 c0 = bp + ImVec2(2 * bw2 + px(6), 0), c1 = bp + ImVec2(inner - px(6), bh);
            logo::Draw(dl, a0, a1, tint);
            logo::DrawFitted(dl, b0, b1, px(6), tint);
            if (logo::Ready())
                dl->AddImage(reinterpret_cast<ImTextureID>(logo::Tex()), c0, c1,
                             ImVec2(0, 0), ImVec2(1, 1), tint);
            const char* kLogoNames[] = { "Draw", "DrawFitted", "Tex + AddImage" };
            const ImVec2 kLogoMn[] = { a0, b0, c0 };
            for (int i = 0; i < 3; ++i)
                ui::Text(dl, theme::fonts.regular, theme::size::Meta,
                         ImVec2(kLogoMn[i].x, bp.y + bh + px(6)),
                         theme::Gray(theme::ink::Tertiary), kLogoNames[i]);
            ImGui::Dummy(ImVec2(inner, bh + px(22)));
            ui::EndCard();
        }

        void DrawEffects(float w)
        {
            const float inner = w - px(36);

            // fx'in üç primitifi: tema yüzeyleri bunlarla kuruluyor, vitrin de
            // tek tek gösteriyor.
            ui::BeginCard("##card_fx_prims", w, "Efekt primitifleri",
                          "RadialGradient, GradientQuad ve Shine — ışığın üç hâli.");
            ImDrawList* dl = ImGui::GetWindowDrawList();
            const ImVec2 op = ImGui::GetCursorScreenPos();
            const float  cw = inner / 3.0f;
            const float  ch = px(92);
            for (int i = 0; i < 3; ++i)
            {
                const ImVec2 mn = op + ImVec2(cw * i + px(6), 0);
                const ImVec2 mx = op + ImVec2(cw * (i + 1) - px(6), ch);
                const ImVec2 ct = (mn + mx) * 0.5f;
                if (i == 0)
                {
                    fx::RadialGradient(dl, ct, (mx.x - mn.x) * 0.5f, (mx.y - mn.y) * 0.5f,
                                       theme::White(0.42f), theme::White(0.0f), 32);
                }
                else if (i == 1)
                {
                    // dört köşe ayrı alpha: kart yüzeyinin yumuşak geçişi bu primitifle çizilir
                    fx::GradientQuad(dl, mn, ImVec2(mx.x, mn.y), mx, ImVec2(mn.x, mx.y),
                                     theme::White(0.34f), theme::White(0.04f),
                                     theme::White(0.16f), theme::White(0.02f));
                }
                else
                {
                    // Shine: t 0..1 arasında gezinen ışık bandı
                    const float t = 0.5f + 0.5f * sinf((float)ImGui::GetTime() * 1.1f);
                    fx::Shine(dl, mn, mx, t, theme::White(0.30f));
                }
                ui::Text(dl, theme::fonts.regular, theme::size::Meta,
                         ImVec2(mn.x, mx.y + px(6)), theme::Gray(theme::ink::Tertiary),
                         kPrimNames[i]);
            }
            ImGui::Dummy(ImVec2(inner, ch + px(22)));
            ui::EndCard();

            ImGui::Dummy(ImVec2(0, px(16)));

            ui::BeginCard("##card_fx", w, "Arka plan efektleri", "Efekt ayarları tek Settings yapısında toplanır.");
            ui::ToggleCard("Parçacıklar", "Düşen ışık noktaları", &fx::settings.particles, inner, false);
            ui::ToggleCard("Yıldız çizgileri", "Geçen iz çizgileri", &fx::settings.lines, inner, false);
            ui::ToggleCard("Işıma", "Kart üstü parlama", &fx::settings.glow, inner, false);
            ui::ToggleCard("Süpürme", "Periyodik ışık süpürmesi", &fx::settings.sweep, inner, false);
            ui::ToggleCard("Fare takibi", "Işık imleci izler", &fx::settings.mouse, inner, false);
            ui::ToggleCard("Bildirimler açık", "Kapatılsa da hata bildirimi geçer",
                           &ui::notificationsEnabled, inner, false);
            ImGui::Dummy(ImVec2(0, px(8)));
            ui::SliderInt("Parçacık sayısı", &fx::settings.count, 0, 200, inner);
            ui::Slider("Hız", &fx::settings.speed, 0.2f, 3.0f, "x%.1f", inner);
            ui::EndCard();

            ImGui::Dummy(ImVec2(0, px(16)));

            ui::BeginCard("##card_notify", w, "Bildirimler", "Toaster ekranın sağ altından yayılır.");
            const ImVec2 bw = ImVec2((inner - px(20)) * 0.5f, px(38));
            if (ui::Button("Başarılı", bw, ui::ButtonStyle::Primary, Icon::Check))
                ui::Notify(ui::Toast::Success, "Kayıt tamam", "İşlem yedeklenerek uygulandı.");
            ImGui::SameLine(0.0f, px(20));
            if (ui::Button("Bilgi", bw, ui::ButtonStyle::Secondary, Icon::Info))
                ui::Notify(ui::Toast::Info, "Sürüm güncel", "Başka bir kayıt aranmadı.");
            ImGui::Dummy(ImVec2(0, px(8)));
            if (ui::Button("Uyarı", bw, ui::ButtonStyle::Secondary, Icon::Warning))
                ui::Notify(ui::Toast::Warning, "Yetki gerekli", "Yazma işlemi yüksek yetki ister.");
            ImGui::SameLine(0.0f, px(20));
            if (ui::Button("Hata", bw, ui::ButtonStyle::Ghost, Icon::Error))
                ui::Notify(ui::Toast::Error, "Başarısız", "Kayıt defteri anahtarı açılamadı.");
            ui::EndCard();
        }

        // ---- tipografi ------------------------------------------------------

        void DrawTypeScale(float w)
        {
            const auto& F  = theme::fonts;
            const float inner = w - px(36);

            ui::BeginCard("##card_scale", w, "Punto ölçeği",
                          "Çağrı yerleri sayı yazmaz, rol yazar: theme::size::*");
            // Kart yuzeyi ust pencerenin listesinde, icerigin arkasinda cizilir;
            // bu yüzden dl her karttan SONRA alinir ve içerik alt pencereye yazar.
            ImDrawList* dl = ImGui::GetWindowDrawList();
            struct Row { const char* role; float size; ImFont* font; const char* sample; };
            const Row rows[] = {
                { "Micro",     theme::size::Micro,     F.medium,  "GÖZ ÜSTÜ ETİKET" },
                { "Meta",      theme::size::Meta,      F.regular, "alt başlık, zaman damgası" },
                { "Caption",   theme::size::Caption,   F.regular, "ikincil açıklama, devre dışı" },
                { "Body",      theme::size::Body,      F.regular, "gövde metni, liste satırı" },
                { "Label",     theme::size::Label,     F.medium,  "düğme, toggle, segment etiketi" },
                { "Title",     theme::size::Title,     F.bold,    "kart başlığı, kenar çubuğu markası" },
                { "Heading",   theme::size::Heading,   F.bold,    "bölüm başlığı" },
                { "PageTitle", theme::size::PageTitle, F.bold,    "sekme başlığı, büyük değer" },
                { "Display",   theme::size::Display,   F.bold,    "giriş ekranı" },
            };
            for (const Row& r : rows)
            {
                const float h = theme::px(r.size) * 1.7f;
                const ImVec2 p = ImGui::GetCursorScreenPos();
                ui::Text(dl, F.medium, theme::size::Meta,
                         ImVec2(p.x, p.y + (h - theme::px(theme::size::Meta)) * 0.5f),
                         theme::Gray(theme::ink::Tertiary), r.role);
                ui::Text(dl, r.font, r.size, ImVec2(p.x + px(92), p.y),
                         theme::Gray(theme::ink::Primary), r.sample);
                ImGui::Dummy(ImVec2(inner, h));
            }
            ui::EndCard();

            ImGui::Dummy(ImVec2(0, px(16)));

            ui::BeginCard("##card_ink", w, "Mürekkep seviyeleri",
                          "Tüm hiyerarşi parlaklık + alfa ile kurulur; renk tonu yok.");
            dl = ImGui::GetWindowDrawList();
            struct Ink { const char* role; float v; const char* note; };
            const Ink inks[] = {
                { "Primary",   theme::ink::Primary,   "başlık, değer, vurgu" },
                { "Secondary", theme::ink::Secondary, "gövde, açıklama" },
                { "Tertiary",  theme::ink::Tertiary,  "meta; 18pt altı 4.5:1'i karşılar" },
                { "Disabled",  theme::ink::Disabled,  "placeholder, kapalı durum" },
            };
            for (const Ink& k : inks)
            {
                const ImVec2 p = ImGui::GetCursorScreenPos();
                const float  h = px(24);
                dl->AddRectFilled(p, p + ImVec2(px(8), h), theme::Gray(k.v), px(3));
                ui::Text(dl, F.medium, theme::size::Meta, ImVec2(p.x + px(20), p.y + px(4)),
                         theme::Gray(k.v), k.role);
                ui::Text(dl, F.regular, theme::size::Caption, ImVec2(p.x + px(92), p.y + px(5)),
                         theme::Gray(theme::ink::Tertiary), k.note);
                ImGui::Dummy(ImVec2(inner, h));
            }
            ImGui::Dummy(ImVec2(0, px(6)));
            ui::SectionLabel("HARF ARALIĞI");
            ui::TextSpaced(dl, F.bold, theme::size::Title, ImGui::GetCursorScreenPos(),
                           theme::Gray(theme::ink::Primary), "TENGRİ", px(theme::track::Wide));
            ImGui::Dummy(ImVec2(0, px(26)));
            ui::TextSpaced(dl, F.medium, theme::size::Micro, ImGui::GetCursorScreenPos(),
                           theme::Gray(theme::ink::Tertiary), "AÇIK KAYNAKLI ARAYÜZ TEMASI",
                           px(theme::track::Micro));
            ImGui::Dummy(ImVec2(0, px(24)));
            ui::EndCard();

            ImGui::Dummy(ImVec2(0, px(16)));

            ui::BeginCard("##card_glyph", w, "Glif kapsamı",
                          "Alt küme üç yüze de Türkçe bloğuyla verildi; aralık olmasa kutu çizilir.");
            dl = ImGui::GetWindowDrawList();
            struct Face { const char* role; ImFont* font; float size; float gray; const char* text; };
            const Face faces[] = {
                { "bold",    F.bold,    theme::size::Heading,  theme::ink::Primary,
                  "Aa Bb Cc Çç Dd Ee Ff Gg Ğğ Hh Ii İı Jj Kk Ll Mm Nn Oo Öö" },
                { "medium",  F.medium,  theme::size::Body,     theme::ink::Secondary,
                  "Pp Rr Ss Şş Tt Uu Üü Vv Yy Zz — 0123456789 ()[]{}<>/-*+=.,;:!?%&#$@" },
                { "regular", F.regular, theme::size::Caption,  theme::ink::Tertiary,
                  "fontdata::kRanges — U+0020..007E, U+00A0..00FF, U+0100..017F (Türkçe), U+2000..206F" },
            };
            for (const Face& f : faces)
            {
                const ImVec2 p = ImGui::GetCursorScreenPos();
                const float  h = theme::px(f.size) * 1.7f;
                ui::Text(dl, F.medium, theme::size::Meta,
                         ImVec2(p.x, p.y + (h - theme::px(theme::size::Meta)) * 0.5f),
                         theme::Gray(theme::ink::Disabled), f.role);
                ui::Text(dl, f.font, f.size, ImVec2(p.x + px(64), p.y), theme::Gray(f.gray), f.text);
                ImGui::Dummy(ImVec2(inner, h));
            }
            ui::EndCard();
        }

        void DrawIconSheet(float w)
        {
            const auto& F  = theme::fonts;
            const float inner = w - px(36);

            // Icon enum'inda None'dan sonraki sirayla ayni olmali.
            static const char* kNames[] = {
                "Dashboard", "Cleaner", "Tweaks", "Network", "Settings", "Key",
                "User", "Logout", "Close", "Minimize", "Check", "Eye", "EyeOff",
                "Cpu", "Memory", "Disk", "Shield", "Bolt", "Info", "Warning",
                "Error", "Alert", "Search", "Refresh", "Monitor", "Globe", "Lock",
                "Chip", "Instagram", "GitHub", "Code",
            };
            constexpr int n     = (int)(sizeof(kNames) / sizeof(kNames[0]));
            constexpr int cols  = 6;
            constexpr int rows  = (n + cols - 1) / cols;
            const ImVec2  cell  = ImVec2(inner / (float)cols, px(72));

            ui::BeginCard("##card_icons", w, "İkon listesi",
                          "Hepsi ImDrawList ile çizilir; ikon fontu ve dosya gerektirmez.");
            ImDrawList* dl = ImGui::GetWindowDrawList();
            const ImVec2 o = ImGui::GetCursorScreenPos();
            for (int i = 0; i < n; ++i)
            {
                const ImVec2 c = o + ImVec2(((i % cols) + 0.5f) * cell.x,
                                            ((i / cols) + 0.34f) * cell.y);
                icons::Draw(dl, (Icon)(i + 1), c, px(22), theme::Gray(theme::ink::Secondary), px(1.6f));
                const ImVec2 ts = ui::TextSize(F.medium, theme::size::Micro, kNames[i]);
                ui::Text(dl, F.medium, theme::size::Micro, ImVec2(c.x - ts.x * 0.5f, c.y + px(20)),
                         theme::Gray(theme::ink::Tertiary), kNames[i]);
            }
            ImGui::Dummy(ImVec2(inner, cell.y * (float)rows));
            ui::EndCard();
        }

        // ---- yerel ImGui bilesenleri ----------------------------------------

        void DrawNative(float w)
        {
            const float  inner = w - px(36);
            const auto&  F     = theme::fonts;

            ui::BeginCard("##card_native_basic", w, "Yerel bileşenler",
                          "ApplyStyle() ImGui'in kendi widgetlarını da stiller: rounding, alfa, kaydırma çubuğu.");
            ImGui::Checkbox("Kutu işareti", &g_native_check);
            ImGui::RadioButton("Birinci", &g_native_radio, 0);
            ImGui::SameLine();
            ImGui::RadioButton("İkinci", &g_native_radio, 1);
            ImGui::SameLine();
            ImGui::RadioButton("Üçüncü", &g_native_radio, 2);
            ImGui::Combo("Seviye", &g_native_combo, kComboItems, IM_ARRAYSIZE(kComboItems));
            ImGui::SameLine();
            ImGui::InputText("##native_input", g_native_text, IM_ARRAYSIZE(g_native_text));
            ImGui::SliderFloat("Değer", &g_native_slider, 0.0f, 1.0f, "%.2f");
            ImGui::ProgressBar(g_native_slider, ImVec2(inner, 0.0f));
            if (ImGui::TreeNode("Ağaç düğümü"))
            {
                ImGui::BulletText("alt düğüm");
                ImGui::BulletText("bir diğer alt düğüm");
                ImGui::TreePop();
            }
            ImGui::ListBox("Liste", &g_native_list, kComboItems, IM_ARRAYSIZE(kComboItems), 3);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("İpucu: PopupBg, PopupRounding ve Border jetonlarını çizer.");
            ui::EndCard();

            ImGui::Dummy(ImVec2(0, px(16)));

            ui::BeginCard("##card_native_state", w, "Durumlar",
                          "Devre dışı, açılır menü, modal ve yerel sekme çubuğu.");
            if (ui::ToggleCard("Devre dışı grup", "Alpha DisabledAlpha'ya düşer", &g_native_disabled, inner, false))
            {}
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha,
                                g_native_disabled ? ImGui::GetStyle().DisabledAlpha : 1.0f);
            const ImVec2 bw = ImVec2((inner - px(20)) * 0.5f, px(38));
            ui::Button("Kilitli işlem", bw, ui::ButtonStyle::Secondary, Icon::Lock);
            ImGui::SameLine(0.0f, px(20));
            if (ui::Button("Açılır menü", bw, ui::ButtonStyle::Ghost, Icon::Settings))
                ImGui::OpenPopup("##demo_menu");
            ImGui::PopStyleVar();

            // Kök düzen için WindowPadding (0,0); yerel popup kendi boşluğunu verir.
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(px(12), px(10)));
            if (ImGui::BeginPopup("##demo_menu"))
            {
                for (int i = 0; i < 3; ++i)
                    if (ImGui::Selectable(kComboItems[i + 1])) g_native_combo = i + 1;
                ImGui::Separator();
                ImGui::TextDisabled("açılır menü satırı");
                ImGui::EndPopup();
            }
            ImGui::PopStyleVar();

            ImGui::Dummy(ImVec2(0, px(6)));
            if (ui::Button("Modal aç", bw, ui::ButtonStyle::Primary, Icon::Alert))
                g_native_modal = true;
            ImGui::SameLine(0.0f, px(20));
            ImGui::AlignTextToFramePadding();
            ui::Label(F.regular, theme::size::Caption, theme::ink::Tertiary,
                      "ModalWindowDimBg ve PopupBg birlikte görünür");

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(px(22), px(20)));
            if (g_native_modal) ImGui::OpenPopup("##demo_modal");
            if (ImGui::BeginPopupModal("##demo_modal", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                // AlwaysAutoResize: genişliği içerik belirler, kart genişliği değil.
                const ImVec2 mb = ImVec2(px(150), px(36));
                ui::Label(F.bold, theme::size::Title, theme::ink::Primary, "Onay gerekiyor");
                ImGui::Dummy(ImVec2(0, px(2)));
                ui::Label(F.regular, theme::size::Body, theme::ink::Secondary,
                          "Bu bir modal pencere; arka plan karartıldı.");
                ImGui::Dummy(ImVec2(0, px(10)));
                if (ui::Button("Vazgeç", mb, ui::ButtonStyle::Secondary))
                {
                    g_native_modal = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine(0.0f, px(20));
                if (ui::Button("Onayla", mb, ui::ButtonStyle::Primary))
                {
                    g_native_modal = false;
                    ui::Notify(ui::Toast::Success, "Onaylandı", "Modal kapandı.");
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
            ImGui::PopStyleVar();

            ImGui::Dummy(ImVec2(0, px(10)));
            if (ImGui::BeginTabBar("##native_tabs"))
            {
                if (ImGui::BeginTabItem("Sekme A"))
                {
                    ui::Label(F.regular, theme::size::Body, theme::ink::Secondary, "Header jetonları");
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Sekme B"))
                {
                    ui::Label(F.regular, theme::size::Body, theme::ink::Secondary, "Sekme geçişi");
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }

            if (ImGui::BeginTable("##native_table", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn("Kayıt");
                ImGui::TableSetupColumn("Değer");
                ImGui::TableSetupColumn("Durum");
                ImGui::TableHeadersRow();
                for (int r = 0; r < 3; ++r)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::Text("satır %d", r + 1);
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%d", (r + 1) * 128);
                    ImGui::TableSetColumnIndex(2); ImGui::TextColored(ImVec4(1, 1, 1, 0.6f), "hazır");
                }
                ImGui::EndTable();
            }
            ui::EndCard();
        }

        void DrawTypography(float w)
        {
            DrawTypeScale(w);
            ImGui::Dummy(ImVec2(0, px(16)));
            DrawIconSheet(w);
        }
    }

    // ---- kabuğun okuduğu kayıt ----------------------------------------------
    const app::Page kPages[] = {
        { "Bileşenler", Icon::Dashboard, DrawComponents },
        { "Grafikler",  Icon::Chip,      DrawCharts     },
        { "Efektler",   Icon::Bolt,      DrawEffects    },
        { "Tipografi",  Icon::Tweaks,    DrawTypography },
        { "ImGui",      Icon::Code,      DrawNative     },
    };
    constexpr int kPageCount = (int)(sizeof(kPages) / sizeof(kPages[0]));
    constexpr int kNativePage = 4;

    const char* const kStatus[] = { "Kenarlıksız pencere", "DPI ölçekli çizim" };

    void Begin(const app::Options& opt)
    {
        // modal yalnızca yerel bileşen sayfasında yaşıyor; "-modal" tek başına
        // verildiğinde o sayfaya geçilir.
        if (opt.modal)
        {
            g_native_modal = true;
            if (opt.start_page < 0) app::SetPage(kNativePage);
        }
        for (int i = 0; i < 64; ++i) g_history[i] = 0.5f;
    }

    void Tick(double now) { UpdateFakeData(now); }
}

namespace app
{
    const Content& content()
    {
        static const Content c = []
        {
            Content x;
            x.brand        = "TENGRİ THEME";
            x.caption      = "tema önizlemesi";
            x.pages        = demo::kPages;
            x.page_count   = demo::kPageCount;
            x.nav_label    = "VİTRİN";
            x.status_lines = demo::kStatus;
            x.status_count = 2;
            x.status_size  = true;   // son satır: canlı pencere ölçüsü
            x.begin        = demo::Begin;
            x.tick         = demo::Tick;
            return x;
        }();
        return c;
    }
}
