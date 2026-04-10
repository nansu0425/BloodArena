# /spec-implement $ARGUMENTS

Parse spec path and task number from: $ARGUMENTS
The last token is the task number. Everything before it is the spec path relative to `Documentation/Specs/`.

Examples:
- `/spec-implement Scene/game-object 1`
- `/spec-implement Editor/click-selection 3`

Spec directory: `Documentation/Specs/{spec-path}/`

## Instructions

1. **Read the full spec**:
   - `Documentation/Specs/{spec-path}/requirements.md`
   - `Documentation/Specs/{spec-path}/design.md`
   - `Documentation/Specs/{spec-path}/tasks.md`
   - If any file is missing, inform the user and stop.

2. **Locate the target task** in tasks.md.
   - If the task does not exist, list available tasks and stop.
   - If prerequisite tasks are not marked complete, warn the user and stop.

3. **Confirm understanding**: State what you will implement, which files you will touch, and the acceptance criteria. Wait for user confirmation before writing code.

4. **Implement**: Write code faithful to design.md.
   - Follow all CLAUDE.md coding conventions.
   - Implement only what the task specifies.
   - Do not add features, optimizations, or abstractions not in the spec.

5. **Report completion**: Summarize what was implemented and which files were changed.
   - Do not run build commands — the user handles builds.
   - Do not mark the task complete — the user decides when it passes.

## Escalation Protocol

When a conflict with the spec is discovered during implementation, classify it:

### Stop and report to the user
- Requirements themselves need to change
- Architecture-level design change is needed
- The change affects other tasks

### Record as implementation note and continue
- Internal implementation detail adjustments within the task
- Local decisions that do not affect other tasks
- Record these in tasks.md under the task as "Implementation Notes"

## Rules

- Implement only the specified task. Do not touch other tasks.
- Do not add features not specified in the spec.
- Follow the escalation protocol when conflicts are discovered.
- Ask questions instead of guessing when the design is unclear.