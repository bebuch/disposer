# parameter

<b>This document is very incomplete!</b>

## Configuration functions

### `default_value_fn`

The `default_value_fn` is used to generate a default values if the use doesn't
configured a value in the config file.

`default_value_fn` takes a callable that is callable with 0 up to 2 parameters.
The optional first parameter is of type `hana::basic_type< type >`. The optional
second parameter is of type `module_make_ref`.

A parameter without an explicit `default_value_fn` function has no default
value, except its type is an instance of `std::optional` in which case it is an
empty optional. This can be used to implement runtime optional parameters.

If you provide a `default_value_fn` function you can return `void` to explicitly
disable a default value.
