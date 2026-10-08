# Bildirimler

[MIT lisansı](LICENSE) yalnızca bu depodaki tema kaynak kodunu kapsar. Bu dosya,
o lisansın dokunmadığı üçüncü taraf bileşenleri bildirir.

| Bileşen | Lisans | Nerede |
|---|---|---|
| **Dear ImGui** (arayüz kütüphanesi) | MIT | `third_party/imgui/LICENSE.txt` |
| **Inter** yazı tipi (exe içine gömülü) | SIL Open Font License 1.1 | `res/fonts/Inter-OFL.txt` |
| **Windows SDK / MSVC araç seti** | Microsoft lisansları | Yalnızca derleme sırasında kullanılır |

## Dear ImGui

`third_party/imgui` altında yalnızca bu projenin derlediği dosyalar bulunur:
çekirdek (`imgui`, `imgui_draw`, `imgui_tables`, `imgui_widgets`), Win32 ve DX11
backend'leri, yapılandırma ve stb başlıkları. Sürüm `1.93.0 WIP`
(`IMGUI_VERSION_NUM 19297`). Stil `FontSizeBase` / `FontScaleDpi` alanlarını
kullandığı için 1.92 veya üzeri gerekir.

`LICENSE.txt` bulunduğu dizinde kalır; kaynağı kendi indirdiğin sürümle
değiştirirsen o dizinin lisansını da koru.

## Inter

Üç ağırlık (`Regular`, `SemiBold`, `Bold`) Latin + Türkçe + noktalama
kapsamıyla altkümelendirilmiş hâlde `res/fonts` altında durur ve
`tools\make_font_data.ps1` ile `src/gui/font_data.cpp` içine gömülür. OFL
metninin bir kopyası gömülü bayt tablolarıyla birlikte taşınır.

Bu lisanslar kendi metinleriyle yürürlüktedir; projenin MIT lisansı onları ne
değiştirir ne de genişletir.
