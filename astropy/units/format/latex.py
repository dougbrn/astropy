# Licensed under a 3-clause BSD style license - see LICENSE.rst

"""
Handles the "LaTeX" unit format.
"""

import re

import numpy as np

from . import base, utils


class Latex(base.Base):
    """
    Output LaTeX to display the unit based on IAU style guidelines.

    Attempts to follow the `IAU Style Manual
    <https://www.iau.org/static/publications/stylemanual1989.pdf>`_.
    """

    @classmethod
    def _latex_escape(cls, name):
        # This doesn't escape arbitrary LaTeX strings, but it should
        # be good enough for unit names which are required to be alpha
        # + "_" anyway.
        return name.replace("_", r"\_")

    @classmethod
    def _get_unit_name(cls, unit):
        name = unit.get_format_name("latex")
        if name == unit.name:
            return cls._latex_escape(name)
        return name

    @classmethod
    def _format_unit_list(cls, units):
        out = []
        for base_, power in units:
            base_latex = cls._get_unit_name(base_)
            if power == 1:
                out.append(base_latex)
            else:
                # If the LaTeX representation of the base unit already ends with
                # a superscript, we need to spell out the unit to avoid double
                # superscripts. For example, the logic below ensures that
                # `u.deg**2` returns `deg^{2}` instead of `{}^{\circ}^{2}`.
                if re.match(r".*\^{[^}]*}$", base_latex):  # ends w/ superscript?
                    base_latex = base_.short_names[0]
                out.append(f"{base_latex}^{{{utils.format_power(power)}}}")
        return r"\,".join(out)

    @classmethod
    def to_string(cls, unit, inline=False):
        if unit.scale == 1:
            s = ""
        else:
            s = cls.format_exponential_notation(unit.scale)

        if len(unit.bases):
            if s:
                s += r"\,"
            if inline:
                nominator = zip(unit.bases, unit.powers)
                denominator = []
            else:
                nominator, denominator = utils.get_grouped_by_powers(
                    unit.bases, unit.powers
                )
            if len(denominator):
                if len(nominator):
                    nominator = cls._format_unit_list(nominator)
                else:
                    nominator = "1"
                denominator = cls._format_unit_list(denominator)
                s += rf"\frac{{{nominator}}}{{{denominator}}}"
            else:
                nominator = cls._format_unit_list(nominator)
                s += nominator

        return rf"$\mathrm{{{s}}}$"

    @classmethod
    def format_exponential_notation(cls, val, format_spec=".8g"):
        """
        Formats a value in exponential notation for LaTeX.

        Parameters
        ----------
        val : number
            The value to be formatted

        format_spec : str, optional
            Format used to split up mantissa and exponent

        Returns
        -------
        latex_string : str
            The value in exponential notation in a format suitable for LaTeX.
        """
        if np.isfinite(val):
            m, ex = utils.split_mantissa_exponent(val, format_spec)

            parts = []
            if m:
                parts.append(m)
            if ex:
                parts.append(f"10^{{{ex}}}")

            return r" \times ".join(parts)
        else:
            if np.isnan(val):
                return r"{\rm NaN}"
            elif val > 0:
                # positive infinity
                return r"\infty"
            else:
                # negative infinity
                return r"-\infty"


class LatexInline(Latex):
    """
    Output LaTeX to display the unit based on IAU style guidelines with negative
    powers.

    Attempts to follow the `IAU Style Manual
    <https://www.iau.org/static/publications/stylemanual1989.pdf>`_ and the
    `ApJ and AJ style guide
    <https://journals.aas.org/manuscript-preparation/>`_.
    """

    name = "latex_inline"

    @classmethod
    def to_string(cls, unit, inline=True):
        return super().to_string(unit, inline)
