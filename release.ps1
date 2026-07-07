param(
	[Parameter(Mandatory = $true)]
	[ValidatePattern('^v\d+\.\d+\.\d+$')]
	[string]$Version,

	[string]$Branch = "",

	[switch]$NoBuild,

	[switch]$NoPush,

	[switch]$IncludeAllChanges
)

$ErrorActionPreference = "Stop"

function Invoke-CheckedCommand([string]$label, [scriptblock]$command) {
	& $command
	if ($LASTEXITCODE -ne 0) {
		throw "$label failed with exit code $LASTEXITCODE"
	}
}

function Write-JsonNoBom([string]$path, $obj) {
	$json = $obj | ConvertTo-Json -Depth 20
	$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
	[System.IO.File]::WriteAllText($path, $json, $utf8NoBom)
}

function Require-CleanTree {
	$status = git status --porcelain
	if ($status) {
		Write-Host "Pending changes:" -ForegroundColor Yellow
		$status | ForEach-Object { Write-Host "  $_" }
		Write-Error "Working tree is not clean. Commit or stash changes before releasing."
	}
}

function Require-TagAbsent([string]$tag) {
	git rev-parse --verify --quiet "refs/tags/$tag" | Out-Null
	if ($LASTEXITCODE -eq 0) {
		Write-Error "Tag '$tag' already exists locally. Use a new version."
	}
}

function Resolve-Branch([string]$branchArg) {
	if ($branchArg) {
		return $branchArg
	}

	$detected = (git branch --show-current).Trim()
	if (-not $detected) {
		Write-Error "Unable to determine current branch. Pass -Branch explicitly."
	}
	return $detected
}

Write-Host "[1/6] Checking git working tree..."
if (-not $IncludeAllChanges) {
	Require-CleanTree
} else {
	Write-Host "IncludeAllChanges enabled: release commit will include current working tree changes."
}

Write-Host "[2/6] Validating version tag..."
Require-TagAbsent $Version

$resolvedBranch = Resolve-Branch $Branch
Write-Host "Using branch: $resolvedBranch"

Write-Host "[3/6] Updating pxt.json version to $Version..."
$pxtPath = Join-Path $PSScriptRoot "pxt.json"
if (-not (Test-Path $pxtPath)) {
	Write-Error "pxt.json not found at $pxtPath"
}

$pxt = Get-Content -Raw -Path $pxtPath | ConvertFrom-Json
$pxt.version = $Version.TrimStart('v')
Write-JsonNoBom $pxtPath $pxt

Write-Host "[4/6] Building package..."
if (-not $NoBuild) {
	Invoke-CheckedCommand "pxt build" { pxt build }
}

Write-Host "[5/6] Creating release commit..."
if ($IncludeAllChanges) {
	Invoke-CheckedCommand "git add" { git add -A }
} else {
	Invoke-CheckedCommand "git add" { git add pxt.json }
}
$commitMsg = "Release $Version"
Invoke-CheckedCommand "git commit" { git commit -m $commitMsg }

Write-Host "[6/6] Tagging and pushing..."
Invoke-CheckedCommand "git tag" { git tag -a $Version -m "Release $Version" }

if (-not $NoPush) {
	Invoke-CheckedCommand "git push branch" { git push origin $resolvedBranch }
	Invoke-CheckedCommand "git push tag" { git push origin $Version }
}

Write-Host "Release complete: $Version" -ForegroundColor Green
