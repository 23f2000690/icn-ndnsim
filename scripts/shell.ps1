# Open a shell inside the simulator tree (/opt/ndnSIM/ns-3).
# The scenario sources are mounted at scratch/.

$ErrorActionPreference = "Stop"
Set-Location (Split-Path -Parent $PSScriptRoot)
docker compose run --rm --entrypoint bash ndnsim
exit $LASTEXITCODE
