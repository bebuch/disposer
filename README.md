# disposer
A dynamic module management system

<b>This software is highly experimental. Do not use it in production code!</b>

## Dependencies

- Boost.TypeIndex
- Boost.Optional
- Boost.Variant
- Boost.Lexical_Cast
- Boost.Range
- Boost.Hana (will be part of boost soon)
- Boost.Spirit X3 (will be part of boost soon)
- Boost.Fusion (as long as X3 does not support Hana)

## Definition

In a software process often different calculation steps are executed in sequence. Such a calculation step is called <i>module</i>. A module has a number of fixed <i>parameters</i> as well as <i>inputs</i> and <i>outputs</i>.

A sequence of modules is called <i>process chain</i>. Each process chain also gets a <i>unique identifier</i>.

The disposer manages the process chains and their modules. This is done through a file that is read at runtime.
