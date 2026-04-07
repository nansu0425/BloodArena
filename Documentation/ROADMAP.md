# BloodArena Project Roadmap

## Overview

A third-person arena wave survival game with a medieval fantasy gladiator theme.
Purpose: Learn core game programming systems + build a game programmer portfolio.

---

## Phase 0: Place and inspect objects in the editor

1. A colored shape must be visible on screen.
2. Game objects must be creatable and deletable at runtime.
3. Each object must have position, rotation, and scale.
4. A debug UI must be displayed over the game screen.
5. Objects must be selectable by clicking on them in the viewport.
6. Selected object properties must be viewable and editable.
7. Editor features must not be included in the final game build.

---

## Phase 1: Navigate 3D space and render objects

1. The camera must be freely movable and rotatable with keyboard and mouse.
2. Movement speed must be independent of frame rate.
3. Multiple 3D objects must render at their respective positions.
4. External 3D model files must be importable and renderable.
5. Objects must have textures applied.
6. Object faces must appear brighter or darker depending on light direction.
7. Objects must cast shadows onto other objects.
8. Closer objects must occlude farther objects.

---

## Phase 2: Audiovisual effects exist

1. Particles must have a creation, lifetime, and destruction cycle.
2. Particle position, velocity, size, and color must be controllable.
3. Decals must be projectable onto surfaces.
4. Post-processing effects must be applicable to the rendered image.
5. Sound files must be loadable and playable.
6. Sounds must convey direction and distance based on their 3D position.
7. Visual effects must be able to trigger sounds in sync.

---

## Phase 3: Characters animate

1. External animation data must be importable.
2. Characters must play skeletal animations.
3. Animations must transition based on state (idle, move, attack, etc.).
4. Animation transitions must blend smoothly.
5. Audiovisual effects must be triggerable at specific animation timings.

---

## Phase 4: The player moves in the arena

1. The player must move relative to camera direction.
2. The camera must follow the player in third person.
3. The player and camera must not pass through walls or obstacles.
4. Gravity must be applied to the player.
5. Player movement must have audiovisual effects.
6. Play mode must be enterable from the editor and returnable back to the editor.

---

## Phase 5: Progression and items exist

1. The player must have a level and stats.
2. The player must level up when enough experience is accumulated.
3. Player stats must change based on level.
4. Items must be acquirable and usable.
5. Items must affect player abilities.

---

## Phase 6: Combat is possible

1. Both the player and enemies must have health.
2. The player must be able to attack enemies and deal damage.
3. Enemies must chase the player when detected and attack when in range.
4. Entities must die when health reaches zero.
5. The player must be able to use skills.
6. Attacks and hits must have audiovisual effects.
7. Killing enemies must grant experience.
8. Enemies must be able to drop items on death.

---

## Phase 7: The wave survival game is playable

1. Enemies must appear in waves.
2. The next wave must start when all enemies in the current wave are defeated.
3. Difficulty must increase as waves progress.
4. The game must end when the player dies.
5. Health and current wave number must be displayed on screen.
6. Background music must play.
7. The game must be startable from a title screen.
8. Audiovisual effects must exist to convey game flow.

---

## Phase Order Rationale

Each phase follows a dependency chain where results cannot be verified without the previous phase.

- **0 → 1**: Objects and the editor must exist to place and verify 3D rendering results.
- **1 → 2**: 3D rendering must exist to verify particles and decals on screen.
- **2 → 3**: Audiovisual effects must exist to link with animation events.
- **3 → 4**: Animations must exist to show a character walking around.
- **4 → 5**: The player must move to experience stat and item effects.
- **5 → 6**: Stats and items must exist for combat damage calculations to be meaningful.
- **6 → 7**: Combat and progression must exist for wave survival to work.
