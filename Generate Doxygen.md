# Generate Doxygen

To generate Doxygen documentation, from the project root run the command

>`doxygen`

This will generate html inside `docs/html`. 

Open `index.html` to start browsing the documentation.

## Doxyfile

The Doxyfile is configured to look in `src` and `include` for code/documentation. To change the location, change the value on the right side of `INPUT                  = src include` inside the file.

> [!warning] This changes the settings for EVERYONE. Be responsible.

If the Doxyfile at any point needs to be replaced, delete the Doxyfile file and run from the repo root:

`doxygen -g <name>` where `<name>` is the desired name of the config file. If no name is provided, Doxyfile will be thee default.
