# BORZOI 2001 — original Xbox

Rotating 3D low-poly Borzoi dog with SDL2, nxdk and stereo background audio.

GitHub Actions runs after each push to main and builds an artifact named **borzoi-original-xbox** containing default.xbe and halo.pcm. Music is converted automatically from halo.mp3 with ffmpeg.

For a modded original Xbox upload these files by FTP to "E:\Apps\Borzoi\" then launch default.xbe via the dashboard. A pauses, left/right changes speed, Back exits. It has not been tested on real Xbox hardware.
