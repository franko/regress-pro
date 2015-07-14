Tauc-Lorentz dispersion model
=============================

Overview
--------


Formula
-------

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

    \epsilon_1(E) = \epsilon_1(\infty) + \frac{A C}{\pi \xi^4} \frac{a_{\textrm{ln}}}{2 \alpha E_0} \ln \left( \frac{E_0^2 + E_g^2+\alpha E_g}{E_0^2 + E_g^2-\alpha E_g} \right)

