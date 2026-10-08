# Vitrin kapsama denetimi.
#
# src/gui/*.hpp ve src/app/shell.hpp icindeki her public fonksiyon, vitrini kuran
# dosyalardan birinde gecmek zorunda. README'deki "vitrin disarida tek bir API
# birakmiyor" iddiasini olculur bir kapıya cevirmek icin yazildi; CI bu betigi
# calistirir ve cikis kodu sifir olmali.
#
# Kullanim:  powershell -NoProfile -File tools\check_showcase.ps1
#           powershell -NoProfile -File tools\check_showcase.ps1 -List
#
# Cikis:  0 = kapsama tam,  1 = en az bir sembol vitrinde gozukmuyor.
# Not: yorumlar ASCII. Windows PowerShell 5.1 BOM'suz UTF-8 betikleri ANSI okur,
# Turkce karakterler ciktida bozulur; mantik bundan etkilenmez ama mesajlar 
# ASCII kaldi.

param([switch]$List)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot

# Vitrinin kendisi: tema vitrini ve kabugun cerceve cizimi.
$showcaseFiles = @('src/demo.cpp', 'src/app/shell.cpp')
# Can dongusu: pencere/D3D kurulmadan cagrilamaz, vitrinde gorunemez.
$harnessFiles  = @('src/main.cpp')
# Yalnizca can dongusunde gecen semboller; vitrinde beklenmez.
$lifecycle = @('Init', 'Shutdown', 'Frame', 'content', 'WinMain', 'main')

$headers = @(
    'src/gui/widgets.hpp',
    'src/gui/fx.hpp',
    'src/gui/icons.hpp',
    'src/gui/theme.hpp',
    'src/gui/logo.hpp',
    'src/app/shell.hpp'
)

function Read-Src {
    param($files)
    $text = ''
    foreach ($f in $files) {
        $p = Join-Path $root $f
        if (Test-Path -LiteralPath $p) { $text += (Get-Content -Raw -LiteralPath $p) }
    }
    return $text
}

$showcase = Read-Src $showcaseFiles
$harness  = Read-Src $harnessFiles

# Bir bildirim satiri: satirin basi bir donus tipiyle acilir, sondaki ')' ya ';'
# ya da '{' ile biter (inline tanim). Cagri satirlari bu kaliba girmez, cunku
# orada satir bir atama ya da kontrol ifadesiyle baslar.
# Anahtar sozcukler sembol adi degildir.
$keywords = @('if', 'for', 'while', 'switch', 'return', 'sizeof', 'catch',
              'else', 'do', 'struct', 'using', 'namespace', 'extern', 'assert')

$found   = @()
$missing = @()
$seen    = @{}

# Bir sembol "kullanildi" sayilir ya tam nitelendirilmis adıyla (ui::Text() gibi;
# ns-on eki baslikta bildirilen namespace'ten gelir) ya da nitelendirilmemis bir
# cagri olarak (using theme::px; satiri nedeniyle px() bare gorunur). Baska bir
# namespace'in ayni adli cagrisi (ImGui::Text) ikinci kurala girmez: ':' on
# tarafi engeller.
function Test-Used {
    param($text, $ns, $name)
    $esc = [regex]::Escape($name)
    return ($text -match "(?<!\w)$ns::$esc\s*\(") -or ($text -match "(?<![\w:])$esc\s*\(")
}

foreach ($h in $headers) {
    $path = Join-Path $root $h
    if (-not (Test-Path -LiteralPath $path)) { continue }

    $headText = Get-Content -Raw -LiteralPath $path
    $nsMatch  = [regex]::Match($headText, '(?m)^namespace\s+(\w+)')
    $ns = if ($nsMatch.Success) { $nsMatch.Groups[1].Value } else { '' }

    foreach ($raw in ($headText -split "`n")) {
        $line = ($raw -replace '//.*$', '').Trim()
        if ($line -eq '') { continue }
        if ($line -match '^(#|struct |using |namespace |}|\{|\*)') { continue }
        if ($line -notmatch '\)\s*(;|\{.*\}|\{$)\s*$') { continue }

        $m = [regex]::Match($line, '\b(\w+)\s*\(')
        if (-not $m.Success) { continue }
        $name = $m.Groups[1].Value
        if ($keywords -contains $name) { continue }

        $key = "$h|$name"
        if ($seen.ContainsKey($key)) { continue }
        $seen[$key] = $true

        $inShowcase = Test-Used $showcase $ns $name
        $inHarness  = Test-Used $harness  $ns $name
        $ok = if ($lifecycle -contains $name) { ($inShowcase -or $inHarness) } else { $inShowcase }

        $found += [pscustomobject]@{ Header = $h; Name = $name; NS = $ns; Ok = $ok }
        if (-not $ok) { $missing += [pscustomobject]@{ Header = $h; Name = $name } }
    }
}

$unique = ($found | Select-Object -ExpandProperty Name | Sort-Object -Unique).Count

if ($List) {
    $found | Sort-Object Header, Name | ForEach-Object {
        $mark = if ($_.Ok) { 'ok  ' } else { 'MISS' }
        Write-Host ("  [{0}] {1} :: {2}()" -f $mark, $_.Header, $_.Name)
    }
}

Write-Host ("vitrin kapsama: {0} benzersiz public sembol ({1} bildirim)" -f $unique, $found.Count)
if ($missing.Count -gt 0) {
    Write-Host "vitrinde gozukmeyenler:" -ForegroundColor Red
    $missing | ForEach-Object { Write-Host ("  {0} :: {1}()" -f $_.Header, $_.Name) }
    exit 1
}
Write-Host "kapsama tam: her public sembol vitrinde (veya can dongusunde) cagriliyor."
exit 0
