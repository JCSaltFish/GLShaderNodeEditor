# OpenGL Shader Node Editor

![Thumb](https://github.com/JCSaltFish/GLShaderNodeEditor/blob/master/doc/thumb.png)

This is an OpenGL shader node editor based on [Dear ImGui](https://github.com/ocornut/imgui) and [ImNodes](https://github.com/Nelarius/imnodes), which deals with GLSL shaders, including dynamic creation, linking, uniforms setting, and buffers binding in runtime via GUI operations. Basically it covers most of the C++ OpenGL client coding throughout the rendering pipeline.

* #### Programs
  ![Program](https://github.com/JCSaltFish/GLShaderNodeEditor/blob/master/doc/program.png)  
  A program can link multiple shaders for a rendering pipeline.

* #### Framebuffers
  ![Framebuffer](https://github.com/JCSaltFish/GLShaderNodeEditor/blob/master/doc/framebuffer.png)

* #### Textures
  ![Texture](https://github.com/JCSaltFish/GLShaderNodeEditor/blob/master/doc/texture.png)  
  Visualization of OpenGL texture objects.

* #### Nodes
  * ##### Event Nodes  
    ![EventNodes](https://github.com/JCSaltFish/GLShaderNodeEditor/blob/master/doc/eventnodes.png)
    * On Init  
      Will be executed once whenever the "start" or "restart" command is sent.
    * On Frame  
      Will be executed every frame during "running" state.
  
  * ##### Program Node  
    * Attributes  
      ![ProgramNodeAttribsArray](https://github.com/JCSaltFish/GLShaderNodeEditor/blob/master/doc/prognodeattribsarr.png)
      ![ProgramNodeAttribsCompute](https://github.com/JCSaltFish/GLShaderNodeEditor/blob/master/doc/prognodeattribscomp.png)  
      * Dispatch Type: `glDrawArray()` or `glDispatchCompute()`
      * Framebuffer: screen or user created framebuffer
      * Draw Mode: 1st param of `glDrawArray()`
      * Size: 3rd param of `glDrawArray()` (The 2nd param will always be 0)
      * Work Group Size X/Y/Z: params of `glDispatchCompute()`  
      
    ![ProgramNode](https://github.com/JCSaltFish/GLShaderNodeEditor/blob/master/doc/prognode.png)  
    * Node Inputs  
      * Flow In
      * Uniforms
      * Uniform Blocks
      * Shader Storage Buffers
    * Node Outputs  
      * Flow Out
      * Image Uniforms
      * Shader Storage Buffers
      * Framebuffer Attachments
   
   * ##### Block Node  
     ![BlockNode](https://github.com/JCSaltFish/GLShaderNodeEditor/blob/master/doc/blocknode.png)  
     Can be either a Uniform Block or a Shader Storage Buffer Block.  
     ![BlockNodeAttribs](https://github.com/JCSaltFish/GLShaderNodeEditor/blob/master/doc/blocknodeattribs.png)  
     Only block pins with same sizes can be linked.
   
   * ##### Texture Node  
     ![TextureNode](https://github.com/JCSaltFish/GLShaderNodeEditor/blob/master/doc/texturenode.png)
   
   * ##### Image Node  
     ![ImageNode](https://github.com/JCSaltFish/GLShaderNodeEditor/blob/master/doc/imagenode.png)
     * Node Inputs  
       * Texture Object
     * Node Outputs  
       * Image Object
       * Texture Object
   
   * ##### Ping-pong Node  
     ![PingPongNode](https://github.com/JCSaltFish/GLShaderNodeEditor/blob/master/doc/pingpongnode.png)  
     Can process Image Objects or Shader Storage Buffers
   
   * ##### Time Node
     ![TimeNode](https://github.com/JCSaltFish/GLShaderNodeEditor/blob/master/doc/timenode.png)
   
   * ##### Mouse Position Node
     ![MousePosNode](https://github.com/JCSaltFish/GLShaderNodeEditor/blob/master/doc/mouseposnode.png)

##
### Integration with any OpenGL Project
* You can find a simple example OpenGL application in _main.cpp_.

##
### Other libraries used:
* OpenGL Mathematics: https://github.com/g-truc/glm
* tiny file dialogs: https://sourceforge.net/projects/tinyfiledialogs/
* stb: https://github.com/nothings/stb
* Source Sans: https://github.com/adobe-fonts/source-sans
* Fork Awesome: https://github.com/ForkAwesome/Fork-Awesome
