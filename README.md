**This is a simple project designed to make it easier to create/test new shaders and meshes.**

I was trying to create a shader program to render text with OpenGL and having issues because no compiler errors were shown but I couldn't see the texture on the final visualization,
so I decided to create this simple project in order to make the process of creating new shader Programs/meshes easier.
<br>
I combined some parts from [Raylib](https://github.com/raysan5/raylib) and others from [threepp](https://github.com/markaren/threepp) after refactoring them a little bit,
panning and zoom are probably not 100% correct so I might change them soon, also will probably work on trying to render the Text now, I will study Raylib to see how they do it
and try to incorporate it on this project, after I might also add ImGui in order to make it easier to build Meshes/materials but not sure...also the text for now is from the
[glText library](https://github.com/vallentin/glText)

---

to build just use:
```bash
cmake -S . -B ./build
cmake --build ./build
```
on the root directory, CommonVariables.cmake just sets a cmake environment variable for some of the .cpp files for convenience...
after building with these commands the final executable will be on ./build/Debug/shaders_test.exe (from root directory)
  - the setup_window files handle GLFW window creation and have some Raylib functions in order to track inputs so we can set events.
  - shader.cpp has a simple class to create shader Programs, buffers, and draw calls
  - filepath.hpp has some simple functions to get shader file path from the assets relative position to the executable
  - math folder has classes to make it easier to do math with vectors, matrices, etc... I created two versions of Vector2 because Raylib uses a different struct,
  so there is RVec2 which is a simple vector for the Raylib related functions (events) but the main one is the Vector2 class which comes from threepp
  - cameras folder has the camera classes, they inherit from Object3D which is a class that holds information on how to update the Objects position, model matrices, etc...on the visualization

<br>

The camera is supposed to be used to update the projection matrix and modelViewMatrix on the shader, the modelViewMatrix is the matrix that is supposed to hold the information
of the camera view * object model, on the example for now I'm using only camera->matrixWorldInverse to represent the modelView on the shader, but if you want to add other
Object3D objects you should multiply it's matrix with the camera matrix and use that on the shader, or you can do the multiplication on the shader and update each value separetly...

```cpp
// Create some class that inherits from Object3D
class programMesh : public Object3D {}
...
// updates Objects modelView matrix by multiplying model worlds coordinate with matrix world
// inverse coordinates
programMesh->modelViewMatrix.multiplyMatrices(camera->matrixWorldInverse, *programMesh->matrixWorld);
programMesh->normalMatrix.getNormalMatrix(programMesh->modelViewMatrix);

// if you don't change the camera you are using you can update the projection only once 
// instead of every render cycle
program->set_glUniformMatrix4fv("projectionMatrix", camera->projectionMatrix);
// inside shader you should have:
// gl_Position = projectionMatrix * modelViewMatrix * vec4(your_objects_local_point_position, 1);
program->set_glUniformMatrix4fv("modelViewMatrix", programMesh->modelViewMatrix);
```
