Dispersions Models
==================

Overview
--------

Regress Pro offer a variety of different dispersions models useful to modelize the dispersion curves of any film.

The models provided by Regress Pro are:

    - Cauchy model, limited to dielectrics in the visible region.
    - Tauc-Lorentz model, useful for amorphous films and for absorbing dielectrics.
    - Forouhi-Bloomer model, same applications of the Tauc-Lorentz model.
    - Harmonic Oscillators model, the most general model that can model any material at the price of adding anough oscillators.
    - Lookup model, interpolate between know dispersion depending on a single parameter. Useful for substitutional films like SiGe or more generally for films whose property depends on a single parameter.

Note that the models based on oscillators let you have one or more oscillators.
In general you can choose to add as many oscillators as is needed to describe a given film type.

When a dispersion model is chosen for a film the parameters of the model will become available to be changed either directly or by performing a fit.

If you need to chose a suitable starting point for the model is is adviced to use the dispersion optimizer.
This latter will let you "optimize" the parameter of the model by comparing with a reference curve.
By using the dispersion optimizer it is possible to prepare a model with "reasonable" starting values to be later optimized based on the experimental spectra.

Cauchy Model
------------

A ecrire.