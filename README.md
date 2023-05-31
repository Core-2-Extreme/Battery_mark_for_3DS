# Battery mark for 3DS
![GitHub all releases](https://img.shields.io/github/downloads/Core-2-Extreme/Battery_mark_for_3DS/total?color=purple&style=flat-square)
![GitHub commits since latest release (by SemVer)](https://img.shields.io/github/commits-since/Core-2-Extreme/Battery_mark_for_3DS/latest?color=orange&style=flat-square)
![GitHub commit activity](https://img.shields.io/github/commit-activity/m/Core-2-Extreme/Battery_mark_for_3DS?color=darkgreen&style=flat-square)

## Index
* [Screenshots](https://github.com/Core-2-Extreme/Battery_mark_for_3DS#Screenshots)
* [Supported languages](https://github.com/Core-2-Extreme/Battery_mark_for_3DS#Supported-languages)
* [Links](https://github.com/Core-2-Extreme/Battery_mark_for_3DS#Links)
* [Build](https://github.com/Core-2-Extreme/Battery_mark_for_3DS#Build)
* [Patch note](https://github.com/Core-2-Extreme/Battery_mark_for_3DS#Patch-note)
* [License](https://github.com/Core-2-Extreme/Battery_mark_for_3DS#License)
* [Credits](https://github.com/Core-2-Extreme/Battery_mark_for_3DS#Credits)
* [Donation](https://github.com/Core-2-Extreme/Battery_mark_for_3DS#Donation)

## Screenshots
**Compare your battery with others.** \
<img src="https://user-images.githubusercontent.com/45873899/157168071-f21d8af2-51ed-4793-98ec-4c3943988a70.png" width="400" height="480"> \
<img src="https://user-images.githubusercontent.com/45873899/157168078-01f94697-6226-42f3-974f-bff92cd78b45.png" width="400" height="480">

## Supported languages
* English
* Japanese/日本語
* Hungarian/Magyar (translated by vargaviktor) (*0)
* Simplified Chinese/简体中文 (translated by LITTOMA) (*0)
* Italian/Italiano (translated by dixy52-beep) (*0)
* Spanish/Español (translated by Cookiee) (*0)
* Romanian/Română (translated by Tescu48) (*0)
* Polish/Polski (translated by JustScratchCoder) (*0)

*0 Setting menu only

## Links
[Discord channel](https://discord.gg/66qCrQNqrw) \
[GBAtemp thread](https://gbatemp.net/threads/release-battery-mark-v2.581951/)

## Build
You need : 
* [devkitpro](https://devkitpro.org/wiki/Getting_Started)

If you want to build .cia, then you also need : 
* [bannertool](https://github.com/Steveice10/bannertool/releases) and [makerom](https://github.com/3DSGuy/Project_CTR/releases) (Copy them in your path e.g. in `{devkitPro_install_dir}\tools\bin`).

If you already have devkitpro, type `{devkitPro_install_dir}\devkitARM\bin\arm-none-eabi-gcc -v`. \
You should see something like : 
```
.....
.....
.....
Thread model: posix
Supported LTO compression algorithms: zlib zstd
gcc version 12.2.0 (devkitARM release 60)
```
Make sure you have release 60 or later. \
If you have older devkitpro, update it or compilation will fail.

* Clone this repository
  * On windows run `build.bat`
  * On other system, type `make` (`make -j` for faster build)

## Patch note
### ver2.1.0
* Battery level/temp/voltage graph has been added to battery mark and battery mark ranking(*0)
* Test result (graph) will be saved to sdmc:/Battery_mark/result/{date}.jpg
* Many adjustment have been made

*0 The graph in battery mark ranking is only avaiable on results that are app ver >= 2.1.0

### ver2.0.0
* Added battery mark ranking
* Test result will be saved to notification list and sdmc:/Battery_mark/result/{date}.csv
* Changed battery mark test(test result is not compatible with older version)

## License
This software is licensed as GNU General Public License v3.0.

Third party libraries are licensed as :

| Library | License |
| ------- | ------- |
| [Base64](https://github.com/ReneNyffenegger/cpp-base64/blob/master/LICENSE) | No specific license name               |
| [citro2d](https://github.com/devkitPro/citro2d/blob/master/LICENSE)         | zlib License                           |
| [citro3d](https://github.com/devkitPro/citro3d/blob/master/LICENSE)         | zlib License                           |
| [curl](https://github.com/curl/curl/blob/master/COPYING)                    | No specific license name               |
| [dav1d](https://github.com/videolan/dav1d/blob/master/COPYING)              | BSD 2-Clause                           |
| [ffmpeg](https://github.com/FFmpeg/FFmpeg/blob/master/COPYING.GPLv2)        | GNU General Public License v2.0        |
| [libctru](https://github.com/devkitPro/libctru#license)                     | zlib License                           |
| [mbedtls](https://github.com/Mbed-TLS/mbedtls/blob/development/LICENSE)     | Apache License 2.0                     |
| [mp3lame](https://github.com/gypified/libmp3lame/blob/master/COPYING)       | GNU Lesser General Public License v2.0 |
| [stb_image](https://github.com/nothings/stb/blob/master/LICENSE)            | Public Domain                          |
| [x264](https://github.com/mirror/x264/blob/master/COPYING)                  | GNU General Public License v2.0        |
| [zlib](https://github.com/madler/zlib/blob/master/LICENSE)                  | zlib License                           |

## Credits
* Core 2 Extreme
* dixy52-beep (in app texture, Italian translation)
* vargaviktor (Hungarian translation)
* LITTOMA (Simplified chinese translation)
* Cookiee (Spanish translation)
* Tescu48 (Romanian translation)
* JustScratchCoder (Polish translation)

## Donation
If you find my app helpful, buy me a cup of coffee.
* BTC : bc1qm7pykag7jv4cgaujz5sm39ewf46teg6xzce5pc
* LTC : MFfS2BG7hq7dXarq2KYiufDddRcCwdCbUd
