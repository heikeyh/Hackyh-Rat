# Build Codejock Xtreme ToolkitPro v18.5.0 x64 static library (Static Release, MBCS, static MFC)
# Uses the official ToolkitProStatic_vc150 project and the patched sources under .\patches
# Output: .\build\ToolkitPro1850vc150S_x64.lib
$ErrorActionPreference = 'Stop'

$projRoot = 'C:\Users\heike\Desktop\本机研究原理'
$cjRoot   = 'C:\Program Files (x86)\Codejock Software\MFC\Xtreme ToolkitPro v18.5.0'
$msb      = 'C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe'
$patches  = Join-Path $projRoot 'ToolkitPro_x64\patches'
$buildDir = Join-Path $projRoot 'ToolkitPro_x64\build'
$work     = 'C:\Windows\Temp\cj_build'

if (-not (Test-Path $cjRoot)) { throw "Codejock install not found: $cjRoot" }
if (-not (Test-Path $msb))    { throw "MSBuild not found: $msb" }

# 1. fresh writable copy
if (Test-Path $work) { Remove-Item $work -Recurse -Force }
New-Item -ItemType Directory -Force -Path (Join-Path $work 'Workspace') | Out-Null
Copy-Item (Join-Path $cjRoot 'Source') (Join-Path $work 'Source') -Recurse
New-Item -ItemType Directory -Force -Path (Join-Path $work 'Workspace\ToolkitPro') | Out-Null
Get-ChildItem (Join-Path $cjRoot 'Workspace\ToolkitPro') -File |
  Copy-Item -Destination (Join-Path $work 'Workspace\ToolkitPro')
Copy-Item (Join-Path $cjRoot 'Workspace\Resource') (Join-Path $work 'Workspace\Resource') -Recurse

# 2. apply patched sources (full-file replacements)
$map = @{
  'XTPMarkupBuilder.cpp' = 'Markup\XTPMarkupBuilder.cpp'
}
foreach ($k in $map.Keys) {
  $src = Join-Path $patches $k
  $dst = Join-Path $work ("Source\" + $map[$k])
  if (-not (Test-Path $src)) { throw "patch missing: $src" }
  Copy-Item $src $dst -Force
  Write-Output "patched: $($map[$k])"
}

# 3. build Static Release x64
$proj = Join-Path $work 'Workspace\ToolkitPro\ToolkitProStatic_vc150.vcxproj'
& $msb $proj /p:Configuration='Static Release' /p:Platform=x64 /p:PlatformToolset=v145 /t:Rebuild /v:minimal
if ($LASTEXITCODE -ne 0) { throw "Codejock build failed (exit=$LASTEXITCODE)" }

# 4. extract produced library
$produced = Join-Path $work 'lib\vc150x64\ToolkitPro1850vc150x64S.lib'
if (-not (Test-Path $produced)) { throw "produced lib missing: $produced" }
New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
$target = Join-Path $buildDir 'ToolkitPro1850vc150S_x64.lib'
Copy-Item $produced $target -Force
Write-Output ("library: {0} ({1:N0} bytes)" -f $target, (Get-Item $target).Length)

# 5. cleanup work copy (best effort; AV may briefly lock files)
for ($i = 0; $i -lt 5 -and (Test-Path $work); $i++) {
  try { Remove-Item $work -Recurse -Force -ErrorAction Stop }
  catch { Start-Sleep -Seconds 2 }
}
if (Test-Path $work) { Write-Output "warning: $work still locked; delete it later" }
Write-Output 'done.'
