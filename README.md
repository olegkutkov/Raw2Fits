Raw2Fits
==
Convert digital cameras RAW to scientific format FITS

Author: Oleg Kutkov <elenbert@gmail.com>, 
Crimean astrophysical observatory.

# Building and installation
```sh
$ git clone https://github.com/olegkutkov/Raw2Fits.git
$ cd Raw2Fits
$ make
$ sudo make install
```
Raw to Fits application should appear in your DE applications menu
# Dependencies
LibRaw - a library for reading RAW files obtained from digital photo cameras.
It's highly recommended to use latest stable version of the library instead of your distro version.
Versions older than 0.17 may not correctly extract exif (exposure, date/time, camera's owner) information from raw files.
You can always get actual version from the official website: https://www.libraw.org/download

# Supported cameras
Raw2Fits was successfully tested with raw files of this cameras vendors and models:
- Canon EOS: 600D, 5DMARK III, 6D
- Nikon D5300
- Fujifilm X series
- Pentax K-7
- Konika-Minolta dimage A200
- Sony Alpha and ILCA series
- Hasselblad CFV

# Usage
Please check out documentation: https://github.com/olegkutkov/Raw2Fits/blob/master/doc/raw2fits.pdf

![](https://raw.githubusercontent.com/olegkutkov/Raw2Fits/master/doc/raw2fits_screenshot.png)
