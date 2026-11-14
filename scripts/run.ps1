# Build the ndnSIM image if needed, then compile and run scenarios/wired-topology.cc.
# Extra simulator arguments are forwarded, for example:
#   .\scripts\run.ps1 --frequency=20 --csSize=50

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root
New-Item -ItemType Directory -Force -Path (Join-Path $root "results") | Out-Null

$image = docker images -q icn-ndnsim:local
if (-not $image) {
    Write-Host "Building the ndnSIM image. The first build clones NS-3 and ndnSIM and compiles them. Expect 30-60 minutes."
    docker compose build
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

$extra = $args -join " "
if ($extra) {
    docker compose run --rm ndnsim ./waf --run "wired-topology --results=/results $extra"
} else {
    docker compose run --rm ndnsim
}
exit $LASTEXITCODE
