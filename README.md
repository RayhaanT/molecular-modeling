# Molecular Modeling

This project builds 3-D models of chemicals based on their IUPAC name or chemical formula.
Formulae or chemical names can be typed into the console window, and a breakdown of the bond structure will be printed. 
Click and drag on the screen to rotate the model, and use the W/S keys or the scrollwheel to zoom. Holding E/Q will cause the model to start rotating automatically.
There are 3 available representations of the chemical: ball-and-stick, van der Waals spheres, and a custom electron orbit model. Representations are cycled with the R key.  
Simple molecular compounds as well as many saturated hydrocarbons are currently supported.  
  
## Demos

Rendering molecules of water (H2O) and carbon dioxide (CO2):  
<img src="Media/InorganicDemo.gif" width="600" height="450" alt="Inorganic demo gif missing" />

Models of organic compounds hexane and 1,2-dimethylcyclopropane:  
<img src="Media/OrganicDemo.gif" width="600" height="450" alt="Organic demo gif missing" />

## Tech
- [OpenGL](https://www.opengl.org/) - Graphics pipeline to render models
- [GLFW](https://www.glfw.org/) - OpenGL function library
- [GLAD](https://github.com/Dav1dde/glad) - OpenGL loading library, loads function pointers at runtime
- [GLM](https://glm.g-truc.net/0.9.8/index.html) - Math library

## Building
Using the GNU's C++ compiler (g++), the following command can be used to compile the source:  
```g++ -g src/*.cpp src/*.c -static -iquote include -Llib -lopengl32 -lglfw3 -lgdi32 -o VSEPR```  
Or, run buildAndRun.bat to compile and launch the executable  
```VSEPR-Modeling/buildAndRun.bat```

## Acknowledgements
- `VSEPR-Modeling/periodTableData.csv` is a derivative of Jeff Bigler's [Periodic Table spreadsheet](http://www.mrbigler.com/documents/Periodic-Table.xls) used under [CC BY-NC-SA 4.0](https://creativecommons.org/licenses/by-nc-sa/4.0/). It was reformatted and extra data was added.
- Sphere geometry generation sourced from Song Ho Ahn's [article](http://www.songho.ca/opengl/gl_sphere.html).
- Shaders and header files under `VSEPR-Modeling/include/OpenGLHeaders` are derivative of samples from Joey de Vrie's [OpenGL tutorial series](https://learnopengl.com/Introduction) used under [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/).
