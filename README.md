# disposer
A dynamic module management system

<b>This software is highly experimental. Do not use it in production code!</b>

## Supported Compilers

- GCC 6.2 and higher
- clang 3.9 and higher

## Direct dependencies

- boost.type_index
- boost.optional
- boost.lexical_cast
- boost.variant
- boost.range
- boost.hana
- boost.spirit (develop branch of X3)
- boost.fusion (as long as Sprit X3 does not support boost.hana)

## Indirect dependencies

- boost.array
- boost.assert
- boost.concept_check
- boost.config
- boost.container
- boost.core
- boost.detail
- boost.exception
- boost.function_types
- boost.functional
- boost.integer
- boost.iterator
- boost.math
- boost.move
- boost.mpl
- boost.numeric_conversion
- boost.predef
- boost.preprocessor
- boost.regex
- boost.smart_ptr
- boost.static_assert
- boost.throw_exception
- boost.type_traits
- boost.typeof
- boost.utility

## Definition

In a software process often different calculation steps are executed in sequence. Such a calculation step is called <i>module</i>. A module has a number of fixed <i>parameters</i> as well as <i>inputs</i> and <i>outputs</i>.

A sequence of modules is called <i>process chain</i>. Each process chain also gets a <i>unique identifier</i>.

The disposer manages the process chains and their modules. This is done through a configuration file that is read at runtime.

## Config-File

The configuration file has a special file format, which is explained below. There is no standardized file extension, but is is recommended to use '\*.ini' to get syntax highlighting in text editors.

