# disposer
A dynamic module management system

<b>This software is highly experimental. Do not use it in production code!</b>

## Supported Compilers

- GCC 7.0 and higher
- LLVM 4.0 and higher

## Direct dependencies

- boost.type_index
- boost.lexical_cast
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
- boost.optional
- boost.predef
- boost.preprocessor
- boost.regex
- boost.smart_ptr
- boost.static_assert
- boost.throw_exception
- boost.type_traits
- boost.typeof
- boost.utility
- boost.variant

## Definition

In a software process often different calculation steps are executed in sequence. Such a calculation step is called <i>module</i>. A module has a number of fixed <i>parameters</i> as well as <i>inputs</i> and <i>outputs</i>.

A sequence of modules is called <i>process chain</i>. Each process chain also gets a <i>unique identifier</i>.

The disposer manages the process chains and their modules. This is done through a configuration file that is read at runtime.

## Config-File

The configuration file has a special file format, which is explained below. There is no standardized file extension, but is is recommended to use '\*.ini' to get syntax highlighting in text editors. It is also recommended to use a text editor which can show whitespaces, because tabulators are important for the file format.

We start with an example file:

```ini
; this section is optional
parameter_set
	; if it exists, it must exist at least one parameter set
	; 'io' is a freely choseable name of our first parameter set
	io
		; it must exist at least one parameter
		; 'start' and 'end' are parameter names from an disposer module
		;     the kind of values also depends on this module
		start = 4
		end = 10

	; another parameter set …
	save
		; this time the value is path
		dir = /media/data/projekte/disposer_module/data/target


; the first core section: we are loading modules and setting its parameters
module
	; 'create' is a name you can freely chose
	; 'maker' is the disposer module name
	create = maker
		; we use all parameters from our parameter set 'io'
		parameter_set = io

	; a second module
	save_tar = png_saver
		; again we use a parameter set
		parameter_set = save
		; additionaly we define the parameter 'tar'
		tar = true

	; a third module
	create_2 = maker
		; we can use multiple parameter sets
		; all parameter sets must apear before the first explicit parameter
		; if a parameter is defined in multiple parameter sets, the
		;     definition from the last set is used
		parameter_set = io
		parameter_set = save
		; an explicit parameter definition overwrites the parameter set
		start = 2

	filter = gauss
		kernel = 3
		sigma = 0.7


; the second core section: we build process chains with our modules
chain
	; 'build_1' is a freely choseable name of our first process chain
	; the group name is implicitly 'default'
	build_1
		; use the module 'create' from the module section
		create
			; set outputs
			->
				; 'sequence' is an output from disposer module 'maker'
				; 'data' is a freely choseable variable name
				sequence = data

		; use module 'save_tar'
		save_tar
			; set inputs
			<-
				; 'sequence' is an input from disposer module 'png_saver'
				; 'data' is the variable name we defined before
				sequence = data

	; another process chain
	; the group name is explicitly 'special'
	build_2 = special
		; explicitly defined id_generator 'abc'
		id_generator = abc

		create_2
			->
				sequence = data1

		filter
			; first inputs
			<-
				sequence = data1
			; second outputs
			->
				sequence = data2

		save_tar
			<-
				sequence = data2

```
