# Katkı rehberi

Küçük bir depo: tema (`src/gui/`), kabuk (`src/app/`), vitrin (`src/demo.cpp`).
Bir değişiklik göndermeden önce aşağıdaki dört kapıyı yerelde koş; CI aynı
kapıları koşuyor.

## Derleme

```bat
build.bat
```

`vcvars64` yolunu vswhere ile bulur, gömülü varlıkları üretir ve iki exe linkler:
`build\TengriApp.exe` (şablon) ve `build\TengriThemePreview.exe` (vitrin).
Ölçüt `/W4` sıfır uyarı — uyarı getiren PR derlse de geçmez sayılır.

CMake karşılığı:

```bat
cmake -S . -B build_cmake -G "Visual Studio 17 2022"
cmake --build build_cmake --config Release
ctest --build-config Release --test-dir build_cmake
```

## Kapılar

```bat
tests\run_tests.bat
```

`WM_NCHITTEST` kenar hesabı, 17 durum. Test UI'a, ağa veya kullanıcı verisine
dokunmaz; yan etkisi yoktur.

```powershell
powershell -NoProfile -File tools\check_showcase.ps1 -List
powershell -NoProfile -File tools\check_text_encoding.ps1
```

- **Vitrin kapsama kapısı**: `src/gui/*.hpp` ve `src/app/shell.hpp` içindeki her
  public bildirim `src/demo.cpp` veya `src/app/shell.cpp` içinde çağrılmalı.
  Yeni bir public API eklerken ya vitrinde göster ya da betiğin başındaki
  `$lifecycle` listesine neden olduğunu yazarak ekle; sessiz muafiyet yok.
- **Kodlama kapısı**: her metin dosyası geçerli UTF-8 olmalı, iki kez kodlanmış
  UTF-8 izi bulunmamalı ve `.ps1` dosyaları BOM ile yazılmalı. Windows PowerShell
  5.1 BOM'suz UTF-8'i CP1254 okur; Türkçe yorum satırları bu yolla bozulur ve
  üreteçler bozuk metni derlenen dosyalara yazar.

## Üretilen dosyalar

`src/gui/font_data.cpp` ve `src/gui/logo_data.cpp` elle düzenlenmez; `build.bat`
her derlemede `tools\make_font_data.ps1` ve `tools\make_logo_data.ps1` ile
yeniden üretir. Kaynaklar `res/fonts/*.subset.ttf` ve `res/tengri-logo.png`.
Yazı tipi değiştirmek istersen alt kümeleme (fontTools) bu depoda değil: hazır
alt kümeyi `res/fonts` altına koy, üreteç BOM'lu kalsın.

## Vitrini ve ekran görüntülerini güncelleme

README'deki görseller `PrintWindow` ile, **fare gönderilmeden** üretilir:

```bat
TengriThemePreview.exe -page 2 -size 1280x800 -toast 3
TengriThemePreview.exe -modal
TengriApp.exe -page 1
```

Bir kart eklediysen veya sıraları değiştiyse ilgili sayfayı yeniden yakala ve
`docs/img/` altındaki dosyanın yerine koy; ekran görüntüsünün gerçekten ne
gösterdiğini README'de yazdığın satırla karşılaştır. Kapalı bir sayfanın
altındaki kart için yakalama yapma: kartı görünür alana sığacak şekilde
düzenle ya da açıklamanı görünen içeriye göre yaz.

## Küçük sözleşmeler

- Piksel yazma: `theme::px(...)`, punto yazma: `theme::size::*`, renk yazma:
  `theme::Gray(theme::ink::*)`. `theme::ink::*` **float**tur, `ImU32` değil —
  çıplak float'ı renge çevirirsen metin sessizce kaybolur.
- Yorumlar neden'i yazsın, ne yaptığını değil; neden-sonuç bağlantısı olmayan
  yorum silinir.
- Commit mesajı tek cümle, emir kipi, Türkçe ya da İngilizce — ama başlığı
  ölçülebilir tut ("vitrine switch kartı eklendi", "geliştirme" değil).
- E-posta gizliliği: GitHub, özel adresi ifşa eden gönderimi `GH007` ile reddeder.
  Profilindeki noreply adresini kullan (`git -c user.email=... commit` yeter;
  depoda git config değiştirilmez).
