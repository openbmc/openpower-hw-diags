
from setuptools import setup, find_packages

setup(
    name        = "pelparsers-openpower-hw-diags",
    version     = "0.1",
    classifiers = [ "License :: OSI Approved :: Apache Software License" ],
    packages    = find_packages("modules"),
    package_dir = { "": "modules" },
)
