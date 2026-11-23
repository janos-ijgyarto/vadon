param(
    [parameter(Mandatory=$true)][string]$sharpmakeCommand
)

$scriptRootPath = (Split-Path -Parent $MyInvocation.MyCommand.Path)
Set-Location -Path $scriptRootPath

function Parse-Sharpmake-Config {
    param (
        [parameter(Mandatory=$true)][string]$scriptRootPath
    )
    $configPath = (Join-Path $scriptRootPath "sharpmake_config.json")

    $configJson = Get-Content $configPath -Raw | ConvertFrom-Json
    return $configJson
}

function Create-Debug-Solution {
    param (
        [parameter(Mandatory=$true)][string]$sharpmakeExePath
    )

    Write-Host "Generating Vadon debug solution"

    $sharpmakeArgs = @(
        "/sources(@'vadon_main.sharpmake.cs')",
        "/verbose",
        "/generateDebugSolution",
        "/debugSolutionPath(@'generated/debugsolution')",
        "/debugSolutionDevEnv('vs2022')",
        "/generateDebugSolutionOnly"
    )

    & $sharpmakeExePath $sharpmakeArgs
}

function Setup-Project {
    param (
        [parameter(Mandatory=$true)][string]$scriptRootPath,
        [parameter(Mandatory=$true)][string]$sharpmakeExePath
    )

    Write-Host "Generating Vadon projects"

    $userConfigPath = (Join-Path $scriptRootPath "vadon_user_config.json")

    $sharpmakeArgs = @(
        "/sources(@'vadon_main.sharpmake.cs')",
        "/vadonUserConfigPath(@'$userConfigPath')",
        "/logAllGeneratedSolutions"
    )

    & $sharpmakeExePath $sharpmakeArgs
}

$sharpmakeConfig = Parse-Sharpmake-Config -scriptRootPath $scriptRootPath

if($sharpmakeCommand -eq "generateDebugSolution") {
    Create-Debug-Solution -sharpmakeExePath $sharpmakeConfig.sharpmakeExe 
} elseif($sharpmakeCommand -eq "setup") {
    Setup-Project -scriptRootPath $scriptRootPath -sharpmakeExePath $sharpmakeConfig.sharpmakeExe
} else {
    Write-Host "Invalid command: $sharpmakeCommand"
    exit 1
}

exit 0