param(
	[Parameter(Mandatory = $true)]
	[ValidatePattern('^v\d+\.\d+\.\d+$')]
	[string]$Version,

	[string]$Branch = "main",

	[switch]$NoBuild,

	[switch]$NoPush,

	[switch]$IncludeAllChanges
)

$ErrorActionPreference = "Stop"

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

Write-Host "[1/6] Checking git working tree..."
if (-not $IncludeAllChanges) {
	Require-CleanTree
} else {
	Write-Host "IncludeAllChanges enabled: release commit will include current working tree changes."
}

Write-Host "[2/6] Validating version tag..."
Require-TagAbsent $Version

Write-Host "[3/6] Updating pxt.json version to $Version..."
$pxtPath = Join-Path $PSScriptRoot "pxt.json"
if (-not (Test-Path $pxtPath)) {
	Write-Error "pxt.json not found at $pxtPath"
}

$pxt = Get-Content -Raw -Path $pxtPath | ConvertFrom-Json
$pxt.version = $Version.TrimStart('v')
$pxt | ConvertTo-Json -Depth 20 | Set-Content -Path $pxtPath -Encoding UTF8

Write-Host "[4/6] Building package..."
if (-not $NoBuild) {
	pxt build
}

Write-Host "[5/6] Creating release commit..."
if ($IncludeAllChanges) {
	git add -A
} else {
	git add pxt.json
}
$commitMsg = "Release $Version"
git commit -m $commitMsg

Write-Host "[6/6] Tagging and pushing..."
git tag -a $Version -m "Release $Version"

if (-not $NoPush) {
	git push origin $Branch
	git push origin $Version
}

Write-Host "Release complete: $Version" -ForegroundColor Green
