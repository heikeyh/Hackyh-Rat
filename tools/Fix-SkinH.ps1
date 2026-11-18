# x64 下条件禁用 SkinH（无64位 SkinH）：StdAfx.h include/pragma + gh0st.cpp AttachEx
$ErrorActionPreference='Stop'
$root="C:\Users\Administrator\Desktop\本机研究原理"
function Detect-Encoding($f){
  $b=[IO.File]::ReadAllBytes($f)
  if($b.Length-ge3 -and $b[0]-eq0xEF -and $b[1]-eq0xBB -and $b[2]-eq0xBF){ return (New-Object Text.UTF8Encoding($true)) }
  try{ $s=New-Object Text.UTF8Encoding($false,$true); [void]$s.GetString($b); return (New-Object Text.UTF8Encoding($false)) }
  catch{ return [Text.Encoding]::GetEncoding(936) }
}
function Wrap-Inc($rel){
  $f=Join-Path $root $rel
  $enc=Detect-Encoding $f
  $raw=[IO.File]::ReadAllText($f,$enc)
  $eol=if($raw.Contains("`r`n")){"`r`n"}else{"`n"}
  $old="#include ""SkinH.h""$eol#pragma comment(lib, ""SkinH.lib"")"
  $new="#ifndef _WIN64$eol#include ""SkinH.h""$eol#pragma comment(lib, ""SkinH.lib"")$eol#endif"
  if($raw.Contains($new)){ "skip $rel"; return }
  if(-not $raw.Contains($old)){ throw "[$rel] 锚未找到" }
  [IO.File]::WriteAllText($f,$raw.Replace($old,$new),$enc)
  "OK $rel"
}
Wrap-Inc 'gh0st\StdAfx.h'

# gh0st.cpp：AttachEx 行包裹
$f=Join-Path $root 'gh0st\gh0st.cpp'
$enc=Detect-Encoding $f
$raw=[IO.File]::ReadAllText($f,$enc)
$eol=if($raw.Contains("`r`n")){"`r`n"}else{"`n"}
$lines=New-Object 'System.Collections.Generic.List[string]'
foreach($l in ($raw -split "`r?`n")){ [void]$lines.Add($l) }
$done=$false
for($i=$lines.Count-1;$i -ge 0;$i--){
  if($lines[$i].Contains('SkinH_AttachEx')){
    $lines.Insert($i+1,'#endif')
    $lines.Insert($i,'#ifndef _WIN64')
    $done=$true; break
  }
}
if(-not $done){
  if($raw.Contains('#ifndef _WIN64')){ 'skip gh0st.cpp' } else { throw 'AttachEx 未找到' }
} else {
  [IO.File]::WriteAllText($f,($lines -join $eol),$enc)
  'OK gh0st.cpp'
}
"DONE"
