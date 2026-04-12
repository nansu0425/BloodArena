# Phase 1: Navigate 3D Space and Render Objects

## Work Order

Tasks are ordered by dependency chain. Each task requires the previous task's result to verify.

```
[1. Math Extensions] --> [2. Camera + Input] --> [3. 3D Mesh + Depth] --> [4. Model Loading] --> [5. Textures] --> [6. Lighting] --> [7. Shadows]
```

### Task 1: Extend math library for 3D transformations

- **Depends on:** Nothing
- **Why first:** Every subsequent task depends on math utilities (view/projection builders, vector operations, full rotation). Also unifies math type naming to Float-based style (Float3, Float4, Float4x4) matching HLSL conventions.

### Task 2: Camera system with keyboard/mouse input and delta time

- **Depends on:** Task 1
- **Why this order:** View and projection matrices from Task 1 are needed to build the camera. 3D space must be navigable before any subsequent rendering results can be verified.

### Task 3: 3D mesh rendering with depth buffer

- **Depends on:** Task 2
- **Why this order:** Camera with perspective projection must exist to see 3D geometry. Depth testing and 3D geometry are tightly coupled — neither can be verified without the other.

### Task 4: External 3D model loading

- **Depends on:** Task 3
- **Why this order:** The mesh system (vertex/index buffers, normals in vertex format) must exist before external models can be loaded into it.

### Task 5: Texture mapping

- **Depends on:** Task 4
- **Why this order:** The model loader provides UV coordinates needed for texture mapping. The mesh system must support the full vertex layout before textures can be applied.

### Task 6: Directional lighting

- **Depends on:** Task 3 (normals in vertex data), Task 5 (textured output)
- **Why this order:** Normals and textured surfaces must exist to properly verify lighting. Light/dark gradients are much clearer on textured models than flat-colored ones.

### Task 7: Shadow mapping

- **Depends on:** Task 6
- **Why this order:** Directional light must exist before shadows can be verified. This is the most complex single feature and builds on depth buffer knowledge from Task 3 and light direction from Task 6.
