# 修复 x64 上线闪退：完成键与 m_Dialog 指针宽度（DWORD/int -> ULONG_PTR）
$ErrorActionPreference='Stop'
$root="C:\Users\Administrator\Desktop\本机研究原理"
function EncOf($f){
  $b=[IO.File]::ReadAllBytes($f)
  if($b.Length-ge3 -and $b[0]-eq0xEF -and $b[1]-eq0xBB -and $b[2]-eq0xBF){ return (New-Object Text.UTF8Encoding($true)) }
  try{ $s=New-Object Text.UTF8Encoding($false,$true); [void]$s.GetString($b); return (New-Object Text.UTF8Encoding($false)) }
  catch{ return [Text.Encoding]::GetEncoding(936) }
}
function Edit-File($rel,[scriptblock]$fn){
  $f=Join-Path $root $rel
  $enc=EncOf $f
  $raw=[IO.File]::ReadAllText($f,$enc)
  $eol=if($raw.Contains("`r`n")){"`r`n"}else{"`n"}
  $lines=New-Object 'System.Collections.Generic.List[string]'
  foreach($l in ($raw -split "`r?`n")){ [void]$lines.Add($l) }
  $res=& $fn $lines $raw
  [IO.File]::WriteAllText($f,($lines -join $eol),$enc)
  "OK $rel  $res"
}

# IOCPServer.h
Edit-File 'gh0st\include\IOCPServer.h' {
  param($lines,$raw)
  $n=0
  for($i=0;$i -lt $lines.Count;$i++){
    if($lines[$i].Contains('m_Dialog[2]') -and $lines[$i].Contains('int')){
      $k=$lines[$i].IndexOf('int'); $lines[$i]=$lines[$i].Substring(0,$k)+'ULONG_PTR'+$lines[$i].Substring($k+3); $n++
    }
    if($lines[$i].Contains('hCompletionPort, DWORD dwCompletionKey)')){
      $lines[$i]=$lines[$i].Replace('hCompletionPort, DWORD dwCompletionKey)','hCompletionPort, ULONG_PTR dwCompletionKey)'); $n++
    }
  }
  "($n)"
}

# IOCPServer.cpp
Edit-File 'gh0st\include\IOCPServer.cpp' {
  param($lines,$raw)
  $n=0
  for($i=0;$i -lt $lines.Count;$i++){
    if($lines[$i].Contains('hCompletionPort, DWORD dwCompletionKey)')){
      $lines[$i]=$lines[$i].Replace('hCompletionPort, DWORD dwCompletionKey)','hCompletionPort, ULONG_PTR dwCompletionKey)'); $n++
    }
    if($lines[$i].Contains('(DWORD) pContext')){
      $lines[$i]=$lines[$i].Replace('(DWORD) pContext','(ULONG_PTR) pContext'); $n++
    }
    if($lines[$i].Contains('(DWORD) NULL')){
      $lines[$i]=$lines[$i].Replace('(DWORD) NULL','(ULONG_PTR) NULL'); $n++
    }
  }
  "($n)"
}

# gh0stView.cpp：m_Dialog[1] 写入改 ULONG_PTR
Edit-File 'gh0st\gh0stView.cpp' {
  param($lines,$raw)
  $n=0
  for($i=0;$i -lt $lines.Count;$i++){
    if($lines[$i].Contains('m_Dialog[1] = (int)dlg')){
      $lines[$i]=$lines[$i].Replace('m_Dialog[1] = (int)dlg','m_Dialog[1] = (ULONG_PTR)dlg'); $n++
    }
  }
  "($n)"
}
"DONE"
