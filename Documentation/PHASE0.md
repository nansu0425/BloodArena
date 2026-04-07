# Phase 0: Place and Inspect Objects in the Editor

## Work Order

Tasks are ordered by dependency chain. Each task requires the previous task's result to verify.

```
[1. Colored Shape] --> [2. Create/Delete Objects] --> [3. Transform] --> [4. Debug UI] --> [5. Click Selection] --> [6. Property Editing] --> [7. Build Separation]
```

### Task 1: A colored shape must be visible on screen

- **Depends on:** Nothing (builds on existing DX11 renderer)
- **Why first:** Everything else needs rendering to verify results.

### Task 2: Game objects must be creatable and deletable at runtime

- **Depends on:** Task 1
- **Why this order:** Need rendering to confirm objects appear and disappear.

### Task 3: Each object must have position, rotation, and scale

- **Depends on:** Task 2
- **Why this order:** Objects must exist before they can have transforms.

### Task 4: A debug UI must be displayed over the game screen

- **Depends on:** Task 3
- **Why this order:** Object system with transforms should exist first so the UI can display real data. Avoids rework from designing UI before data structures are finalized.

### Task 5: Objects must be selectable by clicking on them in the viewport

- **Depends on:** Task 3 (objects with transforms), Task 4 (UI to show selection state)
- **Why this order:** Need transformed objects to pick from, and UI to confirm selection.

### Task 6: Selected object properties must be viewable and editable

- **Depends on:** Task 4 (debug UI), Task 5 (selection)
- **Why this order:** Need both a selected object and a UI to display/edit its properties.

### Task 7: Editor features must not be included in the final game build

- **Depends on:** Tasks 4, 5, 6
- **Why last:** All editor code must exist before we can define the boundary between editor and game. Doing this earlier risks repeated build config changes as the boundary shifts.
