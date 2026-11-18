# 生成 Codejock ToolkitPro v18.5.0 x64 静态库工程
$ErrorActionPreference='Stop'
$src   = "C:\Program Files (x86)\Codejock Software\MFC\Xtreme ToolkitPro v18.5.0\Source"
$projD = "C:\Users\Administrator\Desktop\本机研究原理\ToolkitPro_x64"
$outLib= "C:\Program Files (x86)\Codejock Software\MFC\Xtreme ToolkitPro v18.5.0\Lib\vc150_x64\ToolkitPro1850vc150S_x64.lib"

$mapiCpp=@('XTPCalendarMAPIDataProvider.cpp','XTPCalendarMAPIWrapper.cpp')
$cpps = Get-ChildItem $src -Recurse -File -Filter *.cpp | Where-Object { $_.FullName -notmatch '\\Calendar\\MAPI\\' -and $mapiCpp -notcontains $_.Name } | Sort-Object FullName
"cpp count = $($cpps.Count)"

$sb = New-Object System.Text.StringBuilder
function A($t){ [void]$sb.AppendLine($t) }

A '<?xml version="1.0" encoding="utf-8"?>'
A '<Project DefaultTargets="Build" ToolsVersion="15.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">'
A '  <ItemGroup Label="ProjectConfigurations">'
A '    <ProjectConfiguration Include="Release|x64">'
A '      <Configuration>Release</Configuration>'
A '      <Platform>x64</Platform>'
A '    </ProjectConfiguration>'
A '  </ItemGroup>'
A '  <PropertyGroup Label="Globals">'
A '    <Keyword>MFCProj</Keyword>'
A '    <ProjectGuid>{B7C1A2D3-4E5F-4607-9A8B-CC1122334455}</ProjectGuid>'
A '  </PropertyGroup>'
A '  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />'
A '  <PropertyGroup Condition="''$(Configuration)|$(Platform)''==''Release|x64''" Label="Configuration">'
A '    <ConfigurationType>StaticLibrary</ConfigurationType>'
A '    <PlatformToolset>v145</PlatformToolset>'
A '    <UseOfMfc>Static</UseOfMfc>'
A '    <CharacterSet>MultiByte</CharacterSet>'
A '    <WholeProgramOptimization>false</WholeProgramOptimization>'
A '  </PropertyGroup>'
A '  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />'
A '  <PropertyGroup>'
A '    <OutDir>$(ProjectDir)build\</OutDir>'
A '    <IntDir>$(ProjectDir)build\</IntDir>'
A '  </PropertyGroup>'
A '  <ItemDefinitionGroup Condition="''$(Configuration)|$(Platform)''==''Release|x64''">'
A '    <ClCompile>'
A '      <PrecompiledHeader>Use</PrecompiledHeader>'
A '      <PrecompiledHeaderFile>StdAfx.h</PrecompiledHeaderFile>'
A '      <RuntimeLibrary>MultiThreaded</RuntimeLibrary>'
A '      <Optimization>MaxSpeed</Optimization>'
A '      <StringPooling>true</StringPooling>'
A '      <FunctionLevelLinking>true</FunctionLevelLinking>'
A '      <WarningLevel>Level3</WarningLevel>'
A '      <DisableSpecificWarnings>4005;4244;4267;4101;4189;4456;4457;4458;4459;4505;4701;4702;4706;4800;4815;4996;26439;26451;26495;26812</DisableSpecificWarnings>'
A "      <AdditionalIncludeDirectories>$projD;$src;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>"
A '      <PreprocessorDefinitions>WIN32;_WINDOWS;NDEBUG;_LIB;_XTP_STATICLINK;_XTP_USE_GDIPLUS;_CRT_SECURE_NO_WARNINGS;_WINSOCK_DEPRECATED_NO_WARNINGS;%(PreprocessorDefinitions)</PreprocessorDefinitions>'
A '      <AdditionalOptions>/Zc:forScope- /Zc:wchar_t /Zc:inline %(AdditionalOptions)</AdditionalOptions>'
A '    </ClCompile>'
A '  </ItemDefinitionGroup>'
A '  <ItemGroup>'
A "    <ClCompile Include=`"$projD\StdAfx.cpp`">"
A '      <PrecompiledHeader>Create</PrecompiledHeader>'
A '      <PrecompiledHeaderFile>StdAfx.h</PrecompiledHeaderFile>'
A '    </ClCompile>'
foreach($c in $cpps){
  A ('    <ClCompile Include="'+$c.FullName+'" />')
}
A '  </ItemGroup>'
A '  <ItemDefinitionGroup Condition="''$(Configuration)|$(Platform)''==''Release|x64''">'
A '    <Lib>'
A "      <OutputFile>$outLib</OutputFile>"
A '    </Lib>'
A '  </ItemDefinitionGroup>'
A '  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />'
A '</Project>'

$vcx = "$projD\ToolkitPro_x64.vcxproj"
[IO.File]::WriteAllText($vcx, $sb.ToString(), (New-Object System.Text.UTF8Encoding($false)))
"written: $vcx"
"outlib : $outLib"
