Regress Pro
===========

About
-----

Regress Pro is a free software application that can be used to study experimental data coming from spectroscopic ellipsometers or reflectometers. It was developed for scientific and industrial applications and has been designed and tested mainly for thin film measurements in semiconductor industry.

The application can be used to determine the thickness or the refractive index of thin films. Different kind of dispersion models are available as well as a library of know materials.

Regress Pro is unique among ellipsometry and reflectometry applications because it is free software. This means that everyone can obtain, study or modify the source code and see exactly how each computation is done. The software can also be adapted or improved to suit specific needs or to implement additional functions.

The documentation for the application is available from the `Regress Pro's main page <http://franko.github.io/regress-pro/>`_.

Features
--------

Regress pro features:

 * fit of ellipsometry or reflectivity spectra
 * algorithms to perform grid search of best solutions
 * different models for material dispersion curves: harmonic oscillators model, cauchy model, tauc-lorentz, forhoui-bloomer and lookup model
 * simultaneous fit of multiple spectra with common and individual parameters
 * batch run on a set of spectra with a common fit recipe
 * dispersion optimiser
 * user-friendly graphical user interface

Since version 2.0 all the operations can be performed from the user interface without the need of writing a script.
The film stack an the fit recipe can be easily created and modified using the interface. In the same spirit the dispersion models can be easily created, inspected and modified.

Fit recipes and dispersion models can be saved and loaded in a simple, readable text format.

The application offers also an interactive dispersion optimiser to create parametric models based on a reference dispersion table.

Download
--------

The binary packages as well as the source code are available from the `Github's release page <https://github.com/franko/regress-pro/releases>`_.

License
-------

Regress Pro is distributed under the GNU General Public License (GPL). The application uses different libraries provided under the GPL or LGPL license:

  * GNU Scientific Library, version >= 1.14. The library is used mainly for the Levenberg-Marquardt algorythm for non-linear fitting.
  * The FOX toolkit, version 1.6. This excellent library is used to provide a Graphical User Interface on Linux and Windows platforms.
  * The anti-grain library of Maxi Shemarev

Requirements
------------

The binary packages for Windows does not have any specific requirements and should works on Windows XP and Windows 7.

On linux the binary packages are provided for debian-based distributions for x86 and x86_64 architectures.

The program can be compiled both on Windows and Linux/UNIX platforms using the GCC compiler. The following libraries are needed to compile the program

  * GNU Scientific Library, version >= 1.14
  * FOX toolkit library, version 1.6 
  * AGG library, version 2.5
