Raw2Fits
==
Convert digital camera's RAW files to the scientific FITS format
Console and GUI version.

Author: Oleg Kutkov <elenbert@gmail.com>, 
Crimean astrophysical observatory.

# Building and installation
```sh
$ git clone https://github.com/olegkutkov/Raw2Fits.git
$ cd Raw2Fits
```
## GUI version
```sh
$ make
$ sudo make install
```

## Console version
```sh
$ make clie
$ sudo make install-cli
```

Raw to Fits application should appear in your DE applications menu
# Dependencies

### LibRaw 
Library for reading RAW files obtained from digital photo cameras.
It's highly recommended to use latest stable version of the library instead of your distro version.
Versions older than 0.17 may not correctly extract exif (exposure, date/time, camera's owner) information from raw files.
You can always get actual version from the official website: https://www.libraw.org/download

### cfitsio
Library of C and Fortran subroutines for reading and writing data files in FITS.
You can always get actual version from the official website: https://heasarc.gsfc.nasa.gov/fitsio/fitsio.html

### libgtk-3 (GUI version only)
Install any version available in your distribution

### libconfig (Console version only)
Install any version available in your distribution


# Supported cameras
Raw2Fits was successfully tested with raw files of this cameras vendors and models:
- Canon EOS: 600D, 5DMARK III, 6D
- Nikon D5300
- Fujifilm X series
- Pentax K-7
- Konica-Minolta dimage A200
- Sony Alpha and ILCA series
- Hasselblad CFV

# Usage
Please check out documentation: https://github.com/olegkutkov/Raw2Fits/blob/master/doc/raw2fits.pdf

![](https://raw.githubusercontent.com/olegkutkov/Raw2Fits/master/doc/raw2fits_screenshot.png)

#
![](https://raw.githubusercontent.com/olegkutkov/Raw2Fits/master/doc/fits_header.png)
