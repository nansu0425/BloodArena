---
name: commit-message
description: Analyze staged git changes and generate a commit message following Conventional Commits format
disable-model-invocation: true
argument-hint: "[type] [description]"
---

# Conventional Commit Message Generator

Analyze staged git changes and generate a commit message in Conventional Commits format.

## Instructions

1. Run the following commands to inspect changes:
   - `git diff --cached --stat` — list of changed files and stats
   - `git diff --cached` — detailed diff
   - `git log --oneline -10` — recent commit style reference

2. If there are no staged changes, notify the user and stop.

3. Analyze the changes and determine the following:
   - **Type**: the most appropriate among `feat`, `fix`, `docs`, `refactor`, `chore`, `perf`, `test`, `ci`, `build`, `style`
   - **Scope** (optional): inferred from the changed files/modules (e.g., `renderer`, `input`, `build`)
   - **Subject**: a one-line summary of the changes
   - **Body** (optional): include only when additional context on the reason or details is needed

4. If `$ARGUMENTS` is provided, use it as a hint for type or description.

## Commit Message Format

```
<type>(<scope>): <subject>

[optional body]
```

## Rules

- Commit messages **must** be written in English
- Subject line must be 50 characters or fewer
- Use imperative mood: "add feature" (O), "added feature" (X)
- Start subject with a lowercase letter
- No period at the end of the subject
- Wrap body at 72 characters
- Body should explain "why", not "what"

## Output

- Output the final commit message in a code block
- **Do not run git commit** — only suggest the message
- If multiple messages are possible, suggest only the single best one

## Type Reference

| Type       | Description                                      |
|------------|--------------------------------------------------|
| `feat`     | Add a new feature                                |
| `fix`      | Fix a bug                                        |
| `docs`     | Documentation changes                            |
| `style`    | Code style (formatting, semicolons, etc.)        |
| `refactor` | Code refactoring with no behavior change         |
| `perf`     | Performance improvement                          |
| `test`     | Add or modify tests                              |
| `chore`    | Build, dependencies, tool configuration          |
| `ci`       | CI/CD configuration changes                      |
| `build`    | Build system changes                             |
