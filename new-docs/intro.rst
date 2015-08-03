Introduction to Regress Pro
===========================

Overview
--------

Regress Pro is an application for the analysis of spectroscopic ellipsometry and reflectometry data.

The application can load spectroscopic data coming from ellipsometers and reflectometers and use non-linear fit procedures to determine physical parameters like a film's thickness or it refractive index.
The application assume that the data are acquired in reflection mode with an opaque substrate and the spectra are expected to be resolved in the wavelength domain in a given range.
In the chapter about :ref:`loading spectra <spectra>` there are more details about what kind of files are accepted by the application.

The first things to do when working with data is to setup a model of the "film stack" where you define:

  - the substrate
  - a number of thin films, possibly a single film or non
  - the environment where measurement is done, usually this is just "air"

Each film or medium that you define in the film stack corresponds to a material whose optical properties should be given, either as a table of values, either as a model with free parameters.
For each film of the film stack you need to provide, on the right text fields, the nominal film thickness, expressed in Angstrom.
The nominal thickness is a reference value and does not need to be exact as the fit algorithm will usually search a best fit value.

In the figure below you can see that main window of Regress Pro and the area where the film stack is configured with the corresponding thicknesses on the right.

.. _main-window-figure:

.. figure:: regress-pro-illustrated-window.png

In general, once a spectroscopic spectrum is loaded and a film stack model is defined you can begin to treat your data.
To this purpose Regress Pro offer two mode of working, running a fit recipe or perform the fit interactively.

Fit Recipe
----------

A fit recipe is defined by a list of fitting parameters, their corresponding seed values and an optional range for each parameter.
The fit parameters are configured in the main window in the area shown in the :ref:`figure <main-window-figure>` above.
The entries are chosen from the list and added with the "return" key.
When adding a parameter a seed value can be explicitly given or otherwise an appropriate default value will be automatically selected.
In addition a range can be specified when entering the fit parameter.
If a range is given the system will perform a grid search for the given parameter by selecting a suitable step size.
The grid search is an useful option to ensure that the non-linear fit algorithm can found an optimal solution.

.. warning::

    If the grid search is made on multiple parameters the search space can be very big and the grid search can take a very long time.
    When you need to give a range try to use it only for a very small number of parameters, ideally not more than three.

Fit Parameters
~~~~~~~~~~~~~~

The list of the available fit parameters depends on the film stack.
For any given stack all the thicknesses of each film will be available as a fit parameters.
The thicknesses are are named T1, T2, T3, ... where T1 is the top layer, T2 is the layer below and so on and the values are always expressed in Angstrom.

In addition to the film's thicknesses, if a model is used for any of the films, the model's parameters will be available for fit.
The model parameters are listed grouped by layer number and named accordingly to the model used.
