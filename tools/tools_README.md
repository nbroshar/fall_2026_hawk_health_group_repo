# tools/hh_status.ps1 — subsystem scoreboard

Prints a one-row-per-subsystem table showing how far each subsystem has gotten, by reading the source tree. No build required.

## Run it

Open PowerShell and go to the clone root (the folder that contains `firmware/`, not the `tools/` folder):

```powershell
cd C:\dev\hawkhealth_group
```

Then run it one of these ways.

**Recommended (works on locked-down / school laptops):** pipe the script into PowerShell so the execution-policy signing check does not apply:

```powershell
Get-Content .\tools\hh_status.ps1 -Raw | Invoke-Expression
```

**Or, if your account can set its own policy,** unblock the file once and allow local scripts, then run it normally:

```powershell
Unblock-File .\tools\hh_status.ps1
Set-ExecutionPolicy -Scope CurrentUser RemoteSigned   # answer Y
.\tools\hh_status.ps1
```

If that still errors with "not digitally signed," a group policy is enforcing the restriction and you cannot override it; use the `Invoke-Expression` line above.

> Run it from the clone root, not from inside `tools/`. The script looks for `firmware/` relative to where you run it.

## Reading the output

```
Subsystem   Header  InBuild  Wired  Status
Sensor      yes     yes      yes    RUNNING
Filter      yes     --       --     MISSING
...
2 of 8 subsystems running.
```

- **Header** — a `hh_<name>.h` contract exists for the subsystem.
- **InBuild** — real code is compiled in: the subsystem's entry function (its task, or `Watchdog_Init` / `Filter_ProcessSample`) is defined in a `.c`, not just declared in a header.
- **Wired** — that entry point is actually started in `app_start()`.
- **Status** — `RUNNING` (in the build and started, shown green), `NOT WIRED` (code exists but nothing starts it, yellow), `MISSING` (no implementation yet, red).

The bottom line counts how many are `RUNNING`, and it flags if the throwaway `SkeletonTask` seed is still created in `app_start()`.

## When to run it

Any time you want a quick picture of what is actually built versus what is just a header. It is handy at the start of a work session, before opening a pull request, and as a live check during team meetings. As people land their subsystems, red rows turn green.
