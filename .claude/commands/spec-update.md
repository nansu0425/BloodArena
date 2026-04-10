# /spec-update $ARGUMENTS

Spec path: $ARGUMENTS (relative to `Documentation/Specs/`)

Examples:
- `/spec-update Scene/game-object`
- `/spec-update Editor/click-selection`

Spec directory: `Documentation/Specs/$ARGUMENTS/`

## Instructions

1. **Read current spec**: Read all files in `Documentation/Specs/$ARGUMENTS/`.
   - If the spec does not exist, inform the user and suggest `/spec-create`.

2. **Understand the change**: Ask the user what needs to change and why.

3. **Identify impact scope**: Analyze which spec documents and which source files are affected.
   - List affected spec documents (requirements.md, design.md, tasks.md)
   - List affected source files
   - List affected tests
   - List other specs that may be impacted (cross-module dependencies)

4. **Update spec documents**:
   - Update requirements.md if the change affects what the feature does.
   - Update design.md if the change affects how the feature works.
   - Update tasks.md if new tasks are needed or existing tasks change.
   - Present each changed document for user approval before saving.
   - **GATE**: Wait for user approval on spec changes before touching code.

5. **Propagate to code** (only after spec changes are approved):
   - List the code changes needed based on the spec diff.
   - Wait for user confirmation before applying.
   - Apply code changes following CLAUDE.md coding conventions.

## Rules

- Spec is always updated before code. This order must never be reversed.
- The user approves spec changes before code changes begin.
- If no code has been written yet for the affected parts, skip step 5.
- Do not modify code without updating the corresponding spec first.