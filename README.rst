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
 * different models for material dispersion curves: harmonic oscillators model, cauchy model and lookup model
 * simultaneous fit of multiple spectra with common and individual parameters
 * user-friendly graphical user interface

In version 1.4 the following features have been added:

 * ability to perform regression on dispersion curves based on a given model
 * interactive fit of experimental data

alongside with many other improvements in usability and the introduction of a new plotting style based on the anti-grain library of Maxim Shemarev.

License
-------

Regress Pro is distributed under the GNU General Public License (GPL). The application uses different libraries provided under the GPL or LGPL license:

  * GNU Scientific Library, version 1.4. The library is used mainly for the Levenberg-Marquardt algorythm for non-linear fitting.
  * The FOX toolkit, version 1.6. This excellent library is used to provide a Graphical User Interface on Linux and Windows platforms.
  * The anti-grain library of Maxi Shemarev

The following libraries are also used to implement the RSA algorithm for the registration code signature:

  * The GNU Multiple Precision Arithmetic Library (GMP)
  * libmhash

The binary packages available from the `website <http://www.regresspro.org/>`_ require a registration code for continued use.

Download
--------

The download section is available at `Regress Pro website <http://www.regresspro.org/>`_. Please check also the `registration section <http://www.regresspro.org/register.html>`_ to obtain a registration code.

Requirements
------------

The program can be compiled both on Windows and Linux/UNIX platforms using the GCC compiler. The following libraries are needed to compile the program

  * GNU Scientific Library, version >= 1.14
  * FOX toolkit library, version 1.6 
  * AGG library, version 2.5
  * GMP library
  * libmhash

