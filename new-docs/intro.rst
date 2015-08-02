Introduction to Regress Pro
===========================

Overview
--------

Regress Pro is an application for the analysis of spectroscopic ellipsometry and reflectometry data.

The application can load spectroscopic data coming from ellipsometers and reflectometers and use non-linear fit procedures to determine physical parameters like a film's thickness or it refractive index.
In the chapter about :ref:`loading spectra <spectra>` there are more details about what kind of files are accepted by the application.

To work with a ellipsometer or reflectometer spectrum you need to setup a model of the "film stack" where you define:

  - the substrate
  - a number of thin films, possibly one or zero
  - the environment where measurement is done, usually this is just "air"

The whole application assume that the data are acquired in reflection mode with an opaque substrate, which is the most common configuration.
The data that can be analyzed using the software are spectroscopic data acquired in a given spectral range.

Each film or medium that you define in the film stack correspond to a material whose optical properties should be given, either as a table of values, either as a model with free parameters.

.. figure:: regress-pro-illustrated-window.png

The general steps for using the software are:

- load an ellipsometry of reflectometry spectrum
- define a film stack by choosing the material for the substrate and the films
- define a fit strategy by precising one or more fit parameters
- run the fit to extract the optimized values for the parameters
