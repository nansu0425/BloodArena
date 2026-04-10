# /spec-create $ARGUMENTS

Feature/system description: $ARGUMENTS (free-form name or description)
Examples: `/spec-create game-object`, `/spec-create "camera movement"`

The spec directory path (module and feature name) is determined during the design process, not from the argument.

## Spec Directory Structure

Specs are organized by module, mirroring the code structure under `Application/`.
Each module can contain multiple specs.

```
Documentation/Specs/
  {Module}/
    {feature}/
      requirements.md
      design.md
      tasks.md
```

## Instructions

Proceed through three phases sequentially. Each phase requires explicit user approval before advancing to the next.

### Phase 1: Requirements

1. Research the codebase and `Documentation/` for relevant context.
2. Ask the user clarifying questions about the feature's goals and constraints.
3. Determine which module this feature belongs to and what the spec directory should be named. Confirm with the user.
4. Draft `requirements.md` with these sections:
   - **Context**: Why this feature exists, which system it belongs to
   - **Functional Requirements**: What the feature must do (numbered list)
   - **Non-Functional Requirements**: Performance, maintainability constraints
   - **Out of Scope**: What this feature explicitly does not cover
   - **Dependencies**: What must exist before this feature (including other specs)
   - **Acceptance Criteria**: Observable conditions that prove the feature works
5. Present the draft to the user for review.
6. **GATE**: Wait for explicit user approval. Revise if requested. Do not proceed until approved.

### Phase 2: Design

1. Based on approved requirements, analyze the codebase for integration points.
2. Draft `design.md` with these sections:
   - **Architecture**: How the feature fits into the existing system
   - **Key Decisions**: Design choices with rationale
   - **Data Structures**: New types, modifications to existing types
   - **Interfaces**: Public APIs, function signatures
   - **Cross-Module Dependencies**: Other modules this feature interacts with and how
   - **File Plan**: Which files to create or modify
3. Present the draft to the user for review.
4. **GATE**: Wait for explicit user approval. Revise if requested. Do not proceed until approved.

### Phase 3: Tasks

1. Based on approved design, break down implementation into ordered tasks.
2. Draft `tasks.md` with these sections:
   - **Task List**: Numbered, ordered by dependency
   - Each task includes: description, files involved, acceptance criteria
   - Task dependencies are explicitly stated
   - Test cases corresponding to each acceptance criterion
3. Tasks should be small enough to implement and verify independently.
4. Present the draft to the user for review.
5. **GATE**: Wait for explicit user approval. Spec creation is complete when approved.

## Rules

- You are an information provider. The user makes all design decisions.
- Do not write or modify source code during spec creation.
- Ambiguous requirements must be clarified with questions, not assumptions.
- Cross-reference `Documentation/ROADMAP.md` and related specs in `Documentation/Specs/`.
- Respect existing codebase conventions and architecture.
- If the spec directory already exists, warn the user and ask whether to overwrite or update via `/spec-update`.