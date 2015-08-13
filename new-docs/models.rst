Dispersions Models
==================

Overview
--------

Regress Pro offer a variety of different dispersions models that can be used to describe different kind of materials.
The models available in Regress Pro are: Cauchy, Lookup, Tauc-Lorentz, Forouhi-Bloomer and Harmonic Oscillators.
The latter three models based on oscillators and they can have one or more oscillators.

When a dispersion model is chosen for a film the parameters of the model will become available to be changed either directly or by performing a fit.

.. tip::

	If you need to chose a suitable starting point for the model is is adviced to use the dispersion optimizer.
	This latter will let you "optimize" the parameters of a model by comparing with a reference curve.
	By using the dispersion optimizer it is possible to prepare a model with "reasonable" starting values to be later optimized based on the experimental spectra.

Cauchy Model
------------

The Cauchy model is one of the older and simpler models and it can describe the dispersion of a trasparent dielectric film.
The formula of the Cauchy model for the refractive index is:

.. math::

    n(\lambda) = N_0 + \frac{N_1}{\lambda^2} + \frac{N_2}{\lambda^4}

In theory more higher order terms could be included but the applications is limited to the formula above.

The absorption coefficient for the Cauchy model is equal to zero for all the wavelength.
From the practical point of view the application allow to have a non-zero absorption coefficient with a similar formula:

.. math::

    k(\lambda) = K_0 + \frac{K_1}{\lambda^2} + \frac{K_2}{\lambda^4}

Harmonic Oscillators model
--------------------------

The Harmonic Oscillators model is based on the classical theory of a damped harmonic oscillator.
Each oscillators is specified by

	- the energy, in eV named En in the model. Determine the position of the absoption peak.
	- the damping energy, in eV as well and named Eg. Determine the width of the peak.
	- the intensity of the oscillator, named Nosc. Determine the intensity of the peak.
	- the local field effect coefficient, named Nu. Should be 1/3 for dielectrics. Can be zero for non-localized electronic terms.
	- un empirical phase term, named Phi, to adjust the relative phase of the oscillators. These parameter is absent from the true physical model.

The formula for the harmonic oscillator for the dielectric constant :math:`\epsilon` is the following:

.. math::

    \epsilon(\textrm{E}) = 1 + \frac{\sum_i H_i}{1 - \sum_i \nu_i H_i}

where each term :math:`H_i` comes from a single oscillator.
This formula for this latter is:

.. math::

    H_i = 16 \pi \textrm{Ry}^2 r_0^3 \, \frac{\textrm{Nosc}_i \, e^{- i \phi_i}}{\textrm{En}_i^2 - \textrm{E}^2 + i \, \textrm{Eg}_i \, \textrm{E}}

where :math:`\textrm{Ry}` is the Rydberg constant in eV and :math:`r_0` is the Bohr radius in nm.

Tauc-Lorentz model
------------------

In the Tauc-Lorentz model the imaginary part of :math:`\epsilon` is given by the following expression:

.. math::

    \epsilon_2(E) =  \left\{ \begin{array}{ll}
         \frac{A E_0 C (E - E_g)^2}{(E^2 - E_0^2)^2 + C^2 E^2} \frac{1}{E} & E > Eg \\
         0 & E \le Eg \end{array} \right.

and the real part or :math:`\epsilon` is given by the Kramers-Kronig relations:

.. math::

    \epsilon_1(E) = \epsilon_1(\infty) + \frac{2}{\pi} P \int_{Eg}^\infty \frac{\xi \, \epsilon_2(\xi)}{\xi^2 - E^2} \textrm{d} \xi

An explicit expression for :math:`\epsilon_1` is given by Jellison-Modine:

.. math::

    \epsilon_1(E) = \epsilon_1(\infty) +
    \frac{A \, C}{\pi \xi^4} \frac{a_{\textrm{ln}}}{2 \alpha E_0} \ln \left( \frac{E_0^2 + E_g^2+\alpha E_g}{E_0^2 + E_g^2-\alpha E_g} \right) + \\
    - \frac{A}{\pi \xi^4} \frac{a_{\textrm{tan}}}{E_0} \left[ \pi - \tan^{-1}\left( \frac{2 E_g + \alpha}{C}\right) + \tan^{-1} \left( \frac{-2 E_g + \alpha}{C}\right) \right] + \\
    + 2 \frac{A \, E_0}{\pi \xi^4 \alpha} E_g (E^2 - \gamma^2) \left[ \pi + 2 \tan^{-1}\left(2 \frac{\gamma^2 - E_g^2}{\alpha C}\right)\right] + \\
    - \frac{A \, E_0 \, C}{\pi \xi^4} \frac{E^2 + E_g^2}{E} \ln \left( \frac{|E - E_g|}{E + E_g} \right) +
    \frac{2 A \, E_0 \, C}{\pi \xi^4} E_g \ln\left[\frac{|E - E_g| (E + E_g)}{\sqrt{(E_0^2 - E_g^2)^2 + E_g^2 C^2}}\right]

where

.. math::

    \begin{align*}
    a_{\textrm{ln}} & = (E_g^2 - E_0^2) E^2 + E_g^2 C^2 - E_0^2 (E_0^2 + 3 E_g^2) \\
    a_{\textrm{tan}} & = (E^2 - E_0^2) (E_0^2 + E_g^2) + E_g^2 C^2 \\
    \xi^4 & = (E^2 - \gamma^2)^2 + \alpha^2 C^2/4 \\
    \alpha & = \sqrt{4 E_0^2 - C^2} \\
    \gamma & = \sqrt{E_0^2 - C^2/2}
    \end{align*}


http://www.ita.uni-heidelberg.de/~gail/astromin/chap6.pdf
