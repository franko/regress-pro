Regress Pro
===========

About
-----

Regress Pro is scientific / industrial software that can be used to study experimental data coming from spectroscopic ellipsometers or reflectometers. The program has been developed mainly looking to the application of thin film measurement in semiconductor industry.

The software is suitable both to determine the thickness of the layers and to determine the optical properties of dielectric materials.

Features
--------


Regress pro features:

 * regression of reflectivity spectra at normal incidence
 * regression of ellipsometry spectra with a given angle of incidence
 * algorithms to perform grid search of best solutions
 * different models for material dispersion curves: harmonic oscillators model, cauchy model, tauc-lorentz and lookup model
 * simultaneous fit of multiple spectra with common and individual parameters
 * user-friendly graphical user interface

Since version 2.0 all the operations can be performed from the user interface without the need of writing a script.
The film stack an the fit recipe can be easily created and modified using the interface. In the same spirit the dispersions model can be easily created, inspected and modified.

License
-------

Regress Pro is distributed under the GNU General Public License (GPL). The application uses different libraries provided under the GPL or LGPL license:

  * GNU Scientific Library, version 1.4. The library is used mainly for the Levenberg-Marquardt algorythm for non-linear fitting.
  * The FOX toolkit, version 1.6. This excellent library is used to provide a Graphical User Interface on Linux and Windows platforms.
  * The anti-grain library of Maxi Shemarev

The documentation for the application is available from the `Regress Pro's main page <http://franko.github.io/regress-pro/>`_.

Download
--------

The binary packages as well as the source code are available from the `Github's release page <https://github.com/franko/regress-pro/releases>`_.

Requirements
------------

The program can be compiled both on Windows and Linux/UNIX platforms using the GCC compiler. The following libraries are needed to compile the program

  * GNU Scientific Library, version >= 1.14
  * FOX toolkit library, version 1.6 
  * AGG library, version 2.5
