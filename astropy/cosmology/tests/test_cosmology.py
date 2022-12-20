# Licensed under a 3-clause BSD style license - see LICENSE.rst

"""Stand-alone overall systems tests for :mod:`astropy.cosmology`."""

import numpy as np
import pytest

import astropy.units as u
from astropy.cosmology import flrw
from astropy.units import allclose
from astropy.utils.compat.optional_deps import HAS_SCIPY

###############################################################################
# TODO! sort and refactor following tests.
# overall systems tests stay here, specific tests go to new test suite.


@pytest.mark.skipif(not HAS_SCIPY, reason="test requires scipy")
def test_distances():
    # Test distance calculations for various special case
    # scenarios (no relativistic species, normal, massive neutrinos)
    # These do not come from external codes -- they are just internal
    # checks to make sure nothing changes if we muck with the distance
    # calculators

    z = np.array([1.0, 2.0, 3.0, 4.0])

    # The pattern here is: no relativistic species, the relativistic
    # species with massless neutrinos, then massive neutrinos
    cos = flrw.LambdaCDM(75.0, 0.25, 0.5, Tcmb0=0.0)
    assert allclose(
        cos.comoving_distance(z),
        [2953.93001902, 4616.7134253, 5685.07765971, 6440.80611897] * u.Mpc,
        rtol=1e-4,
    )
    cos = flrw.LambdaCDM(75.0, 0.25, 0.6, Tcmb0=3.0, Neff=3, m_nu=u.Quantity(0.0, u.eV))
    assert allclose(
        cos.comoving_distance(z),
        [3037.12620424, 4776.86236327, 5889.55164479, 6671.85418235] * u.Mpc,
        rtol=1e-4,
    )
    cos = flrw.LambdaCDM(75.0, 0.3, 0.4, Tcmb0=3.0, Neff=3, m_nu=u.Quantity(10.0, u.eV))
    assert allclose(
        cos.comoving_distance(z),
        [2471.80626824, 3567.1902565, 4207.15995626, 4638.20476018] * u.Mpc,
        rtol=1e-4,
    )
    # Flat
    cos = flrw.FlatLambdaCDM(75.0, 0.25, Tcmb0=0.0)
    assert allclose(
        cos.comoving_distance(z),
        [3180.83488552, 5060.82054204, 6253.6721173, 7083.5374303] * u.Mpc,
        rtol=1e-4,
    )
    cos = flrw.FlatLambdaCDM(75.0, 0.25, Tcmb0=3.0, Neff=3, m_nu=u.Quantity(0.0, u.eV))
    assert allclose(
        cos.comoving_distance(z),
        [3180.42662867, 5059.60529655, 6251.62766102, 7080.71698117] * u.Mpc,
        rtol=1e-4,
    )
    cos = flrw.FlatLambdaCDM(75.0, 0.25, Tcmb0=3.0, Neff=3, m_nu=u.Quantity(10.0, u.eV))
    assert allclose(
        cos.comoving_distance(z),
        [2337.54183142, 3371.91131264, 3988.40711188, 4409.09346922] * u.Mpc,
        rtol=1e-4,
    )
    # Add w
    cos = flrw.FlatwCDM(75.0, 0.25, w0=-1.05, Tcmb0=0.0)
    assert allclose(
        cos.comoving_distance(z),
        [3216.8296894, 5117.2097601, 6317.05995437, 7149.68648536] * u.Mpc,
        rtol=1e-4,
    )
    cos = flrw.FlatwCDM(
        75.0, 0.25, w0=-0.95, Tcmb0=3.0, Neff=3, m_nu=u.Quantity(0.0, u.eV)
    )
    assert allclose(
        cos.comoving_distance(z),
        [3143.56537758, 5000.32196494, 6184.11444601, 7009.80166062] * u.Mpc,
        rtol=1e-4,
    )
    cos = flrw.FlatwCDM(
        75.0, 0.25, w0=-0.9, Tcmb0=3.0, Neff=3, m_nu=u.Quantity(10.0, u.eV)
    )
    assert allclose(
        cos.comoving_distance(z),
        [2337.76035371, 3372.1971387, 3988.71362289, 4409.40817174] * u.Mpc,
        rtol=1e-4,
    )
    # Non-flat w
    cos = flrw.wCDM(75.0, 0.25, 0.4, w0=-0.9, Tcmb0=0.0)
    assert allclose(
        cos.comoving_distance(z),
        [2849.6163356, 4428.71661565, 5450.97862778, 6179.37072324] * u.Mpc,
        rtol=1e-4,
    )
    cos = flrw.wCDM(
        75.0, 0.25, 0.4, w0=-1.1, Tcmb0=3.0, Neff=3, m_nu=u.Quantity(0.0, u.eV)
    )
    assert allclose(
        cos.comoving_distance(z),
        [2904.35580229, 4511.11471267, 5543.43643353, 6275.9206788] * u.Mpc,
        rtol=1e-4,
    )
    cos = flrw.wCDM(
        75.0, 0.25, 0.4, w0=-0.9, Tcmb0=3.0, Neff=3, m_nu=u.Quantity(10.0, u.eV)
    )
    assert allclose(
        cos.comoving_distance(z),
        [2473.32522734, 3581.54519631, 4232.41674426, 4671.83818117] * u.Mpc,
        rtol=1e-4,
    )
    # w0wa
    cos = flrw.w0waCDM(75.0, 0.3, 0.6, w0=-0.9, wa=0.1, Tcmb0=0.0)
    assert allclose(
        cos.comoving_distance(z),
        [2937.7807638, 4572.59950903, 5611.52821924, 6339.8549956] * u.Mpc,
        rtol=1e-4,
    )
    cos = flrw.w0waCDM(
        75.0, 0.25, 0.5, w0=-0.9, wa=0.1, Tcmb0=3.0, Neff=3, m_nu=u.Quantity(0.0, u.eV)
    )
    assert allclose(
        cos.comoving_distance(z),
        [2907.34722624, 4539.01723198, 5593.51611281, 6342.3228444] * u.Mpc,
        rtol=1e-4,
    )
    cos = flrw.w0waCDM(
        75.0, 0.25, 0.5, w0=-0.9, wa=0.1, Tcmb0=3.0, Neff=3, m_nu=u.Quantity(10.0, u.eV)
    )
    assert allclose(
        cos.comoving_distance(z),
        [2507.18336722, 3633.33231695, 4292.44746919, 4736.35404638] * u.Mpc,
        rtol=1e-4,
    )
    # Flatw0wa
    cos = flrw.Flatw0waCDM(75.0, 0.25, w0=-0.95, wa=0.15, Tcmb0=0.0)
    assert allclose(
        cos.comoving_distance(z),
        [3123.29892781, 4956.15204302, 6128.15563818, 6948.26480378] * u.Mpc,
        rtol=1e-4,
    )
    cos = flrw.Flatw0waCDM(
        75.0, 0.25, w0=-0.95, wa=0.15, Tcmb0=3.0, Neff=3, m_nu=u.Quantity(0.0, u.eV)
    )
    assert allclose(
        cos.comoving_distance(z),
        [3122.92671907, 4955.03768936, 6126.25719576, 6945.61856513] * u.Mpc,
        rtol=1e-4,
    )
    cos = flrw.Flatw0waCDM(
        75.0, 0.25, w0=-0.95, wa=0.15, Tcmb0=3.0, Neff=3, m_nu=u.Quantity(10.0, u.eV)
    )
    assert allclose(
        cos.comoving_distance(z),
        [2337.70072701, 3372.13719963, 3988.6571093, 4409.35399673] * u.Mpc,
        rtol=1e-4,
    )
    # wpwa
    cos = flrw.wpwaCDM(75.0, 0.3, 0.6, wp=-0.9, zp=0.5, wa=0.1, Tcmb0=0.0)
    assert allclose(
        cos.comoving_distance(z),
        [2954.68975298, 4599.83254834, 5643.04013201, 6373.36147627] * u.Mpc,
        rtol=1e-4,
    )
    cos = flrw.wpwaCDM(
        75.0,
        0.25,
        0.5,
        wp=-0.9,
        zp=0.4,
        wa=0.1,
        Tcmb0=3.0,
        Neff=3,
        m_nu=u.Quantity(0.0, u.eV),
    )
    assert allclose(
        cos.comoving_distance(z),
        [2919.00656215, 4558.0218123, 5615.73412391, 6366.10224229] * u.Mpc,
        rtol=1e-4,
    )
    cos = flrw.wpwaCDM(
        75.0,
        0.25,
        0.5,
        wp=-0.9,
        zp=1.0,
        wa=0.1,
        Tcmb0=3.0,
        Neff=4,
        m_nu=u.Quantity(5.0, u.eV),
    )
    assert allclose(
        cos.comoving_distance(z),
        [2629.48489827, 3874.13392319, 4614.31562397, 5116.51184842] * u.Mpc,
        rtol=1e-4,
    )

    # w0wz
    cos = flrw.w0wzCDM(75.0, 0.3, 0.6, w0=-0.9, wz=0.1, Tcmb0=0.0)
    assert allclose(
        cos.comoving_distance(z),
        [3051.68786716, 4756.17714818, 5822.38084257, 6562.70873734] * u.Mpc,
        rtol=1e-4,
    )
    cos = flrw.w0wzCDM(
        75.0, 0.25, 0.5, w0=-0.9, wz=0.1, Tcmb0=3.0, Neff=3, m_nu=u.Quantity(0.0, u.eV)
    )
    assert allclose(
        cos.comoving_distance(z),
        [2997.8115653, 4686.45599916, 5764.54388557, 6524.17408738] * u.Mpc,
        rtol=1e-4,
    )
    cos = flrw.w0wzCDM(
        75.0, 0.25, 0.5, w0=-0.9, wz=0.1, Tcmb0=3.0, Neff=4, m_nu=u.Quantity(5.0, u.eV)
    )
    assert allclose(
        cos.comoving_distance(z),
        [2676.73467639, 3940.57967585, 4686.90810278, 5191.54178243] * u.Mpc,
        rtol=1e-4,
    )

    # Also test different numbers of massive neutrinos
    # for FlatLambdaCDM to give the scalar nu density functions a
    # work out
    cos = flrw.FlatLambdaCDM(75.0, 0.25, Tcmb0=3.0, m_nu=u.Quantity([10.0, 0, 0], u.eV))
    assert allclose(
        cos.comoving_distance(z),
        [2777.71589173, 4186.91111666, 5046.0300719, 5636.10397302] * u.Mpc,
        rtol=1e-4,
    )
    cos = flrw.FlatLambdaCDM(75.0, 0.25, Tcmb0=3.0, m_nu=u.Quantity([10.0, 5, 0], u.eV))
    assert allclose(
        cos.comoving_distance(z),
        [2636.48149391, 3913.14102091, 4684.59108974, 5213.07557084] * u.Mpc,
        rtol=1e-4,
    )
    cos = flrw.FlatLambdaCDM(75.0, 0.25, Tcmb0=3.0, m_nu=u.Quantity([4.0, 5, 9], u.eV))
    assert allclose(
        cos.comoving_distance(z),
        [2563.5093049, 3776.63362071, 4506.83448243, 5006.50158829] * u.Mpc,
        rtol=1e-4,
    )
    cos = flrw.FlatLambdaCDM(
        75.0, 0.25, Tcmb0=3.0, Neff=4.2, m_nu=u.Quantity([1.0, 4.0, 5, 9], u.eV)
    )
    assert allclose(
        cos.comoving_distance(z),
        [2525.58017482, 3706.87633298, 4416.58398847, 4901.96669755] * u.Mpc,
        rtol=1e-4,
    )
