Raw2Fits
==
Convert digital camera's RAW files to the scientific FITS format
Console and GUI version.

Author: Oleg Kutkov <contact@olegkutkov.me>

# Building and installation
```sh
$ git clone https://github.com/olegkutkov/Raw2Fits.git
$ cd Raw2Fits
```

## Install dependencies
### Ubuntu/Debian:
```
sudo apt-get install libraw-dev libcfitsio-dev libconfig-dev libgtk-3-dev
```
### Fedora:
```
sudo yum install gcc LibRaw-devel libconfig-devel cfitsio-devel gtk3-devel
```

## GUI version
```sh
$ make
$ sudo make install
```

## Console version
```sh
$ make cli
$ sudo make install-cli
```

Raw to Fits application should appear in your DE applications menu


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
