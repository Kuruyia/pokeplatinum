# Map documentation

This section of the documentation aims to provide an overview of the different
map-related subsystems, the interaction between them, and how they load their
data from the ROM so that, in the end, the game can seamlessly render a world
in which the player evolves.

## Glossary

- **Area**: a collection of maps that load the same set of textures and map prop
  models.
- **BDHC**: unknown acronym, contains data used to calculate the height of the
  terrain. Used for collision detection and the camera's height, for example.
- **Map**: a 2D grid of 32x32 tiles that form part of the game's world.
- **Map header**: a structure that contains metadata about a map, such as its
  music, its name, whether cycling is allowed, etc. Each map is associated with
  a map header, and has a specific, well-known ID.
- **Map matrix**: a 2D array of maps that form a playable area. This system allows
  the game to dynamically load and unload maps as the player moves around the world.
  The overworld is one such example of a map matrix, containing 30x30 maps.
- **Map prop**: a 3D model that is placed on the map. An example is the various
  buildings in the game.
- **NARC**: Nitro ARChive, a file format used by Nintendo DS games to store files.
  (think archive as in a `.tar` file, a file that contains files)
- **NSBMD**: unknown acronym, a file format used to store 3D models and textures for
  the Nintendo DS. See [apicula's documentation](https://raw.githubusercontent.com/scurest/nsbmd_docs/master/nsbmd_docs.txt)
  for more information.
