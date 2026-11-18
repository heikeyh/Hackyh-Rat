# 修复 Codejock ToolkitPro v18.5.0 以在 x64 静态库形态编译
$ErrorActionPreference='Stop'
$src = "C:\Program Files (x86)\Codejock Software\MFC\Xtreme ToolkitPro v18.5.0\Source"

function Edit-File($rel, [string[]]$pairs){
  $f = Join-Path $src $rel
  $b=[IO.File]::ReadAllBytes($f)
  $hasBom=($b.Length-ge3 -and $b[0]-eq0xEF -and $b[1]-eq0xBB -and $b[2]-eq0xBF)
  $enc=New-Object System.Text.UTF8Encoding($hasBom)
  $c=[IO.File]::ReadAllText($f,$enc)
  for($i=0;$i -lt $pairs.Count;$i+=2){
    $old=$pairs[$i]; $new=$pairs[$i+1]
    if($c.Contains($new)){ "skip(already) $rel"; continue }
    if(-not $c.Contains($old)){ throw "[$rel] 未找到待替换文本" }
    $c=$c.Replace($old,$new)
  }
  [IO.File]::WriteAllText($f,$c,$enc)
  "OK  $rel"
}

$NL="`n"
$T="`t"

# 1) XTPCalendarData.cpp：MAPI include 段加条件
$old1 =
"#include ""Common/Base/Diagnostic/XTPDisableAdvancedWarnings.h""$NL"+
"#include ""Calendar/MAPI/mapidefs.h""$NL"+
"#include ""Calendar/MAPI/mapix.h""$NL"+
"#include ""Common/Base/Diagnostic/XTPEnableAdvancedWarnings.h""$NL$NL"+
"#include ""Calendar/XTPCalendarMAPIWrapper.h""$NL"+
"#include ""Calendar/XTPCalendarMAPIDataProvider.h""$NL"
$new1 =
"#if defined(_XTP_INCLUDE_CALENDAR_MAPI)$NL"+
"#include ""Common/Base/Diagnostic/XTPDisableAdvancedWarnings.h""$NL"+
"#include ""Calendar/MAPI/mapidefs.h""$NL"+
"#include ""Calendar/MAPI/mapix.h""$NL"+
"#include ""Common/Base/Diagnostic/XTPEnableAdvancedWarnings.h""$NL$NL"+
"#include ""Calendar/XTPCalendarMAPIWrapper.h""$NL"+
"#include ""Calendar/XTPCalendarMAPIDataProvider.h""$NL"+
"#endif // _XTP_INCLUDE_CALENDAR_MAPI$NL"

# 1b) provider=mapi 检测加条件
$old2 =
"${T}BOOL bProviderMAPI   = strConnStr_lower.Find(_T(""provider=mapi""))   != -1;$NL"+
"${T}if (bProviderMAPI)$NL${T}{$NL${T}${T}return xtpCalendarDataProviderMAPI;$NL${T}}$NL"
$new2 =
"#if defined(_XTP_INCLUDE_CALENDAR_MAPI)$NL"+
"${T}BOOL bProviderMAPI   = strConnStr_lower.Find(_T(""provider=mapi""))   != -1;$NL"+
"${T}if (bProviderMAPI)$NL${T}{$NL${T}${T}return xtpCalendarDataProviderMAPI;$NL${T}}$NL"+
"#endif$NL"

# 1c) CreateDataProvider 的 MAPI case 加条件
$old3 =
"${T}case xtpCalendarDataProviderMAPI:$NL"+
"${T}${T}pDataProvider = new CXTPCalendarMAPIDataProvider();$NL"+
"${T}${T}break;$NL"
$new3 =
"#if defined(_XTP_INCLUDE_CALENDAR_MAPI)$NL"+
"${T}case xtpCalendarDataProviderMAPI:$NL"+
"${T}${T}pDataProvider = new CXTPCalendarMAPIDataProvider();$NL"+
"${T}${T}break;$NL"+
"#endif$NL"

Edit-File 'Calendar\XTPCalendarData.cpp' @($old1,$new1,$old2,$new2,$old3,$new3)

# 2) FlowGraph XTPTextOnPath.cpp：全限定 GDI+
$old4="${T}m_pFont = new Font(FontFamily::GenericMonospace(), 11, FontStyleBold );$NL"
$new4="${T}m_pFont = new Gdiplus::Font(Gdiplus::FontFamily::GenericMonospace(), 11, Gdiplus::FontStyleBold );$NL"
$old5="${T}m_pFillBrush = new SolidBrush((ARGB)Color::Black);$NL"
$new5="${T}m_pFillBrush = new Gdiplus::SolidBrush((ARGB)Gdiplus::Color::Black);$NL"
Edit-File 'FlowGraph\XTPTextOnPath.cpp' @($old4,$new4,$old5,$new5)

# 3) Markup XTPMarkupBuilder.cpp：用兄弟节点是否元素来决定强制空格
$old6 =
"${T}${T}${T}${T}lpszContent = NormalizeXmlContentText(lpszContent, textBuffer,$NL"+
"${T}${T}${T}${T}${T}nodeText.length(), pContent->previousSibling, pContent->nextSibling);$NL"
$new6 =
"${T}${T}${T}${T}XTPXML::IXMLDOMNodePtr pPrevSib = pContent->previousSibling;$NL"+
"${T}${T}${T}${T}XTPXML::IXMLDOMNodePtr pNextSib = pContent->nextSibling;$NL"+
"${T}${T}${T}${T}BOOL bForceSpaceLeft  = (pPrevSib && pPrevSib->nodeType == XTPXML::NODE_ELEMENT);$NL"+
"${T}${T}${T}${T}BOOL bForceSpaceRight = (pNextSib && pNextSib->nodeType == XTPXML::NODE_ELEMENT);$NL"+
"${T}${T}${T}${T}lpszContent = NormalizeXmlContentText(lpszContent, textBuffer,$NL"+
"${T}${T}${T}${T}${T}nodeText.length(), bForceSpaceLeft, bForceSpaceRight);$NL"
Edit-File 'Markup\XTPMarkupBuilder.cpp' @($old6,$new6)

# 4) 补 Common\XTPDisableNoisyWarnings.h 转发头（cpp 按 Common/ 引用，实际在 Base/Diagnostic）
$fwd = Join-Path $src 'Common\XTPDisableNoisyWarnings.h'
$fwdContent = "// 兼容转发：实际文件位于 Base/Diagnostic$NL#pragma once$NL#include ""Base/Diagnostic/XTPDisableNoisyWarnings.h""$NL"
[IO.File]::WriteAllText($fwd,$fwdContent,(New-Object System.Text.UTF8Encoding($false)))
"OK  Common\XTPDisableNoisyWarnings.h (created)"

"ALL FIXES APPLIED"
