# 为 gh0st 主控工程添加 Release|x64 配置（保留现有 Win32）
$ErrorActionPreference='Stop'
$root="C:\Users\Administrator\Desktop\本机研究原理"
$f=Join-Path $root 'gh0st\gh0st.vcxproj'

$raw=[IO.File]::ReadAllText($f)
$eol = if($raw.Contains("`r`n")){"`r`n"}else{"`n"}
$lines=New-Object 'System.Collections.Generic.List[string]'
foreach($l in ($raw -split "`r?`n")){ [void]$lines.Add($l) }

function Block($s){ [string[]]@($s -split "`r?`n" | Where-Object { $_ -ne '' }) }
function Find-Index($pred){ for($i=0;$i -lt $lines.Count;$i++){ if(& $pred $lines[$i]){ return $i } }; throw "anchor not found" }
function Insert-Before($idx,$blk){ $lines.InsertRange($idx,[string[]]@($blk)) }
function Insert-After($idx,$blk){ $lines.InsertRange($idx+1,[string[]]@($blk)) }

$pc = Block @'
    <ProjectConfiguration Include="Release|x64">
      <Configuration>Release</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
'@
$cfg = Block @'
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <PlatformToolset>v145</PlatformToolset>
    <UseOfMfc>Static</UseOfMfc>
    <CharacterSet>MultiByte</CharacterSet>
  </PropertyGroup>
'@
$psg = Block @'
  <ImportGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'" Label="PropertySheets">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
    <Import Project="$(VCTargetsPath)Microsoft.Cpp.UpgradeFromVC60.props" />
  </ImportGroup>
'@
$out = Block @'
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <OutDir>.\Release_x64\</OutDir>
    <IntDir>.\Release_x64\</IntDir>
    <LinkIncremental>false</LinkIncremental>
  </PropertyGroup>
'@
$idg = Block @'
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <ClCompile>
      <RuntimeLibrary>MultiThreaded</RuntimeLibrary>
      <InlineFunctionExpansion>Default</InlineFunctionExpansion>
      <StringPooling>true</StringPooling>
      <FunctionLevelLinking>true</FunctionLevelLinking>
      <Optimization>MaxSpeed</Optimization>
      <SuppressStartupBanner>true</SuppressStartupBanner>
      <WarningLevel>TurnOffAllWarnings</WarningLevel>
      <AdditionalIncludeDirectories>C:\Program Files (x86)\Codejock Software\MFC\Xtreme ToolkitPro v18.5.0\Source;$(ProjectDir);../common;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
      <PreprocessorDefinitions>WIN32;_WIN64;NDEBUG;_WINDOWS;_XTPLIB_NOAUTOLINK;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <AssemblerListingLocation>.\Release_x64\</AssemblerListingLocation>
      <PrecompiledHeaderOutputFile>.\Release_x64\gh0st.pch</PrecompiledHeaderOutputFile>
      <ObjectFileName>.\Release_x64\</ObjectFileName>
      <ProgramDataBaseFileName>.\Release_x64\</ProgramDataBaseFileName>
      <AdditionalOptions>/Zc:forScope- %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <Midl>
      <SuppressStartupBanner>true</SuppressStartupBanner>
      <PreprocessorDefinitions>NDEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <TypeLibraryName>.\Release_x64\gh0st.tlb</TypeLibraryName>
      <MkTypLibCompatible>true</MkTypLibCompatible>
      <TargetEnvironment>X64</TargetEnvironment>
    </Midl>
    <ResourceCompile>
      <Culture>0x0804</Culture>
      <PreprocessorDefinitions>NDEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
    </ResourceCompile>
    <Bscmake>
      <SuppressStartupBanner>true</SuppressStartupBanner>
      <OutputFile>.\Release_x64\gh0st.bsc</OutputFile>
    </Bscmake>
    <Link>
      <AdditionalLibraryDirectories>C:\Program Files (x86)\Codejock Software\MFC\Xtreme ToolkitPro v18.5.0\Lib\vc150_x64;%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
      <SuppressStartupBanner>true</SuppressStartupBanner>
      <SubSystem>Windows</SubSystem>
      <IgnoreSpecificDefaultLibraries>msvcrt.lib;%(IgnoreSpecificDefaultLibraries)</IgnoreSpecificDefaultLibraries>
      <OutputFile>../Bin/远控.exe</OutputFile>
      <AdditionalDependencies>../common/zlib/zlib_x64_mt.lib;ToolkitPro1850vc150S_x64.lib;SHLWAPI.LIB;nafxcw.lib;wininet.lib;%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
'@
$pch = Block @'
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Release|x64'">Create</PrecompiledHeader>
      <PrecompiledHeaderFile Condition="'$(Configuration)|$(Platform)'=='Release|x64'">stdafx.h</PrecompiledHeaderFile>
'@

# 从后往前插入
$pchIdx = Find-Index { param($l) $l.Contains('PrecompiledHeaderFile') -and $l.Contains('Release|Win32') }
Insert-After $pchIdx $pch

$idgEnd = Find-Index { param($l) $l.Contains('</ItemDefinitionGroup>') }
$nextItem=$idgEnd+1
while(-not $lines[$nextItem].Contains('<ItemGroup>')){ $nextItem++ }
Insert-Before $nextItem $idg

$outAnchor = Find-Index { param($l) $l.Contains('ItemDefinitionGroup') -and $l.Contains('Release|Win32') }
Insert-Before $outAnchor $out

$psgAnchor = Find-Index { param($l) $l.Contains('Label="UserMacros"') }
Insert-Before $psgAnchor $psg

$cfgAnchor = Find-Index { param($l) $l.Contains('Microsoft.Cpp.props') }
Insert-Before $cfgAnchor $cfg

$pcAnchor = Find-Index { param($l) $l.Trim() -eq '</ItemGroup>' }
Insert-Before $pcAnchor $pc

[IO.File]::WriteAllText($f,($lines -join $eol),(New-Object Text.UTF8Encoding($true)))
"gh0st x64 配置已写入"
