$dst = "C:\Users\heike\Desktop\本机研究原理\ToolkitPro_x64\patches"
$LF = [char]10
$t = [char]9

function Patch-File($file, $pairs) {
    $p = Join-Path $dst $file
    $bytes = [IO.File]::ReadAllBytes($p)
    $hasBom = $bytes.Length -ge 3 -and $bytes[0] -eq 0xEF -and $bytes[1] -eq 0xBB -and $bytes[2] -eq 0xBF
    $enc = New-Object Text.UTF8Encoding($hasBom)
    $c = $enc.GetString($bytes)
    $n = 0
    foreach ($pair in $pairs) {
        if ($c.Contains($pair[0])) { $c = $c.Replace($pair[0], $pair[1]); $n++ }
        else { Write-Output "$file : MISS" }
    }
    [IO.File]::WriteAllText($p, $c, $enc)
    Write-Output "$file : $n patches applied"
}

# 1. TabColor: fix include path
Patch-File "XTPTabPaintManagerColorSet.cpp" @(
    ,@(
        '#include "Common/XTPDisableNoisyWarnings.h"',
        '#include "Common/Base/Diagnostic/XTPDisableNoisyWarnings.h"'
    )
)

# 2. TextOnPath: qualify Gdiplus types
Patch-File "XTPTextOnPath.cpp" @(
    ,@(
        'm_pFont = new Font(FontFamily::GenericMonospace(), 11, FontStyleBold );',
        'm_pFont = new Gdiplus::Font(Gdiplus::FontFamily::GenericMonospace(), 11, Gdiplus::FontStyleBold );'
    ),
    ,@(
        'm_pFillBrush = new SolidBrush((ARGB)Color::Black);',
        'm_pFillBrush = new Gdiplus::SolidBrush((Gdiplus::ARGB)Gdiplus::Color::Black);'
    )
)

# 3. Markup: pass BOOL for force-space flags
Patch-File "XTPMarkupBuilder.cpp" @(
    ,@(
        'nodeText.length(), pContent->previousSibling, pContent->nextSibling);',
        'nodeText.length(), (NULL != pContent->previousSibling), (NULL != pContent->nextSibling));'
    )
)

# 4. CalendarData: guard MAPI code
$old1 = (@(
    '#include "Common/Base/Diagnostic/XTPDisableAdvancedWarnings.h"',
    '#include "Calendar/MAPI/mapidefs.h"',
    '#include "Calendar/MAPI/mapix.h"',
    '#include "Common/Base/Diagnostic/XTPEnableAdvancedWarnings.h"',
    '',
    '#include "Calendar/XTPCalendarMAPIWrapper.h"',
    '#include "Calendar/XTPCalendarMAPIDataProvider.h"',
    '#include "Calendar/XTPCalendarCustomDataProvider.h"'
) -join $LF)
$new1 = (@(
    '#ifdef _XTP_INCLUDE_CALENDAR_MAPI',
    '#include "Common/Base/Diagnostic/XTPDisableAdvancedWarnings.h"',
    '#include "Calendar/MAPI/mapidefs.h"',
    '#include "Calendar/MAPI/mapix.h"',
    '#include "Common/Base/Diagnostic/XTPEnableAdvancedWarnings.h"',
    '',
    '#include "Calendar/XTPCalendarMAPIWrapper.h"',
    '#include "Calendar/XTPCalendarMAPIDataProvider.h"',
    '#endif',
    '#include "Calendar/XTPCalendarCustomDataProvider.h"'
) -join $LF)

$old2 = (@(
    ($t + 'BOOL bProviderMAPI   = strConnStr_lower.Find(_T("provider=mapi"))   != -1;'),
    ($t + 'if (bProviderMAPI)'),
    ($t + '{'),
    ($t + $t + 'return xtpCalendarDataProviderMAPI;'),
    ($t + '}')
) -join $LF)
$new2 = (@(
    '#ifdef _XTP_INCLUDE_CALENDAR_MAPI',
    $old2,
    '#endif'
) -join $LF)

$old3 = (@(
    ($t + 'case xtpCalendarDataProviderMAPI:'),
    ($t + $t + 'pDataProvider = new CXTPCalendarMAPIDataProvider();'),
    ($t + $t + 'break;')
) -join $LF)
$new3 = (@(
    '#ifdef _XTP_INCLUDE_CALENDAR_MAPI',
    $old3,
    '#endif'
) -join $LF)

Patch-File "XTPCalendarData.cpp" @(
    ,@($old1, $new1),
    ,@($old2, $new2),
    ,@($old3, $new3)
)
