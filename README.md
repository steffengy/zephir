Zephir (Experiment for PHP 7)
======

[![Build Status](https://secure.travis-ci.org/steffengy/zephir.svg?branch=zephir-ngexp)](http://travis-ci.org/steffengy/zephir)
[![License](https://poser.pugx.org/phalcon/zephir/license.svg)](https://packagist.org/packages/phalcon/zephir)

Zephir - Ze(nd Engine) Ph(p) I(nt)r(mediate) - is a high level language that eases the creation and maintainability
of extensions for PHP. Zephir extensions are exported to C code that can be compiled and optimized by major C compilers
such as gcc/clang/vc++. Functionality is exposed to the PHP language.

Experiment Information
-------------------
* Based on #ab2bbbf36b121c9f9804f4131a04c538ee6f2365
* Current Incompatibilities:
  - Parameters are passed as value 
    (leading to failing PregMatch Tests, before they were pased as zval pointers directly into zephir functions)
  - Constant Inheritance seems broken (HEAP Corruption, atleast for interface inheritance as tested in zephir's unit tests)
* This experiment did not focus on performance, so some implementations may be heavily inefficient.
* This experiment will very likely contain some (more or less) heavy memory leaks, since it was completed in very short time
  and is not tested at all (except unit tests - which are very limited)
* PSR2 checking disabled, because it's not necessary for an experiment, and I do not want to invest time into fixing PSR-2.

Main features:

* Both dynamic/static typing
* Reduced execution overhead compared with full interpretation
* Restricted procedural programming, promoting OOP
* Memory safety
* Ahead-of-time (AOT) compiler to provide predictable performance

Compiler design goals:

* Multi-pass compilation
* Type speculation/inference
* Allow runtime profile-guided optimizations, pseudo-constant propagation and indirect/virtual function inlining

Requirements
------------

To compile zephir-parser:

* [json-c](https://github.com/phalcon/json-c) (Please install this one from Github)
* [re2c](http://re2c.org/)

To build the PHP extension:

* g++ >= 4.4/clang++ >= 3.x/vc++ 9
* gnu make 3.81 or later
* php development headers and tools

Installation
------------
You can install zephir using composer.
Run `composer require phalcon/zephir`, then run `zephir`
from your `bin-dir`. By default it is `./vendor/bin/zephir`.
You can read more about composer binaries
in it's [documentation](https://getcomposer.org/doc/articles/vendor-binaries.md).

For global installation via composer you can use `composer global require`.
Do not forget add `~/.composer/vendor/bin` into your `$PATH`.

Also you can just clone zephir repository and run `./install`.
For global installation add `-c` flag.

Additional notes on Ubuntu
--------------------------
The following packages are needed in Ubuntu:

* apt-get install re2c libpcre3-dev

Usage
-----
Compile the extension:

```bash
./bin/zephir compile
```

External Links
--------------
* [Documentation](http://zephir-lang.com/)
* [Official Blog](http://blog.zephir-lang.com/)
* [Forum](http://forum.zephir-lang.com/)
* [Twitter](http://twitter.com/zephirlang)

License
-------
Zephir is open-sourced software licensed under the MIT License. See the LICENSE file for more information.

Contributing
------------

See [CONTRIBUTING.md](https://github.com/phalcon/zephir/blob/master/CONTRIBUTING.md) for details about contributions to this repository.

Current Build Status
--------------------
Zephir is built under Travis CI service. Every commit pushed to this repository will queue a build into the continuous integration service and will run all PHPUnit tests to ensure that everything is going well and the project is stable. The current build status is:

[![Build Status](https://secure.travis-ci.org/steffengy/zephir.svg?branch=master)](http://travis-ci.org/steffengy/zephir)
