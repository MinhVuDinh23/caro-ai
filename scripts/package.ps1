# Đóng gói bài nộp: AI_GK_<MaLop>_Nhom<SoNhom>.zip gồm BaoCao/, MaNguon/, Slide/.
#
#   powershell -ExecutionPolicy Bypass -File scripts\package.ps1 -MaLop INT3401_1 -Nhom 5
#
# Các bước: biên dịch + kiểm thử + cập nhật bin\ (bỏ qua bằng -SkipBuild), chép mã nguồn vào MaNguon\,
# chép BaoCao\*.pdf và Slide\* từ thư mục gốc (nếu có), nén thành dist\<tên>.zip.
param(
    [Parameter(Mandatory = $true)][string]$MaLop,
    [Parameter(Mandatory = $true)][string]$Nhom,
    [switch]$SkipBuild
)
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$name = "AI_GK_${MaLop}_Nhom${Nhom}"
$dist = Join-Path $root "dist"
$stage = Join-Path $dist $name

if (-not $SkipBuild) {
    & cmd /c "`"$root\scripts\update_bin.bat`""
    if ($LASTEXITCODE -ne 0) { throw "Biên dịch hoặc kiểm thử thất bại - dừng đóng gói" }
}
foreach ($f in @("bin\caro_gui.exe", "bin\caro_bench.exe", "results\cross.csv")) {
    if (-not (Test-Path (Join-Path $root $f))) { throw "Thiếu $f (chạy scripts\update_bin.bat và caro_bench trước)" }
}

if (Test-Path $stage) { Remove-Item $stage -Recurse -Force }
$src = Join-Path $stage "MaNguon"
New-Item -ItemType Directory -Force $src, (Join-Path $stage "BaoCao"), (Join-Path $stage "Slide") | Out-Null

# Mã nguồn + bản chạy sẵn + kết quả (không chép build\, plans\, ghi chú nội bộ)
foreach ($item in @("src", "tests", "third_party", "assets", "scripts", "results", "bin",
                    "CMakeLists.txt", "build.bat", "play.bat", "README.md", ".gitignore")) {
    $p = Join-Path $root $item
    if (Test-Path $p) { Copy-Item $p -Destination $src -Recurse -Force }
    else { Write-Warning "Không có $item" }
}

# Báo cáo và slide do nhóm tự đặt vào BaoCao\ và Slide\ ở thư mục gốc
foreach ($dir in @("BaoCao", "Slide")) {
    $from = Join-Path $root $dir
    if (Test-Path $from) { Copy-Item (Join-Path $from "*") -Destination (Join-Path $stage $dir) -Recurse -Force }
    if (-not (Get-ChildItem (Join-Path $stage $dir) -ErrorAction SilentlyContinue)) {
        Write-Warning "$dir\ đang trống - nhớ đặt file vào $from rồi đóng gói lại"
    }
}

$zip = Join-Path $dist "$name.zip"
if (Test-Path $zip) { Remove-Item $zip -Force }
# ZipFile (.NET >= 4.6.1) ghi đường dẫn dạng "/" -> giải nén được trên mọi hệ điều hành
# Tự ghi từng mục với dấu "/" (ZipFile của PowerShell 5.1 ghi "\" -> 7-Zip/Linux không hiểu là thư mục).
Add-Type -AssemblyName System.IO.Compression, System.IO.Compression.FileSystem
$fs = [System.IO.File]::Open($zip, [System.IO.FileMode]::Create)
$archive = New-Object System.IO.Compression.ZipArchive($fs, [System.IO.Compression.ZipArchiveMode]::Create)
try {
    foreach ($file in Get-ChildItem $stage -Recurse -File) {
        $rel = $file.FullName.Substring($stage.Length + 1).Replace('\', '/')
        [System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile(
            $archive, $file.FullName, $rel, [System.IO.Compression.CompressionLevel]::Optimal) | Out-Null
    }
    foreach ($dir in @("BaoCao", "Slide")) {  # giữ thư mục rỗng trong zip
        if (-not (Get-ChildItem (Join-Path $stage $dir))) { $archive.CreateEntry("$dir/") | Out-Null }
    }
} finally {
    $archive.Dispose()
    $fs.Dispose()
}
$size = [math]::Round((Get-Item $zip).Length / 1MB, 1)
Write-Host "Đã tạo $zip ($size MB)"
