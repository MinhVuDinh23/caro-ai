# Trình chạy nhanh Cờ Caro 9x9: chọn chế độ chơi / chạy thực nghiệm / kiểm thử.
# Gọi qua play.bat (bấm đúp) hoặc:  powershell -ExecutionPolicy Bypass -File scripts\play.ps1 [-Choice 1..9]
# Ưu tiên file chạy sẵn trong bin\, nếu không có thì build\Release\, nếu vẫn không có thì biên dịch.
param([string]$Choice = "")

[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

$bin = Join-Path $root "bin"
if (-not (Test-Path (Join-Path $bin "caro_gui.exe"))) { $bin = Join-Path $root "build\Release" }
if (-not (Test-Path (Join-Path $bin "caro_gui.exe"))) {
    Write-Host "Chưa có file chạy - đang biên dịch (cần Visual Studio có C++)..."
    & cmd /c "`"$root\build.bat`""
    if ($LASTEXITCODE -ne 0) {
        Read-Host "Biên dịch thất bại, xem README.md. Nhấn Enter để thoát" | Out-Null
        exit 1
    }
}

function Start-Gui([string]$argsText) {
    Start-Process -FilePath (Join-Path $bin "caro_gui.exe") -ArgumentList $argsText -WorkingDirectory $bin
}

function Wait-Key {
    if ($Choice -eq "") { Read-Host "Nhấn Enter để quay lại menu" | Out-Null }
}

# Thực hiện một lựa chọn; trả về $false khi người dùng chọn thoát.
function Invoke-Choice([string]$c) {
    switch ($c) {
        "1" { Start-Gui "--x human --o minimax" }
        "2" { Start-Gui "--x human --o mcts" }
        "3" { Start-Gui "--x minimax --o human" }
        "4" { Start-Gui "--x human --o human" }
        "5" { Start-Gui "--x mcts --o minimax --auto" }
        "6" { Start-Gui "--x mcts --o mcts --auto" }
        # | Out-Host: in kết quả ra màn hình thay vì thành giá trị trả về của hàm
        "7" { & (Join-Path $bin "caro_bench.exe") --exp all --seed 2026 --out results | Out-Host; Wait-Key }
        "8" { & (Join-Path $bin "caro_bench.exe") --exp plot --out results | Out-Host; Wait-Key }
        "9" {
            $t = Join-Path $bin "caro_tests.exe"
            if (Test-Path $t) { & $t | Out-Host } else { Write-Host "Không tìm thấy caro_tests.exe" }
            Wait-Key
        }
        "0" { return $false }
        default { }
    }
    return $true
}

if ($Choice -ne "") {  # chạy thẳng một lựa chọn, không hiện menu
    Invoke-Choice $Choice | Out-Null
    exit $LASTEXITCODE
}

while ($true) {
    Clear-Host
    Write-Host "=============================================="
    Write-Host "       CỜ CARO 9x9  -  MiniMax và MCTS"
    Write-Host "=============================================="
    Write-Host "  1. Người (X) đấu MiniMax (O)"
    Write-Host "  2. Người (X) đấu MCTS (O)"
    Write-Host "  3. MiniMax (X, đi trước) đấu Người (O)"
    Write-Host "  4. Người đấu Người"
    Write-Host "  5. Máy - Máy: MCTS (X) đấu MiniMax (O), tự chạy"
    Write-Host "  6. Máy - Máy: MCTS (X) đấu MCTS (O), tự chạy"
    Write-Host "  ----------------------------------------------"
    Write-Host "  7. Chạy toàn bộ thực nghiệm E1-E5 (khoảng 4 phút) -> results\"
    Write-Host "  8. Vẽ lại biểu đồ + bảng từ CSV có sẵn trong results\"
    Write-Host "  9. Chạy kiểm thử tự động (caro_tests)"
    Write-Host "  0. Thoát"
    Write-Host ""
    Write-Host "  (Trong giao diện vẫn đổi được người chơi và tham số bất cứ lúc nào.)"
    Write-Host ""
    $c = Read-Host "Chọn [0-9]"
    if ($null -eq $c) { break }  # hết đầu vào
    if (-not (Invoke-Choice $c.Trim())) { break }
}
