Tauc-Lorentz dispersion model
=============================

Overview
--------


Formula
-------

.. math::

    \epsilon_2 = \frac{A E_0 C (E - E_g)^2}{(E^2 - E_0^2)^2 + C^2 E^2} \frac{1}{E}

B-splines are commonly used as basis functions to fit smoothing curves
to large data sets. To do this, the abscissa axis is broken up into
some number of intervals, where the endpoints of each interval are
called "breakpoints". These breakpoints are then converted to "knots"
by imposing various continuity and smoothness conditions at each
interface. Given a nondecreasing knot vector t = {t\ :sub:`0`, t\ :sub:`1`, ...,
t\ :sub:`n+k-1`}, the n basis splines of order k are defined by
