# Skybox

[Skybox](https://learnopengl.com/Advanced-OpenGL/Cubemaps) is a technique used to fake a distant view with 2D textures.
Generally, those textures are stored in a [cube map](https://www.khronos.org/opengl/wiki/Cubemap_Texture).
A cube map consists of 6 textures, and those textures together form a cube.
The distant view (mostly the sky) is rendered to the interior of the cube.
That's why it is called skybox.

To make a skybox realistic, the view must be distant enough so that even the viewer moves, it barely change.
This is the same as our experience in the real world, when we walk toward a sky.

# Result

![result](./result.png)
