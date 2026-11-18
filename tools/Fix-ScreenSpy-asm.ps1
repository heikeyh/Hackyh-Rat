# 将 ScreenSpyDlg.cpp 的内联汇编差分块拷贝改写为可移植 C（x64 不支持 __asm）
$ErrorActionPreference='Stop'
$root="C:\Users\Administrator\Desktop\本机研究原理"
$f=Join-Path $root 'gh0st\ScreenSpyDlg.cpp'
$enc=[Text.Encoding]::GetEncoding(936)
$raw=[IO.File]::ReadAllText($f,$enc)
$eol= if($raw.Contains("`r`n")){"`r`n"}else{"`n"}
$lines=New-Object 'System.Collections.Generic.List[string]'
foreach($l in ($raw -split "`r?`n")){ [void]$lines.Add($l) }

$start=-1
for($i=0;$i -lt $lines.Count;$i++){ if($lines[$i].Trim() -eq '__asm'){ $start=$i; break } }
if($start -lt 0){ throw 'no __asm' }
$end=-1
for($j=$start+1;$j -lt $lines.Count;$j++){ if($lines[$j].Trim() -eq '}'){ $end=$j; break } }
if($end -lt 0){ throw 'no asm end' }

$rep=[string[]]@(
"`t// 屏幕差分块拷贝（x64 无内联汇编，改用可移植 C）：每块 = [DWORD 偏移][DWORD 大小][数据]",
"`t{",
"`t`tDWORD dwRemain = dwBytes;",
"`t`tBYTE* pSrc = (BYTE*)lpNextScreen;",
"`t`tBYTE* pDstBase = (BYTE*)lpFirstScreen;",
"`t`twhile(dwRemain > 0)",
"`t`t{",
"`t`t`tDWORD dwBlockOffset = *(DWORD*)pSrc; pSrc += 4;",
"`t`t`tDWORD dwBlockSize   = *(DWORD*)pSrc; pSrc += 4;",
"`t`t`tmemcpy(pDstBase + dwBlockOffset, pSrc, dwBlockSize);",
"`t`t`tpSrc += dwBlockSize;",
"`t`t`tdwRemain -= 8;",
"`t`t`tdwRemain -= dwBlockSize;",
"`t`t}",
"`t}"
)

$lines.RemoveRange($start,$end-$start+1)
$lines.InsertRange($start,$rep)
[IO.File]::WriteAllText($f,($lines -join $eol),$enc)
"ScreenSpyDlg __asm 已改写（行 $start..$end）"
