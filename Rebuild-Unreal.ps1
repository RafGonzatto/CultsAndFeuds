param(
  [string]$ProjectRoot = ".",
  [string]$EngineRoot  = "C:\Program Files\Epic Games\UE_5.6",
  [switch]$OpenEditor,
  [switch]$FixModes,
  [switch]$Build,                 # compila só se passar -Build
  [switch]$NoClean,               # não limpar (apenas locks)
  [string]$Config   = "Development",
  [string]$Platform = "Win64"
)

$ErrorActionPreference = "Stop"
function Say($m){ Write-Host "==> $m" -ForegroundColor Cyan }

# 1) Raiz e .uproject
$ProjectRoot = (Resolve-Path $ProjectRoot).Path
$uproject = Get-ChildItem $ProjectRoot -Filter *.uproject | Select-Object -First 1
if(-not $uproject){ throw "Nenhum .uproject encontrado em $ProjectRoot" }
$uprojectPath = $uproject.FullName
$projName = [IO.Path]::GetFileNameWithoutExtension($uproject.Name)
$Target = "${projName}Editor"

# 2) Ferramentas
$UBT      = Join-Path $EngineRoot "Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll"
$BuildBat = Join-Path $EngineRoot "Engine\Build\BatchFiles\Build.bat"
$UVS      = Join-Path $EngineRoot "Engine\Binaries\Win64\UnrealVersionSelector.exe"
$EditorExe= Join-Path $EngineRoot "Engine\Binaries\Win64\UnrealEditor.exe"

# 3) Matar processos zumbis (inclui Live Coding/UBT/Uba)
Say "Matando processos UE/LiveCoding..."
$procNames = @("UnrealEditor","UnrealEditor-Cmd","LiveCodingConsole","LiveCoding","UHT","UnrealBuildTool","ShaderCompileWorker","UbaServer","UbaAgent")
foreach($n in $procNames){
  Get-Process -Name $n -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue
}
Start-Sleep -Milliseconds 300

# 4) Corrigir pasta Modes (opcional)
if($FixModes){
  $moduleRoot = Join-Path $ProjectRoot ("Source\" + $projName)
  $modesDir   = Join-Path $moduleRoot "Modes"
  if(Test-Path $modesDir){
    Say "Movendo arquivos de 'Modes'..."
    $pubDir = Join-Path $moduleRoot "Public"
    if(-not (Test-Path $pubDir)){ $pubDir = $moduleRoot }
    $priDir = Join-Path $moduleRoot "Private"
    if(-not (Test-Path $priDir)){ $priDir = $moduleRoot }

    Get-ChildItem $modesDir -Filter *.h   -Recurse -ErrorAction SilentlyContinue | ForEach-Object {
      Move-Item -LiteralPath $_.FullName -Destination (Join-Path $pubDir $_.Name) -Force
    }
    Get-ChildItem $modesDir -Filter *.cpp -Recurse -ErrorAction SilentlyContinue | ForEach-Object {
      Move-Item -LiteralPath $_.FullName -Destination (Join-Path $priDir $_.Name) -Force
    }
    if((Get-ChildItem $modesDir -Recurse -ErrorAction SilentlyContinue | Measure-Object).Count -eq 0){
      Remove-Item $modesDir -Force -Recurse -ErrorAction SilentlyContinue
    }
  }
}

# 5) Limpeza (padrão) + locks de LiveCoding
if(-not $NoClean){
  Say "Limpando Binaries/, Intermediate/, .vs/, Saved/Temp, DerivedDataCache/..."
  foreach($d in @("Binaries","Intermediate",".vs","Saved\Temp","DerivedDataCache")){
    $p = Join-Path $ProjectRoot $d
    if(Test-Path $p){ Remove-Item $p -Recurse -Force -ErrorAction SilentlyContinue }
  }
}else{
  Say "Sem limpeza pesada (-NoClean)."
}
# locks específicos mesmo com -NoClean
Remove-Item (Join-Path $ProjectRoot "Saved\LiveCoding")                 -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item (Join-Path $ProjectRoot "Saved\HotReload")                  -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item (Join-Path $ProjectRoot "Intermediate\LiveCoding*")         -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item (Join-Path $ProjectRoot "Intermediate\Build\Win64\*.hobj")  -Force   -ErrorAction SilentlyContinue

# 6) Duplicatas .cpp por nome-base
Say "Verificando duplicatas de .cpp por nome-base..."
$srcDir = Join-Path $ProjectRoot "Source"
$dups = Get-ChildItem $srcDir -Recurse -Include *.cpp -ErrorAction SilentlyContinue |
  Group-Object { $_.BaseName } | Where-Object { $_.Count -gt 1 }
if($dups){
  Write-Host "Duplicatas encontradas:" -ForegroundColor Yellow
  foreach($g in $dups){
    Write-Host (" - {0}" -f $g.Name) -ForegroundColor Yellow
    $g.Group | ForEach-Object { Write-Host ("     {0}" -f $_.FullName) }
  }
  throw "Resolva as duplicatas acima e rode novamente."
}

# 7) Regenerar Project Files (sem compilar)
if(Test-Path $UVS){
  Say "Gerando Project Files (UnrealVersionSelector)..."
  & "$UVS" /projectfiles "$uprojectPath"
}else{
  Say "Gerando Project Files (UBT -ProjectFiles)..."
  & dotnet "$UBT" -ProjectFiles -project="$uprojectPath" -game -engine -progress
}

# 8) Compilar (apenas se -Build)
if($Build){
  Say "Compilando $Target $Platform $Config..."
  & "$BuildBat" $Target $Platform $Config -Project="$uprojectPath" -WaitMutex -FromMsBuild -architecture=x64
}else{
  Say "Sem build (use -Build para compilar)."
}

# 9) Abrir Editor (opcional)
if($OpenEditor){
  Say "Abrindo Unreal Editor..."
  Start-Process "$EditorExe" -ArgumentList "`"$uprojectPath`""
}

Say "OK ✅ Projeto: $projName"
