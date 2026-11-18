# 主控 x64：链接 libpng；条件禁用无64位版本的 SkinH
$ErrorActionPreference='Stop'
$root="C:\Users\Administrator\Desktop\本机研究原理"

function Detect-Encoding($f){
  $b=[IO.File]::ReadAllBytes($f)
  if($b.Length-ge3 -and $b[0]-eq0xEF -and $b[1]-eq0xBB -and $b[2]-eq0xBF){ return (New-Object Text.UTF8Encoding($true)) }
  try{ $s=New-Object Text.UTF8Encoding($false,$true); [void]$s.GetString($b); return (New-Object Text.UTF8Encoding($false)) }
  catch{ return [Text.Encoding]::GetEncoding(936) }
}

# A) vcxproj：x64 依赖增加 libpng
$vp=Join-Path $root 'gh0st\gh0st.vcxproj'
$enc=Detect-Encoding $vp
$c=[IO.File]::ReadAllText($vp,$enc)
$oldDep='ToolkitPro1850vc150S_x64.lib;SHLWAPI.LIB'
$newDep='ToolkitPro1850vc150S_x64.lib;../libpng_x64/libpng_x64_mt.lib;SHLWAPI.LIB'
if($c.Contains($newDep)){ 'skip vcxproj' }
else {
  if(-not $c.Contains($oldDep)){ throw 'vcxproj 依赖锚未找到' }
  $c=$c.Replace($oldDep,$newDep)
  [IO.File]::WriteAllText($vp,$c,$enc)
  'OK vcxproj 链接 libpng'
}

# B) gh0st.cpp：SkinH 条件禁用（x64）
$f=Join-Path $root 'gh0st\gh0st.cpp'
$enc2=Detect-Encoding $f
$raw=[IO.File]::ReadAllText($f,$enc2)
$eol=if($raw.Contains("`r`n")){"`r`n"}else{"`n"}
$lines=New-Object 'System.Collections.Generic.List[string]'
foreach($l in ($raw -split "`r?`n")){ [void]$lines.Add($l) }

# B1) include + pragma 包裹
$incOld="#include ""SkinH.h""$eol#pragma comment(lib, ""SkinH.lib"")"
$incNew="#ifndef _WIN64$eol#include ""SkinH.h""$eol#pragma comment(lib, ""SkinH.lib"")$eol#endif"
if($raw.Contains($incNew)){ 'skip include' }
else {
  if(-not $raw.Contains($incOld)){ throw 'SkinH include 锚未找到' }
  $raw=$raw.Replace($incOld,$incNew)
}

# 重建行（B1 已改 raw）
$lines=New-Object 'System.Collections.Generic.List[string]'
foreach($l in ($raw -split "`r?`n")){ [void]$lines.Add($l) }

# B2) AttachEx 行包裹（从后往前）
for($i=$lines.Count-1;$i -ge 0;$i--){
  if($lines[$i].Contains('SkinH_AttachEx')){
    $lines.Insert($i+1,'#endif')
    $lines.Insert($i,'#ifndef _WIN64')
    break
  }
}
[IO.File]::WriteAllText($f,($lines -join $eol),$enc2)
'OK gh0st.cpp SkinH x64 禁用'
"DONE"
