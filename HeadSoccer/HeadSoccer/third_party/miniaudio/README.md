# miniaudio

Bundled upstream version: **0.11.22**.

- Source: https://github.com/mackron/miniaudio/tree/0.11.22
- Header: https://raw.githubusercontent.com/mackron/miniaudio/0.11.22/miniaudio.h
- License: public domain or MIT-0; original `LICENSE` is included.
- Original header SHA-256: `9019743287e443c55e5737a7297f38e5e358561701d6db2d905afb114390c410`

The implementation is compiled once in `src/Miniaudio.cpp`. Game code uses
`SoundHandler.hpp`; no separate SDK installation or DLL distribution is needed.
The game loads the converted PCM WAV files, so no Vorbis decoder is required.
