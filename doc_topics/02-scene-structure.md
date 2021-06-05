# Scene Structure

A scene represents a level in a game or a menu screen etc.
A scene consists of objects and controllers.
The objects represent the data that is present in the the scene while controllers define their behaviour.
The following sections describe how they work in detail.

## Objects

An object represents *something* that is present in the scene.
This can be the player character, an enemy, the text that displays the current score, a sound source, etc.
When you create a new object it just has name and nothing else.
You can define what the object actually represents by adding so called *components* to it.

### Components
**Predefined Components**

- @{ovis.core.Transform|Transform} will give the object a position in space as well as a rotation and scaling.
- @{ovis.rendering2d.Sprite|Sprite} will attach a 2D image to the object.
- @{ovis.physics2d.RigidBody2D|RigidBody2D} will give the object physical properties like velocity and damping used by the physics simulation.
- @{ovis.physics2d.RigidBody2DFixture|RigidBody2DFixture} will define the shape of the object for collision detection.

## Controllers
