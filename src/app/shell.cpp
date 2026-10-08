#include "shell.hpp"
#include "../gui/theme.hpp"
#include "../gui/fx.hpp"
#include "../gui/widgets.hpp"
#include "../gui/icons.hpp"
#include "../gui/logo.hpp"
#include "imgui_internal.h"
#include <cstdio>

using theme::px;

namespace app
{
    namespace
    {
        HWND        g_hwnd   = nullptr;
        float       g_corner = 0.0f;
        const Content* g_c   = nullptr;
        int         g_page   = 0;
        Options     g_opt    = {};
        bool        g_first_frame = true;
        bool        g_dragging = false;
        POINT       g_dragStart = {};
        RECT        g_dragRect  = {};

        void DrawTopBar(const ImVec2& ds, ImDrawList* dl)
        {
            const auto& F = theme::fonts;
            if (logo::Ready())
                logo::Draw(dl, ImVec2(px(22), px(20)), ImVec2(px(22) + px(22), px(20) + px(22)), theme::Gray(0.92f));
            else
                icons::Draw(dl, Icon::Bolt, ImVec2(px(33), px(31)), px(20), theme::Gray(0.92f));

            const char* name = g_c->brand;
            ui::TextSpaced(dl, F.bold, theme::size::Title, ImVec2(px(54), px(24)),
                           theme::Gray(theme::ink::Primary), name, px(theme::track::Micro));
            if (g_c->caption)
            {
                const float nw = ui::SpacedSize(F.bold, theme::size::Title, name, px(theme::track::Micro)).x;
                ui::Text(dl, F.regular, theme::size::Meta, ImVec2(px(54) + nw + px(12), px(27)),
                         theme::Gray(theme::ink::Tertiary), g_c->caption);
            }

            // pencere düğmeleri
            const ImVec2 bs = px(34, 28);
            const float  y  = px(12);
            ImGui::SetCursorScreenPos(ImVec2(ds.x - px(12) - bs.x * 2.0f - px(4), y));
            if (ui::IconButton("##minimize", Icon::Minimize, bs, px(12)))
                ::PostMessageW(g_hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
            ImGui::SetCursorScreenPos(ImVec2(ds.x - px(12) - bs.x, y));
            if (ui::IconButton("##close", Icon::Close, bs, px(11), true))
                ::PostMessageW(g_hwnd, WM_CLOSE, 0, 0);
        }

        // Akışa bırakılmış hiçbir öğe yok: kök pencerede WindowPadding ve Indent 0
        // olduğu için ImGui her satır sonunda X'i pencerenin soluna (0) geri çekiyor.
        // Bu yüzden burada her öğe açıkça konumlandırılıyor.
        void DrawSidebar(const ImVec2& pos, float width, float height)
        {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ui::Card(dl, pos, pos + ImVec2(width, height), px(16));

            ImGui::SetCursorScreenPos(pos + px(18, 18));
            ui::SectionLabel(g_c->nav_label);

            const float tabH = px(40), gap = px(6);
            const float tx = pos.x + px(12), tw = width - px(24);
            const float ty0 = pos.y + px(52);

            // seçili sekmenin arkasında kayan gösterge
            const float target = ty0 + g_page * (tabH + gap);
            // İlk karede AnimSet: -page 3 ile açılınca gösterge süzülerek gelmez,
            // doğrudan kendi satırında belirir.
            static bool s_snap = true;
            if (s_snap) { ui::AnimSet(ImGui::GetID("##tabind"), target); s_snap = false; }
            const float ty = ui::Anim(ImGui::GetID("##tabind"), target, 16.0f);
            dl->AddRectFilled(ImVec2(tx, ty), ImVec2(tx + tw, ty + tabH), theme::White(0.065f), px(9));
            dl->AddRect(ImVec2(tx, ty), ImVec2(tx + tw, ty + tabH), theme::White(0.06f), px(9), 0,
                        ImMax(1.0f, px(1)));
            fx::RadialGradient(dl, ImVec2(0.0f, ty + tabH * 0.5f), px(30), px(26), theme::White(0.22f),
                               theme::White(0.0f), 24);
            dl->AddRectFilled(ImVec2(pos.x, ty + px(11)), ImVec2(pos.x + px(3), ty + tabH - px(11)),
                              theme::White(0.95f), px(2));

            for (int i = 0; i < g_c->page_count; ++i)
            {
                ImGui::SetCursorScreenPos(ImVec2(tx, ty0 + i * (tabH + gap)));
                const Page& p = g_c->pages[i];
                if (ui::Tab(p.label, p.icon, g_page == i, ImVec2(tw, tabH))) SetPage(i);
            }

            // alt durum kartı: sabit yükseklik, panelin dibine yaslanır
            const int total = ImMin(3, g_c->status_count + (g_c->status_size ? 1 : 0));
            if (total <= 0) return;
            const ImVec2 u0(tx, pos.y + height - px(104)), u1(tx + tw, pos.y + height - px(16));
            ui::Card(dl, u0, u1, px(12));
            ImGui::SetCursorScreenPos(ImVec2(u0.x + px(16), u0.y + px(14)));
            ui::SectionLabel("DURUM");

            // satır dikey konumları sabit: kart yüksekliği de sabit
            const float kLineY[3] = { px(36), px(52), px(70) };
            const auto& F = theme::fonts;
            char size_line[24];
            for (int i = 0; i < total; ++i)
            {
                const bool size_row = g_c->status_size && i == total - 1;
                const char* text = nullptr;
                if (size_row)
                {
                    snprintf(size_line, sizeof(size_line), "%d x %d px",
                             (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
                    text = size_line;
                }
                else if (i < g_c->status_count)
                    text = g_c->status_lines[i];
                if (!text) continue;

                // son satır vurgu satırıdır: medium + Secondary
                const bool emphasis = (i == total - 1);
                ui::Text(dl, emphasis ? F.medium : F.regular, theme::size::Caption,
                         ImVec2(u0.x + px(16), u0.y + kLineY[i]),
                         theme::Gray(emphasis ? theme::ink::Secondary : theme::ink::Tertiary), text);
            }
        }

        void HandleDrag()
        {
            const ImGuiIO& io = ImGui::GetIO();
            if (ImGui::IsMouseClicked(0) && io.MousePos.y >= 0.0f && io.MousePos.y < px(64) &&
                !ImGui::IsAnyItemHovered() && !ImGui::IsAnyItemActive())
            {
                g_dragging = true;
                ::GetCursorPos(&g_dragStart);
                ::GetWindowRect(g_hwnd, &g_dragRect);
            }
            if (!g_dragging) return;
            if (!ImGui::IsMouseDown(0)) { g_dragging = false; return; }
            POINT p;
            ::GetCursorPos(&p);
            ::SetWindowPos(g_hwnd, nullptr, g_dragRect.left + p.x - g_dragStart.x, g_dragRect.top + p.y - g_dragStart.y,
                           0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
    }

    int   CurrentPage() { return g_page; }
    HWND  Hwnd() { return g_hwnd; }
    float Corner() { return g_corner; }

    void SetPage(int index)
    {
        if (g_c && index >= 0 && index < g_c->page_count) g_page = index;
    }

    void Init(HWND hwnd, float corner_radius, const Content& c, const Options& opt)
    {
        g_hwnd   = hwnd;
        g_corner = corner_radius;
        g_c      = &c;
        g_opt    = opt;
        g_page   = (opt.start_page >= 0 && opt.start_page < c.page_count)
                       ? opt.start_page : ImMin(c.first_page, ImMax(0, c.page_count - 1));
    }

    void Frame()
    {
        if (!g_c) return;
        const ImVec2 ds = ImGui::GetIO().DisplaySize;
        if (ds.x <= 0.0f || ds.y <= 0.0f) return;

        if (g_c->tick) g_c->tick(ImGui::GetTime());

        // -toast / -page: belirli bir durumu tıklamadan ekrana getir. begin()
        // ilk karede çağrılır, böylece ImGui bağlamı (GetID, bildirim kuyruğu)
        // henüz çizim başlamadan kullanılabiliyor.
        if (g_first_frame)
        {
            g_first_frame = false;
            if (g_c->begin) g_c->begin(g_opt);
            if (g_opt.toast >= 0 && g_opt.toast <= 3)
                ui::Notify((ui::Toast)g_opt.toast, "Bildirim",
                           "Bu kayıt -toast ile açıldı, tıklama gerektirmez.");
        }

        const float a = 1.0f;
        fx::DrawBackground(ImGui::GetBackgroundDrawList(), ImVec2(0, 0), ds, a);

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ds);
        ImGui::Begin("##root", nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground |
                     ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoScrollWithMouse);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        DrawTopBar(ds, dl);

        const float sidebarW = px(216);
        const ImVec2 bodyPos = ImVec2(px(22), px(64));
        const float bodyH    = ds.y - bodyPos.y - px(22);
        DrawSidebar(bodyPos, sidebarW, bodyH);

        const float contentW = ds.x - bodyPos.x - sidebarW - px(38);
        ImGui::SetCursorScreenPos(ImVec2(bodyPos.x + sidebarW + px(16), bodyPos.y));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        ImGui::BeginChild("##content", ImVec2(contentW, bodyH), ImGuiChildFlags_None,
                          ImGuiWindowFlags_NoBackground);
        const Page& page = (g_page >= 0 && g_page < g_c->page_count) ? g_c->pages[g_page] : Page{};
        if (page.draw) page.draw(contentW);

        // 6px'lik silik tutamaç tek başına "burada fazlası var" demiyor; ipucu
        // yalnızca içerik gerçekten taşıyorsa çiziliyor. Kartların üstüne değil,
        // onların altındaki 22px'lik paya oturuyor — sağ alt toaster'ın yeri.
        // Ön plana çizildiği için modal karartmasının da üstünde kalır; açık bir
        // popup varken gösterilmez.
        if (ImGui::GetScrollMaxY() > 0.0f &&
            !ImGui::IsPopupOpen(0u, ImGuiPopupFlags_AnyPopup))
        {
            const char*  hint = "İçerik taşıyor — fare tekerleğiyle kaydır";
            const ImVec2 ts   = ui::TextSize(theme::fonts.regular, theme::size::Micro, hint);
            const ImVec2 vp   = ImGui::GetWindowPos();
            const ImVec2 vs   = ImGui::GetWindowSize();
            const ImVec2 c    = vp + ImVec2(px(18), vs.y + px(7));
            ImDrawList* fdl = ImGui::GetForegroundDrawList();
            // Payın yüksekliği px(22): kapsama kenarları kesmeyecek kadar küçük tutuluyor.
            fdl->AddRectFilled(c - px(7, 3), c + ts + px(7, 3), theme::Black(0.62f), px(7));
            fdl->AddRect(c - px(7, 3), c + ts + px(7, 3), theme::White(0.10f), px(7), 0,
                         ImMax(1.0f, px(1)));
            ui::Text(fdl, theme::fonts.regular, theme::size::Micro, c,
                     theme::Gray(theme::ink::Tertiary), hint);
        }
        ImGui::EndChild();
        ImGui::PopStyleVar(2);

        HandleDrag();
        ImGui::End();

        fx::DrawSweep(ImGui::GetForegroundDrawList(), ImVec2(0, 0), ds, a * 0.5f);
        ui::RenderNotifications(ds);
        dl = ImGui::GetForegroundDrawList();
        dl->AddRect(ImVec2(0.5f, 0.5f), ds - ImVec2(0.5f, 0.5f), IM_COL32(255, 255, 255, (int)(26.0f * a)),
                    g_corner, 0, 1.0f);
    }
}
