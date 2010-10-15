Regress Pro
===========

About
-----

Regress Pro is scientific / industrial software that can be used to study experimental data coming from spectroscopic ellipsometers or reflectometers. The program has been developed mainly looking to the application of thin film measurement in semiconductor industry.

The software is suitable both to determine the thickness of the layers and to determine the optical properties of dielectric materials.

Here a couple of screenshots of Regress Pro at work:
.. image: images/screenshot-1.png

Features
--------


Regress pro features:

 * regression of reflectivity spectra at normal incidence
 * regression of ellipsometry spectra with a given angle of incidence
 * algorithms to perform grid search of best solutions
 * different models for material dispersion curves: harmonic oscillators model, cauchy model and lookup model
 * simultaneous fit of multiple spectra with common and individual parameters
 * user-friendly graphical user interface

Current Plans
-------------

It is planned to add new features in Regress Pro. Most notably :

  * ability to perform regression on dispersion curves based on a given model
  * multiple step regressions
  * interactive fitting of spectra
  * more accurate fitting by taking into account the numerical aperture of the light beam

License
-------

Regress Pro is distributed under the GNU General Public License (GPL). The program use two libraries provided under the GNU Lesser General Public License (LGPL): 

  GNU Scientific Library, version 1.7. The library is used mainly for the Levenberg-Marquardt algorythm for non-linear fitting.

  The FOX toolkit, version 1.4.33. This excellent library is used to provide a Graphical User Interface on Linux and Windows platforms.
 
Requirements
------------

The program can be compiled both on Windows and Linux/UNIX platforms using the GCC compiler. The following libraries are needed to compile the program

  * GNU Scientific Library, version 1.7
  * FOX toolkit library, a recent version of the 1.4 branch (at least 1.4.33)

The provided windows binary doesn't need any external library.


