# Computer Animation Project

## Build Instructions
- If you've built in a previous environment already, empty the `gdextension/build` folder.
### Windows
- Open the `gdextension` folder in Visual Studio.
- Build as a CMAKE project.
### Linux
- `cd` to `gdextension/build`.
- Run `cmake ..` / `cmake .. -G Ninja`.
- Run `make` / `ninja`.
### Alternatives
- A Windows build can be performed without Visual Studio by passing `-DCMAKE_CXX_COMPILER="path/to/compiler"` to CMAKE.
