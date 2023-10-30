# Preprocessor
The new Preprocessor for the Ultra Engine entity component system. 

Simply call the Executable in your Pre-Build Event and the tool will generate a RegisterComponents.h file within your Source directory. This assumes you are making one component per file and the name of the class matches the hpp file. A blank JSON file for the editor will be generated if none exists. 

By default this assumes it's in the same location as your soultion. To set the working directory, use *+path* to set the location
```
$(SolutionDir)\Tools\Preprocessor.exe +path "path/to/project"
```
