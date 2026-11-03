# Slug Generator

## Overview
Droplet Engine's Slug Generator is a development tool that preprocesses TrueType fonts for efficient GPU-based text
rendering.

The generator converts TrueType (.ttf) fonts into binary assets in a `.slug` format, containing the curve and band data
required by the GPU-based Slug rendering algorithm by Eric Lengyel.

## Usage
The generator has its own Premake file in `Premake/Devtools/`, and project files are generated along with the rest of
the engine. Ahead of a fresh start of the engine, run the generator before launching the engine to generate the needed
`.slug` files for text rendering.

The generator uses the following header-only dependencies:
- stb_truetype - For reading the TrueType font data and extracting the glyph outlines.
- GLM - For vector mathematics when processing Bézier curves.

 