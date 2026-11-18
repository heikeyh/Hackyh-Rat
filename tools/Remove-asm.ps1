# 删除被控 Buffer.cpp / Proxy.h 中无实际效果的内联汇编占位块（x64 不支持 __asm）
$ErrorActionPreference='Stop'
$root="C:\Users\Administrator\Desktop\本机研究原理"
$files=@('Server\svchost\common\Buffer.cpp','Server\svchost\common\Proxy.h')

foreach($rel in $files){
  $f=Join-Path $root $rel
  $b=[IO.File]::ReadAllBytes($f)
  $hasBom=($b.Length-ge3 -and $b[0]-eq0xEF -and $b[1]-eq0xBB -and $b[2]-eq0xBF)
  $enc=New-Object Text.UTF8Encoding($hasBom)
  $raw=[IO.File]::ReadAllText($f,$enc)
  $eol= if($raw.Contains("`r`n")){"`r`n"}else{"`n"}
  $lines=New-Object 'System.Collections.Generic.List[string]'
  foreach($l in ($raw -split "`r?`n")){ [void]$lines.Add($l) }

  $count=0
  while($true){
    $idx=-1
    for($i=0;$i -lt $lines.Count;$i++){ if($lines[$i].Trim() -eq '__asm'){ $idx=$i; break } }
    if($idx -lt 0){ break }
    $end=-1
    for($j=$idx+1;$j -lt $lines.Count;$j++){ if($lines[$j].Trim() -eq '};'){ $end=$j; break } }
    if($end -lt 0){ throw "$rel __asm 未闭合" }
    $lines.RemoveRange($idx,$end-$idx+1)
    $count++
  }
  [IO.File]::WriteAllText($f,($lines -join $eol),$enc)
  "$rel 删除 __asm 块 $count 个"
}
"DONE"
