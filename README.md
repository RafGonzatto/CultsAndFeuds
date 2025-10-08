# Vazio (Unreal Engine)

This repo contains the source, config, and content for the Unreal Engine project `Vazio`.

What is tracked (keep in Git):

- Source/\*\* (C++ code)
- Config/\*\* (project settings)
- Content/\*\* (assets) — tracked via Git LFS
- Vazio.uproject

Ignored (auto-generated):

- Binaries/, Intermediate/, DerivedDataCache/, Saved/
- Visual Studio/IDE caches and build outputs

## Git setup (Windows PowerShell)

1. Install Git and Git LFS (only once per machine):

- https://git-scm.com/download/win
- https://git-lfs.com

2. Initialize repo (first time):

```powershell
# in project root
git init
git lfs install

# if pushing to a new GitHub repo
git remote add origin https://github.com/<your-user>/<your-repo>.git

# add and commit tracked files
git add .gitattributes .gitignore Vazio.uproject Config Content Source README.md
git commit -m "Initial Unreal project: code, config, content (LFS)"

# push
git branch -M main
git push -u origin main
```

3. Regular workflow:

```powershell
git add -A
git commit -m "Describe your change"
git push
```

Notes:

- Large binary assets (umap/uasset, media) are stored in Git LFS as configured in `.gitattributes`.
- Don’t commit Binaries/Intermediate/DerivedDataCache/Saved — they’re re-generated.
- If you clone on a new machine, run `git lfs install` once, then `git lfs pull` is automatic during checkout with recent Git.

## Logging configuration

The shared logging facade reads boolean flags and an optional level from `.env` at startup. Missing keys default to `false`, except `LOG_LEVEL`, which defaults to `INFO`.

```
LOG_ENABLED=true
LOG_UI=false
LOG_ENEMIES=true
LOG_WEAPONS=false
LOG_UPGRADES=false
LOG_MASS=false
LOG_LOOPS=true
LOG_LEVEL=DEBUG
```

- `LOG_ENABLED=false` silences every log regardless of category.
- Set individual `LOG_*` keys to `true` to allow that category.
- `LOG_LEVEL` accepts `TRACE`, `DEBUG`, `INFO`, `WARN`, `ERROR` (default `INFO`). Messages below the level are suppressed.

Include `Logging/VazioLogFacade.h` and use the helpers:

```cpp
LOG_ENEMIES(Info, TEXT("Spawned enemy %d"), EnemyId);
LOG_UI(Warn, TEXT("Missing widget %s"), *WidgetName);
LOG_LOOP_THROTTLE(Debug, FName(TEXT("EnemySpawnLoop")), 100, 1.0f, TEXT("Wave tick %d"), TickId);
```

Loop helpers mark output with `[LOOP]` and support throttling by count (`LOG_LOOP_EVERY`) or count + minimum seconds (`LOG_LOOP_THROTTLE`). Call `FVazioLog::FVazioLogFacade::ReloadConfig()` after editing `.env` to hot-reload the toggles at runtime.
