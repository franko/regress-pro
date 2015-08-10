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

To begin to work the first step is to load some data to start working with.
The applications accept a variery of spectroscopic spectra from ellipsometers or reflectometers.
To load a spectrum use the menu function "Spectra -> Load Spectra" and choose the file.
Once the file is loaded you can visualize it by using the interactive fit window.
It can be opened using the menu function "Fitting -> Interactive Fit".

The interactive fit let you experiment interactively with your data but before you need to setup a suitable film stack.
In the next section we explain how to setup a film stack and define a fit recipe.

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
When the fit terminated the results will be shown in the bottom part of the main windows.

The output will include the optimized value of each parameter and the residual Chi Square.

.. note::

    The fit is done using the Levenberg–Marquardt non-linear fit algorithm.
	It is a method that finds a solution that minimise the sum of squares of the residual between the experimental data and the model.
	If a range is specified for one or more parameters a grid search is done before the Levenberg–Marquardt minimization.
	The search works looking to the residuals and choose the point in the grid with the smaller residuals.
	The point selected by the grid search is used as the initial seed for the Levenberg–Marquardt non-linear fit.

Once the fit is terminated you can visualize the experimental data together with the theoretical curve.
This can be done by opening the interactive fit window using the menu function "Fitting -> Interactive Fit".

It is important to know that when you perform a fit:

    - the "Result Stack" is updated with fit's result values
    - the interactive fit is updated also accordingly to the fit's results.

The "Result Stack" is a film stack where the results of the fit are stored.
On the other side, the film stack shown in the main window *is not modified* when you run a fit.
So, in general, if you want to inspect the results of the fit you can look to the "Result Stack".
This latter is accessible using the menu function "Fitting -> Edit Result Stack".

Interactive Fit Window
~~~~~~~~~~~~~~~~~~~~~~

In the previous section we said that the interactive fit window is modified according to the fit's results.
While this is true, in reality the general rule is that the interactive fit window is directly linked to the "Result Stack".
This means that any change made on the "Result Stack" will be reflected in the interactive fit and viceversa.
You can also *edit* the result stack and see the changes reported to the interactive fit window.

.. note::

    The fact that the interactive fit window is linked to the result stack means that if you want to change the model used for the interactive fit you need to make the changes in the "Result Stack".
    For example you could go in the result stack and change a dispersion model, add a layer or anything you want and then come back to the interactive fit window.
    Just be awar that when you run the fit recipe from the main window the result stack will be overwritten so be careful to save any important elements before running a fit recipe.

Running an Interactive Fit
~~~~~~~~~~~~~~~~~~~~~~~~~~

The interactive fit window let you perform a fit by choosing on the fly the fit parameters.
It is limited because it is not possible to use a grid search by specifying a range so only a simple non-linear fit will be done.
The interactive fit is useful because let you experiment to see how the ellipsometry or reflectometry response changes with each of the parameters.
You can change the value of each parameter to see how the response change and check when you get close to the experimental spectrum.

You can also perform a fit just by checking the parameters and using the menu function "Fit -> Run".


.. _interactive-fit-window-figure:

.. figure:: interactive-fit-window.png
