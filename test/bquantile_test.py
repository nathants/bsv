# base on: https://github.com/DataDog/sketches-py/tree/88ec88dacde7bd9c3f0bd198635d7f2df371b095/tests
#
# Unless explicitly stated otherwise all files in this repository are licensed
# under the Apache License 2.0.
# This product includes software developed at Datadog (https://www.datadoghq.com/).
# Copyright 2018 Datadog, Inc.
#
# flake8: noqa

from collections import defaultdict, namedtuple
import abc
import numpy as np
import pytest
import os
import shell
from test_util import run, rm_whitespace, rm_whitespace, clone_source

def setup_module(m):
    m.tempdir = clone_source()
    m.orig = os.getcwd()
    m.path = os.environ['PATH']
    os.chdir(m.tempdir)
    os.environ['PATH'] = f'{os.getcwd()}/bin:/usr/bin:/usr/local/bin:/sbin:/usr/sbin:/bin'
    shell.run('make clean && make bsv csv bschema bquantile-sketch bquantile-merge', stream=True)

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/') or m.tempdir.startswith('/private/var/folders/')
    shell.run('rm -rf', m.tempdir)

class Dataset(object):
    __metaclass__ = abc.ABCMeta

    def __init__(self, size):
        self.size = int(size)
        self.data = self.populate()

    def __str__(self):
        return '{}_{}'.format(self.name, self.size)

    def __len__(self):
        return self.size

    def rank(self, value):
        lower = np.array(sorted(self.data)) < value
        if np.all(lower):
            return self.size
        else:
            return np.argmin(lower) + 1

    def quantile(self, q):
        self.data.sort()
        rank = int(q*(self.size - 1) + 1)
        return self.data[rank-1]

    @property
    def sum(self):
        return np.sum(self.data)

    @property
    def avg(self):
        return np.mean(self.data)

    @abc.abstractproperty
    def name(self):
        """name of dataset"""

    @abc.abstractmethod
    def populate(self):
        """populate self.data with self.size values"""

class EmptyDataset(Dataset):
    @property
    def name(self):
        return 'no_name'

    def populate(self):
        return []

    def add(self, val):
        self.size += 1
        self.data.append(val)

    def add_all(self, vals):
        self.size += len(vals)
        self.data.extend(vals)

class UniformForward(Dataset):
    @property
    def name(self):
        return 'uniform_forward'

    def populate(self):
        return list(self.generate())

    def generate(self):
        for x in range(self.size):
            yield x

class UniformBackward(Dataset):
    @property
    def name(self):
        return 'uniform_backward'

    def populate(self):
        return list(self.generate())

    def generate(self):
        for x in range(self.size, 0, -1):
            yield x

class UniformZoomIn(Dataset):
    @property
    def name(self):
        return 'uniform_zoomin'

    def populate(self):
        return list(self.generate())

    def generate(self):
        if self.size % 2 == 1:
            for item in range(self.size//2):
                yield item
                yield self.size - item - 1
            yield self.size//2
        else:
            for item in range(self.size//2):
                yield item
                yield self.size - item - 1

class UniformZoomOut(Dataset):
    @property
    def name(self):
        return 'uniform_zoomout'

    def populate(self):
        return list(self.generate())

    def generate(self):
        if self.size % 2 == 1:
            yield self.size//2
            half = int(np.ceil(self.size/2))
            for item in range(1, half+1):
                yield half + item
                yield half - item
        else:
            half = int(np.ceil(self.size/2)) - 0.5
            for item in range(0, int(half+0.5)):
                yield int(half + item + 0.5)
                yield int(half - item - 0.5)

class UniformSqrt(Dataset):
    @property
    def name(self):
        return 'uniform_sqrt'

    def populate(self):
        return list(self.generate())

    def generate(self):
        t = int(np.sqrt(2*self.size))
        item = 0
        initial_item = 0
        initial_skip = 1
        emitted = 0
        i = 0
        while emitted < self.size:
            item = initial_item
            skip = initial_skip
            for j in range(t-i):
                if item < self.size:
                    yield item
                    emitted += 1
                item += skip
                skip += 1
            if t-i > 1:
                initial_skip += 1
                initial_item += initial_skip
                i += 1
            else:
                initial_item += 1

class Constant(Dataset):
    constant = 42.0

    @property
    def name(self):
        return 'constant'

    def populate(self):
        return [self.constant] * self.size

class Exponential(Dataset):
    scale = 0.01

    @classmethod
    def from_params(cls, scale, n):
        cls.scale = scale
        return cls(n)

    @property
    def name(self):
        return 'exponential'

    def populate(self):
        return np.random.exponential(scale=self.scale, size=self.size)

class Lognormal(Dataset):
    scale = 100.0

    @classmethod
    def from_params(cls, scale, n):
        cls.scale = scale
        return cls(n)

    @property
    def name(self):
        return 'lognormal'

    def populate(self):
        return np.random.lognormal(size=self.size) / self.scale

class Normal(Dataset):
    loc = 37.4
    scale = 1.0

    @classmethod
    def from_params(cls, loc, scale, n):
        cls.loc = loc
        cls.scale = scale
        return cls(n)

    @property
    def name(self):
        return 'normal'

    def populate(self):
        return np.random.normal(loc=self.loc, scale = self.scale, size=self.size)

class Laplace(Dataset):
    loc = 11278.0
    scale = 100.0

    @classmethod
    def from_params(cls, loc, scale, n):
        cls.loc = loc
        cls.scale = scale
        return cls(n)

    @property
    def name(self):
        return 'laplace'

    def populate(self):
        return np.random.laplace(loc=self.loc, scale=self.scale, size=self.size)

class Bimodal(Dataset):
    right_loc = 17.3
    left_loc = -2.0
    left_std = 3.0

    @property
    def name(self):
        return 'bimodal'

    def populate(self):
        return [next(self.generate()) for _ in range(int(self.size))]

    def generate(self):
        if np.random.random() > 0.5:
            yield np.random.laplace(self.right_loc)
        else:
            yield np.random.normal(self.left_loc, self.left_std)

class Mixed(Dataset):
    mean = 0.0
    sigma = 0.25
    scale_factor = 0.1
    loc = 10.0
    scale = 0.5

    def __init__(self, size, ratio=0.9, ignore_rank=False):
        self.size = int(size)
        self.ratio = ratio
        self.data = self.populate()
        self._ignore_rank = ignore_rank

    @property
    def name(self):
        return 'mixed'

    def populate(self):
        return [next(self.generate()) for _ in range(int(self.size))]

    def generate(self):
        if np.random.random()<  self.ratio:
            yield self.scale_factor*np.random.lognormal(self.mean, self.sigma)
        else:
            yield np.random.normal(self.loc, self.scale)

class Trimodal(Dataset):
    right_loc = 17.3
    left_loc = 5.0
    left_std = 0.5
    exp_scale = 0.01

    @property
    def name(self):
        return 'trimodal'

    def populate(self):
        return [next(self.generate()) for _ in range(int(self.size))]

    def generate(self):
        if np.random.random() > 2.0/3.0:
            yield np.random.laplace(self.right_loc)
        elif np.random.random() > 1.0/3.0:
            yield np.random.normal(self.left_loc, self.left_std)
        else:
            yield np.random.exponential(scale=self.exp_scale)

test_quantiles = [0, 0.1, 0.25, 0.5, 0.75, 0.9, 0.95, 0.99, 0.999, 1]
test_sizes = [10, 100, 1000, 10000]
datasets = [UniformForward, UniformBackward, UniformZoomIn, UniformZoomOut, UniformSqrt, Constant, Exponential, Lognormal, Normal, Laplace, Bimodal, Trimodal, Mixed]

test_alpha = 0.05
test_bin_limit = 1024
test_min_value = 1.0e-9

def evaluate_sketch_accuracy(sketch, data, eps):
    n = data.size
    for q, sketch_q in zip(test_quantiles, sketch):
        data_q = data.quantile(q)
        err = abs(sketch_q - data_q)
        np.testing.assert_equal(err - eps*abs(data_q) <= 1e-15, True)

def sketch_data(datas, alpha, bins, minval, quantiles):
    quantiles = ','.join(map(str, quantiles))
    with shell.tempdir():
        for data in datas:
            shell.run(f'bsv | bschema a:f64 | bquantile-sketch f64 -b {bins} -a {alpha} >> sketches', stdin='\n'.join(map(str, data)) + '\n')
        csv = shell.run(f'cat sketches | bquantile-merge {quantiles} | bschema f64:a,f64:a | csv')
        return [float(v) for line in csv.splitlines() for [q, v] in [line.split(',')]]

def test_distributions():
    for dataset in datasets:
        for n in test_sizes:
            data = dataset(n)
            sketch = sketch_data([data.data], test_alpha, test_bin_limit, test_min_value, test_quantiles)
            evaluate_sketch_accuracy(sketch, data, test_alpha)

def test_merge_equal():
    parameters = [(35, 1), (1, 3), (15, 2), (40, 0.5)]
    for n in test_sizes:
        d = EmptyDataset(0)
        datas = []
        for params in parameters:
            generator = Normal.from_params(params[0], params[1], n)
            datas.append([])
            for v in generator.data:
                datas[-1].append(v)
                d.add(v)
        s = sketch_data(datas, test_alpha, test_bin_limit, test_min_value, test_quantiles)
        evaluate_sketch_accuracy(s, d, test_alpha)

def test_merge_unequal():
    ntests = 20
    for i in range(ntests):
        for n in test_sizes:
            d = Lognormal(n)
            datas = [[], []]
            for v in d.data:
                if np.random.random() > 0.7:
                    datas[0].append(v)
                else:
                    datas[1].append(v)
            s = sketch_data(datas, test_alpha, test_bin_limit, test_min_value, test_quantiles)
            evaluate_sketch_accuracy(s, d, test_alpha)

def test_merge_mixed():
    ntests = 20
    datasets = [Normal, Exponential, Laplace, Bimodal]
    for i in range(ntests):
        d = EmptyDataset(0)
        datas = []
        for dataset in datasets:
            datas.append([])
            generator = dataset(np.random.randint(0, 500))
            for v in generator.data:
                datas[-1].append(v)
                d.add(v)
        s = sketch_data(datas, test_alpha, test_bin_limit, test_min_value, test_quantiles)
        evaluate_sketch_accuracy(s, d, test_alpha)
