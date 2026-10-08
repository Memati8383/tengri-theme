# Değişim günlüğü

Biçim [Keep a Changelog](https://keepachangelog.com/tr/1.1.0/)'a, sürümleme
[SemVer](https://semver.org/lang/tr-TR/)'e göre.

## [Unreleased]

### Değişti

- `tools/check_showcase.ps1` çağrı aradığı metni yorumlardan arındırıyor: bir
  sembolün adını yoruma yazmak artık kapıyı geçirmiyor. Kapsama değişmedi
  (48 sembol, 0 eksik) ve sayım betikten bağımsız ikinci bir çözümleyiciyle
  doğrulandı.
- `build.bat` ve CMake linker'a `/Brepro` veriyor: PE başlığındaki
  `TimeDateStamp` sabitleniyor. Ölçüldü — `build/` dizini iki kez silinip
  temizlenen derlemeler `TengriApp.exe` ve `TengriThemePreview.exe` için
  bayt bayt aynı çıktıyı verdi (`cmp` farkı yok, `/W4` uyarı sayısı 0). v1.1.0
  artefaktları bu bayraktan önce linklendi, yani o sürümün sağlama değerleri
  tek seferlik birer kimlikti; bir sonraki sürümden itibaren doğrulanabilir.

[Unreleased]: https://github.com/Memati8383/tengri-theme/compare/v1.1.0...HEAD

## [1.1.0] — 2026-10-08

### Eklendi

- `tools/check_showcase.ps1`: vitrin kapsama kapısı. Başlık dosyalarındaki her
  public bildirimi toplar ve `src/demo.cpp` / `src/app/shell.cpp` içinde
  çağrıldığını arar; eksikse çıkış kodu 1. Ölçülen kapsam: 48 benzersiz sembol,
  0 eksik. CI'de ayrı bir adım olarak çalışır.
- `tools/check_text_encoding.ps1`: kodlama kapısı. Her metin dosyasının geçerli
  UTF-8 olduğunu, iki kez kodlanmış UTF-8 izi taşımadığını ve `.ps1` dosyalarının
  BOM ile yazıldığını denetler. CI'da ayrı adım.
- Vitrinde iki yeni kart: **Switch** (`ui::DrawSwitch` canlı anahtar +
  `on = 0.0/0.5/1.0` ara değerleri, `ui::Key`/`ui::Anim` ile yumuşatma) ve
  **Marka resmi** (`logo::Draw`, `logo::DrawFitted`, `logo::Tex` + `Ready`
  karşılaştırması). **Efekt primitifleri** kartı Efektler sayfasının en üstüne
  alındı: `fx::RadialGradient`, `fx::GradientQuad`, `fx::Shine` tek tek görünüyor.
- `.gitattributes`: satır sonu ve ikili dosya politikası; `third_party/**`
  linguist-vendored, üretilen gömülü veri dosyaları linguist-generated.
- `CHANGELOG.md` ve `CONTRIBUTING.md`.

### Düzeltilenler

- Üç dosyada iki kez kodlanmış UTF-8 yorum satırları (`src/gui/brand_icons.cpp`,
  `src/gui/font_data.cpp`, `src/gui/logo_data.cpp`). Kök neden: üreteç `.ps1`
  dosyaları BOM'suzdu, Windows PowerShell 5.1 onları CP1254 okuyup bozuk metin
  üretiyordu. Üreteçler BOM ile yazıldı, çıktılar yeniden üretildi.
- Olmayan bir dosyaya atıf yapan yorumlar: `tools/make_brand_icons.ps1` depoda
  yok; `brand_icons.hpp`, `brand_icons.cpp` ve `icons.cpp` içindeki atıflar
  verinin kaynağını anlatacak şekilde düzeltildi.
- `src/gui/fx.hpp` içindeki "parımcıklar" yazımı; `src/gui/theme.hpp` içindeki
  kontrast tablosu artık `docs/img/preview-components.png`'den ölçülen gerçek
  piksellerle yazılı ve var olmayan `TextCentered` çağrısını anmıyor.
- README: üç yerde "ışma" → "ışıma"; vitrin tablosuna yeni kartların API'si
  eklendi; hiç kullanılmayan `preview-modal.png` ve `scaffold-settings.png`
  Ekranlar bölümüne alındı; sürüm varlıklarının nereden geldiği ve sağlama
  değerlerinin neden bit-bit yeniden üretilebilir olmadığı yazıldı.

### Değişti

- Grafikler sayfası sıkılaştırıldı (grafik yüksekliği 140 → 118 px, marka
  kartı 78 → 62 px): dört kart 1280×800 ölçekte katlanmadan görünüyor.
- CI dört adıma çıktı: `build.bat`, CMake + `ctest`, `tests\run_tests.bat`,
  `tools\check_showcase.ps1` + `tools\check_text_encoding.ps1`.

## [1.0.0] — 2026-10-08

### Eklendi

- İlk yayın: monokrom Dear ImGui teması (`src/gui/`), kenarlıksız DPI ölçekli
  pencere + uygulama kabuğu (`src/app/shell.*`), beş sayfalık vitrin
  (`src/demo.cpp`) ve `src/app/my_app.cpp` başlangıç şablonu.
- Gömülü Inter alt kümesi (regular/medium/bold, Türkçe bloğu), gömülü marka
  PNG'si (RT_RCDATA + WIC + DX11 dokusu), 31 vektör ikon, bildirim katmanı,
  arka plan efektleri.
- `build.bat` ve CMake hedefleri; `WM_NCHITTEST` kenar hesabı için 17 durumlu
  yan etkisiz birim testi; `windows-latest` GitHub Actions iş akımı.

[1.1.0]: https://github.com/Memati8383/tengri-theme/compare/v1.0.0...v1.1.0
[1.0.0]: https://github.com/Memati8383/tengri-theme/releases/tag/v1.0.0
