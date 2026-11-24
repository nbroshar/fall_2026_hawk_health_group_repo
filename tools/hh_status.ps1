# hh_status.ps1 -- HawkHealth subsystem scoreboard.
#
# Run from your clone root (the folder that contains firmware/):
#     pwsh tools/hh_status.ps1
# or in Windows PowerShell:
#     powershell -ExecutionPolicy Bypass -File tools\hh_status.ps1
#
# It reads the source tree (no build needed) and reports, per subsystem:
#   Header  = a hh_*.h contract exists
#   InBuild = real code for it is compiled (its entry symbol is defined in a src .c)
#   Wired   = it is actually started in app_start()
# Status: RUNNING (in build and wired), NOT WIRED (code exists but not started),
#         MISSING (no implementation in the build yet).

param([string]$Root = ".")

$src = Join-Path $Root "firmware/src"
$inc = Join-Path $Root "firmware/include"
$appStart = Join-Path $src "app/app_start.c"

if (-not (Test-Path $src)) {
    Write-Host "Can't find firmware/src under '$Root'. Run this from your clone root (the folder with firmware/)." -ForegroundColor Red
    exit 1
}

# Subsystem -> contract header + the entry symbol that marks a real implementation / running task.
$subsystems = @(
    @{ Name = "Sensor";    Header = "hh_sensor_contract.h"; Entry = "SensorTask" }
    @{ Name = "Filter";    Header = "hh_filter.h";          Entry = "Filter_ProcessSample" }
    @{ Name = "Alert";     Header = "hh_alert.h";           Entry = "AlertTask" }
    @{ Name = "Watchdog";  Header = "hh_watchdog.h";        Entry = "Watchdog_Init" }
    @{ Name = "Telemetry"; Header = "hh_telemetry.h";       Entry = "TelemetryTask" }
    @{ Name = "Display";   Header = "hh_display.h";          Entry = "DisplayTask" }
    @{ Name = "Command";   Header = "hh_command_header.h";  Entry = "Command_TaskHandler" }
    @{ Name = "Logger";    Header = "hh_logger.h";          Entry = "LoggerTask" }
)

# All compiled .c files except app_start.c, so a mere xTaskCreate reference there
# does not count as "implemented".
$appFull = (Resolve-Path $appStart -ErrorAction SilentlyContinue).Path
$implFiles = Get-ChildItem -Path $src -Recurse -Filter *.c |
             Where-Object { $_.FullName -ne $appFull }
$appText = if (Test-Path $appStart) { Get-Content $appStart -Raw } else { "" }

$running = 0
$rows = foreach ($s in $subsystems) {
    $hasHeader = Test-Path (Join-Path $inc $s.Header)
    $hasImpl   = [bool]($implFiles | Select-String -SimpleMatch -Pattern $s.Entry -List)
    $isWired   = $appText -match [regex]::Escape($s.Entry)

    $status = if (-not $hasImpl) { "MISSING" }
              elseif (-not $isWired) { "NOT WIRED" }
              else { "RUNNING" }
    if ($status -eq "RUNNING") { $running++ }

    [pscustomobject]@{
        Subsystem = $s.Name
        Header    = if ($hasHeader) { "yes" } else { "--" }
        InBuild   = if ($hasImpl)   { "yes" } else { "--" }
        Wired     = if ($isWired)   { "yes" } else { "--" }
        Status    = $status
    }
}

Write-Host ""
Write-Host "HawkHealth subsystem scoreboard" -ForegroundColor Cyan
Write-Host ("-" * 58)
Write-Host ("{0,-11} {1,-7} {2,-8} {3,-6} {4}" -f "Subsystem","Header","InBuild","Wired","Status")
Write-Host ("-" * 58)
foreach ($r in $rows) {
    $color = switch ($r.Status) {
        "RUNNING"   { "Green" }
        "NOT WIRED" { "Yellow" }
        "MISSING"   { "Red" }
    }
    Write-Host ("{0,-11} {1,-7} {2,-8} {3,-6} {4}" -f $r.Subsystem,$r.Header,$r.InBuild,$r.Wired,$r.Status) -ForegroundColor $color
}
Write-Host ("-" * 58)
Write-Host ("{0} of {1} subsystems running." -f $running, $subsystems.Count)
if ($appText -match "SkeletonTask") {
    Write-Host "Note: the SkeletonTask seed is still created in app_start() (remove once real tasks are wired)." -ForegroundColor Yellow
}
Write-Host ""
