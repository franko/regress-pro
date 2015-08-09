Introduction to Regress Pro
===========================

Overview
--------

Regress Pro is an application for the analysis of spectroscopic ellipsometry and reflectometry data.

The application can load spectroscopic data coming from ellipsometers and reflectometers and use non-linear fit procedures to determine physical parameters like a film's thickness or it refractive index.
The application assume that the data are acquired in reflection mode with an opaque substrate and the spectra are expected to be resolved in the wavelength domain in a given range.
In the chapter about :ref:`loading spectra <spectra>` there are more details about what kind of files are accepted by the application.

Using the Application
---------------------

The first things to do when working with data is to setup a model of the "film stack" where you define:

  - the substrate
  - a number of thin films, possibly a single film or none
  - the environment where measurement is done (usually "vacuum").

Each film or medium that you define in the film stack corresponds to a material whose optical properties should be given, either as a table of values, either as a model.
For each film the nominal film thickness in Angstrom is given on the text field on the right.

.. note::

   The nominal thickness is a just reference value and does not need to be exact as the fit algorithm will usually search a best fit value.
   You need to provide an accurate value only if the thickness in kept fixed in the fit recipe.

In the figure below you can see that main window of Regress Pro and the area where the film stack is configured with the corresponding thicknesses on the right.

.. _main-window-figure:

.. figure:: regress-pro-illustrated-window.png

In general, once a spectroscopic spectrum is loaded and a film stack model is defined you can begin to treat your data.
To this purpose Regress Pro offer two mode of working, running a fit recipe or perform the fit interactively.

Fit Recipe
----------

A fit recipe is defined by a list of fitting parameters each with a corresponding seed value and an optional range.
The fit parameters are configured in the main window in the area shown in the :ref:`figure <main-window-figure>` above.
The entries are chosen from the list and added with the "return" key.
When adding a parameter a seed value can be explicitly given or otherwise the seed will be marked an undefined.
When the seed value is undefined its nominal value will be retrieved from the film stack.

When entering a seed value a range can be also optionally given.
If a range is given the system will perform a grid search for the given parameter around the seed value plus and minus the range.
The step size for the grid search is automatically determined by the software.
The grid search is an useful option to ensure that the non-linear fit algorithm can found a global, optimal, solution.

.. note::

    If the grid search is made on multiple parameters the search space can be very big and the grid search can take a very long time.
    When you need to give a range try to use it only for a very small number of parameters, ideally not more than three.

Fit Parameters
~~~~~~~~~~~~~~

The list of the available fit parameters depends on the film stack.
For any given stack the thicknesses of each film will be available in the parameters list.
The thicknesses are are named T1, T2, T3, ... where T1 is the top layer, T2 is the layer below and so on and their values are always expressed in Angstrom.

When a film stack use a dispersion model for some of the layers the corresponding parameters will be available in the parameters list.
The model parameters are grouped by layer number and named accordingly to the model.
Adding a model parameters means that the fit will adjust its value to optimize the fit.
As for thicknesses you can specify a seed value to be used as a starting point and you can optionally give a range.

Running the Fit Recipe
~~~~~~~~~~~~~~~~~~~~~~

Once the film stack is configured and the list of fit parameters is defined with the corresponding seed values the fit can be done using th menu function "Fitting -> Run Fitting".
When the fit terminated you will have the fit's output in the bottom part of the main windows.

The output will include the optimized value of each parameter and the residual Chi Square.
The fit is done internally using the Levenberg–Marquardt non-linear fit algorithm.
Its work is to find a solution that minimise the sum of squares of the residual between the experimental data and the model.
If for one or more parameters a range is specified a grid search is done beform the Levenberg–Marquardt minimization.
The search works looking to the residuals and choose the point in the grid with the smaller residuals.
The point selected by the grid search is used as the initial seed for the Levenberg–Marquardt non-linear fit.

