# Scene Structure

A scene represents a level in a game or a menu screen etc.
A scene consists of objects and controllers.
The objects represent the data that is present in the the scene while controllers define their behaviour.
The following sections describe how they work in detail.

## Objects

A scene object represents *something* that is present in the scene.
This can be the player character, an enemy, the text that displays the current score, a sound source, etc.
When you create a new object it just has name and nothing else.
You can define what the object actually represents by adding so called *components* to it.
Components are used to give objects a visual appearance or a position in space.
There are a number of predefined components that you can use to customize your objects.

**Predefined Components**

- @{ovis.core.Transform|Transform} will give the object a position in space as well as a rotation and scaling.
- @{ovis.rendering2d.Sprite|Sprite} will attach a 2D image to the object.
- @{ovis.physics2d.RigidBody2D|RigidBody2D} will give the object physical properties like velocity and damping used by the physics simulation.
- @{ovis.physics2d.RigidBody2DFixture|RigidBody2DFixture} will define the shape of the object for collision detection.

In addition to components scene objects can also have other child objects so it forms a hierarchy.
Scene objects can have multiple child objects but each child object can only have a single parent object.
Objects that to not have a parent are called *root objects*.
Creating child objects is useful to logically group different objects.
E.g., if you have a car you probably want to separate the wheels and the chassis in order to be able to rotate the wheels without rotating the chassis.
You can achieve this by making all wheels and the chassis individual root objects.
However, now there is no relationship between the different objects.
If you change the position of the chassis, the tires will stay in place an vice versa.
Instead, you could make the wheels childs objects of the chassis.
Now their position will be relative to the chassis and when you move the chassis the wheels will move accordingly.

## Controllers
