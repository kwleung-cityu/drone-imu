# Release Checklist (MakeCode Extension)

Use this for publishing a new extension version that MakeCode can reliably fetch.

## One-command release

Run from repository root:

```powershell
./release.ps1 v1.0.32
```

What it does:

1. Verifies working tree is clean.
2. Verifies tag does not already exist.
3. Updates `pxt.json` version to `1.0.32`.
4. Runs `pxt build`.
5. Commits `pxt.json` with message `Release v1.0.32`.
6. Creates annotated tag `v1.0.32`.
7. Pushes the current branch and `v1.0.32` to `origin`.

## Optional flags

- Skip build:

```powershell
./release.ps1 v1.0.32 -NoBuild
```

- Create commit/tag only (no push):

```powershell
./release.ps1 v1.0.32 -NoPush
```

- Use branch other than main:

```powershell
./release.ps1 v1.0.32 -Branch feature/my-branch
```

- Include intentional pending edits in the release commit:

```powershell
./release.ps1 v1.0.32 -IncludeAllChanges
```

## Manual fallback flow

```powershell
pxt build
git add -A
git commit -m "Release v1.0.32"
git push origin main
git tag -a v1.0.32 -m "Release v1.0.32"
git push origin v1.0.32
```

## Notes

- Use a new tag for every release users should consume.
- Do not reuse or move an existing tag.
- The script now writes `pxt.json` as UTF-8 without BOM for MakeCode compatibility.
- The script stops immediately if `pxt build`, `git commit`, or push commands fail.
- After publishing, test in MakeCode with:
  - `github:kwleung-cityu/drone-imu#v1.0.32`
