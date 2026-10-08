#pragma once

// Hakkında sayfasındaki marka işaretlerinin üçgen verisi.
//
// Marka logolarının üçgen verisi tek sefer üretilip buraya gömüldü; depoda
// üreteci yok, bu yüzden dosya kaynaktır ve elle düzenlenebilir.
//
// Koordinatlar -1..1 aralığında, işaretin merkezine göre konumlanmış float çiftleridir,
// böylece doğrudan icons::Draw'ın normalleştirilmiş P(x, y) yardımcısına takılır.
namespace brandicons
{
    // Sayılar üreticiden gelir, çünkü diziler burada extern bildiriliyor ve eksik türlü
    // bir dizi üzerinde sizeof derlenmiyor.
    extern const int kGitHubPointCount;
    extern const int kGitHubTriCount;

    extern const float         kGitHubPoints[];
    extern const unsigned short kGitHubTris[];
}