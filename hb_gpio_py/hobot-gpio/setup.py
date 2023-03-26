
from setuptools import setup

classifiers = ['Operating System :: POSIX :: Linux',
               'License :: OSI Approved :: MIT License',
               'Intended Audience :: Developers',
               'Programming Language :: Python :: 2.7',
               'Programming Language :: Python :: 3',
               'Topic :: Software Development',
               'Topic :: System :: Hardware']

setup(name                          = 'Hobot.GPIO',
      version                       = '0.0.2',
      author                        = 'HORIZON',
      author_email                  = 'technical_support@horizon.ai',
      description                   = 'A module to control Hobot GPIO channels',
      long_description              = open('README.md').read(),
      long_description_content_type = 'text/markdown',
      license                       = 'MIT',
      keywords                      = 'Hobot GPIO',
      url                           = '',
      classifiers                   = classifiers,
      package_dir                   = {'': 'lib/python/'},
      packages                      = ['Hobot', 'Hobot.GPIO', 'RPi', 'RPi.GPIO'],
      package_data                  = {'Hobot.GPIO': []},
      include_package_data          = True,
)
