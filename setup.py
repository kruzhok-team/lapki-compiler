#!/usr/bin/env python3

from setuptools import setup, find_packages

setup(
    name='lapki-compiler',
    version='0.1',
    packages=find_packages(),
    include_package_data=True,
    # python_requires=">=3.10",
    entry_points = {
        'console_scripts': [
            'lapki-compiler = compiler.main:sync_main'
        ],
    },
)
