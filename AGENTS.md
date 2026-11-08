# AGENTS.md

Instructions for AI coding agents working in this project. This is the cross-tool
entry point. Codex, Google Antigravity, and OpenCode read `AGENTS.md`. Other
compatible tools do too. Claude Code reads `CLAUDE.md`, which imports this file,
so there is a single source of truth.

Claude Code, Codex, and every other AI tool must not add AI attribution to
commits or pull requests, including AI `Co-Authored-By` trailers or generated-by
signatures. Preserve genuine human attribution. See
[Commit and PR attribution](blueprint/context/ai-interaction.md#commit-and-pr-attribution)
for optional tool settings.

## What this is

**Arduino Chess** - A minimalist chess game for Arduino Nano with ST7735 TFT display.

MVP chess program for human vs computer play with full chess rules, three difficulty levels (700/900/1100 Elo), joystick control, and random color selection. Built for the XC Slim Pad hardware configuration with strict time and budget constraints.

This project is built with the **AI Blueprint**, a workflow layer, not an
app skeleton. The workflow is defined by the local skills and context files below.

## Proportional engineering

Build for established requirements, not hypothetical scale, threats, or future
flexibility. Reuse existing code, the standard library, native platform features,
and installed dependencies before adding machinery.

- Unknown scale or extensibility defaults to the smaller reversible design. Do
  not infer enterprise, multi-tenant, hostile-user, or compliance requirements.
- Derive trust and data-integrity boundaries from actual reachability: untrusted
  input, auth/session/ownership, shared persisted data, destructive operations,
  payments, secrets, and sensitive data.
- Ask only when an unknown materially changes behavior, architecture, persisted
  data, interoperability, a real security boundary, or cost. Otherwise choose the
  simplest repository-native implementation.
- Add an abstraction, dependency, service, configuration surface, compatibility
  layer, or security mechanism only for a current requirement.
- Simplicity never removes real trust-boundary validation, data-loss prevention,
  accessibility, explicit security requirements, configured tests, or project rules.
- Stack-specific template standards apply only when the project uses that stack.

## Read these when relevant

- `blueprint/config.json` - deterministic project workflow settings
- `blueprint/workflow.md` - workflow reference for configuration, skills, dashboard activity, and checks
- `blueprint/context/project-overview.md` - the project's source of truth
- `blueprint/context/coding-standards.md` - read before changing code
- `blueprint/context/ai-interaction.md` - read when running the Blueprint workflow
- `blueprint/context/current-feature.md` - the one feature, fix, or rollback being built in this checkout

Reuse relevant context already loaded in the session. Claude Code imports only
this file; its Blueprint skills load the other files on demand.

## Workflow

Build one feature, fix, or rollback at a time, behind review gates. The feature
loop is `/feature` -> `/implement` -> `/check` -> `/audit current` ->
`/complete`; approve the Feature spec before Implement. Each command is a local
skill at `.agents/skills/<skill>/SKILL.md` or `.claude/skills/<skill>/SKILL.md`
that carries its own instructions. Claude Code and Google Antigravity use slash
commands such as `/feature`, Codex uses `$feature`, and other tools can follow
the matching `SKILL.md` when asked.

Skills: `adopt`, `audit`, `blueprint-doctor`, `blueprint-status`, `brief`,
`check`, `ci`, `complete`, `debug`, `discovery`, `explore`, `feature`, `fix`,
`implement`, `onboard`, `overview`, `prototype`, `release`, `rollback`, and
`tests`, plus the explicit-only `autopilot` and `continuous`.

- Keep one active work item per checkout. Parallel work needs its own clone or
  Git worktree and branch; never replace another worker's active spec.
- `blueprint/config.json` can make review or verification stricter and tune
  local workflow settings. It never grants permission to commit, merge, push,
  deploy, publish, send, delete data, waive a failing check, or accept a finding.
- Never create or edit `blueprint/.state/run.json` directly. Tracked skills
  record dashboard activity through the managed helper.

## Commands

PlatformIO embedded development for Arduino Nano (ATmega328P).

- Build: `pio run -e nanoatmega328`
- Upload: `pio run --target upload`
- Serial monitor: `pio device monitor` (115200 baud)
- Clean: `pio run --target clean`
- Test: `pio test -e native` (Unity tests on host)
- Verify: `pio run -e nanoatmega328 && pio test -e native` (build + tests)

## Commit messages

- Use conventional commit format: `type: Subject line`
- Subject after the prefix starts with a capital letter, like a sentence
- Examples:
  - ✓ `feat: Add joystick input handling`
  - ✓ `fix: Correct board rendering coordinates`
  - ✗ `feat: add joystick input handling`
- Types: `feat`, `fix`, `docs`, `refactor`, `test`, `chore`, `revert`

## Git workflow

- **Merging strategy:** All merges happen on the Git server (GitHub/GitLab), never locally
- **Squash merge is forbidden:** Preserve full commit history from feature branches
- **Local merge:** Only by explicit user request in the current conversation
- **Default workflow:** Push feature branch → Create pull request → Merge on server
- Feature branches are based on `main` and merged back to `main` via pull request
- Never force-push, rebase shared branches, or amend pushed commits
