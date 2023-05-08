# Ovis Engine
A small game engine with strong focus on the Web platform which I developed in my spare time.
One of its main features is the inclusion of a custom visual programming language à la [Scratch](https://scratch.mit.edu/) which you can read about in my [blog post](https://soehrl.github.io/posts/visual-scripting/).

## Modules
* [ovis::utils](./utils): a general C++ utility library not necessarily specific to the engine.
* [ovis::vm](./vm): the virtual machine and parser for the custom visual scripting language. 
* [ovis::core](./core): the basic structure of the engine like the ECS implementation, event system and the job scheduler.
* [ovis::math](./math): (legacy) only contains marching cubes and heightmap implementations everything else was moved to [core](./core).  
* [ovis::input](./input): contains the event structures for the input events.
* [ovis::graphics](./graphics): a thin wrapper around OpenGL/WebGL.
* [ovis::rendering](./rendering): contains the basic rendering jobs.
* [ovis::rendering2d](./rendering2d): handles 2D rendering of shapes and text.
* [ovis::audio](./audio): (legacy) contained basic functionality for playing sound, currently not functional, needs rewrite.
* [ovis::networking](./networking): only contains the `fetch()` function for the web platform, all future networking code should go here.
* [ovis::sdl](./sdl): implements sdl window and events.
* [ovis::emscripten](./emscripten): contains the canvas viewport when compiling for web.
* [ovis::editor](./editor): exposes and implements features used for the web editor.
* [ovis::player](./player): executable that runs exported games on the website, currently not functional.
* [ovis::test](./test): a utility library for writing unit tests.



