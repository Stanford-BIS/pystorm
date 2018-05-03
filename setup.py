#!/usr/bin/env python
import io

try:
    from setuptools import find_packages, setup
except ImportError:
    raise ImportError(
        "'setuptools' is required but not installed. To install it, "
        "follow the instructions at "
        "https://pip.pypa.io/en/stable/installing/#installing-with-get-pip-py")

def read(*filenames, **kwargs):
    encoding = kwargs.get('encoding', 'utf-8')
    sep = kwargs.get('sep', '\n')
    buf = []
    for filename in filenames:
        with io.open(filename, encoding=encoding) as f:
            buf.append(f.read())
    return sep.join(buf)

setup(
    name='pystorm',
    version='0.1',
    description='Python package for the Brainstorm chip',
    long_description=read('README.md'),
    classifiers=[
      'Development Status :: 3 - Alpha',
      'Programming Language :: Python :: 3.5',
      'Programming Language :: Python :: 3.6',
      'Intended Audience :: Science/Research'
    ],
    keywords='neuromorphic, neural networks',
    url='https://github.com/Stanford-BIS/pystorm',
    author='Sanford Brains in Silicon group, Center for Theoretical Neuroscience, University of Waterloo',
    packages=find_packages(),
    install_requires=[
        'numpy>=1.7',
        'rectpack>=0.2',
        'pyyaml>=3.12',
        'metis>=0.1'
    ],
    include_package_data=True,
    zip_safe=False
)
