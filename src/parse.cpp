//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <disposer/parse.hpp>

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <fstream>
#include <string>
#include <map>

#include <logsys/stdlogb.hpp>
#include <logsys/log.hpp>



BOOST_FUSION_ADAPT_STRUCT(
	disposer::types::parse::parameter,
	key,
	value
)

BOOST_FUSION_ADAPT_STRUCT(
	disposer::types::parse::parameter_set,
	name,
	parameters
)

BOOST_FUSION_ADAPT_STRUCT(
	disposer::types::parse::io,
	name,
	variable
)

BOOST_FUSION_ADAPT_STRUCT(
	disposer::types::parse::module_parameters,
	parameter_sets,
	parameters
)

BOOST_FUSION_ADAPT_STRUCT(
	disposer::types::parse::module,
	type_name,
	parameters,
	inputs,
	outputs
)

BOOST_FUSION_ADAPT_STRUCT(
	disposer::types::parse::chain,
	name,
	group,
	id_generator,
	modules
)

BOOST_FUSION_ADAPT_STRUCT(
	disposer::types::parse::config,
	sets,
	chains
)


namespace disposer::parser{


	class syntax_error: public std::logic_error{
	public:
		syntax_error(std::string&& message, std::string::iterator pos):
			std::logic_error(std::move(message)),
			pos_(pos)
			{}

		std::string::iterator pos()const{ return pos_; }


	private:
		std::string::iterator const pos_;
	};


	namespace x3 = boost::spirit::x3;


	using x3::lit;
	using x3::char_;
	using x3::eol;
	using x3::eoi;


	/// \brief Parser for line counting
	template < typename Iterator >
	std::size_t line_count(Iterator first, Iterator last){
		namespace x3 = boost::spirit::x3;
		using x3::eol;

		std::size_t n = 1;
		auto const inc = [&n](x3::unused_type){ ++n; };

		x3::phrase_parse(first, last,
			x3::no_skip[ *(*(x3::omit[char_] - eol) >> eol[inc]) ],
			x3::space);

		return n;
	}

	/// \brief Get text from the last line
	template < typename Iterator >
	std::string get_error_line_before(Iterator first, Iterator pos){
		namespace x3 = boost::spirit::x3;
		using x3::eol;

		auto line_start = std::find_if(
			std::make_reverse_iterator(pos),
			std::make_reverse_iterator(first),
			[](char c){ return c == '\n' || c == '\r'; }).base();

		return std::string(line_start, pos);
	}

	/// \brief Get text from the first line
	template < typename Iterator >
	std::string get_error_line_after(Iterator pos, Iterator last){
		namespace x3 = boost::spirit::x3;
		using x3::eol;

		auto line_end = std::find_if(
			pos, last, [](char c){ return c == '\n' || c == '\r'; });

		if(last == line_end){
			return std::string(pos, line_end);
		}else{
			return std::string(pos, line_end) + '\n';
		}
	}


	struct error_base{
		template < typename Iter, typename Exception, typename Context >
		x3::error_handler_result on_error(
			Iter& /*first*/, Iter const& /*last*/,
			Exception const& x, Context const& /*context*/
		){
			throw syntax_error(this->message(), x.where());
		}

		virtual const char* message()const = 0;
	};


	namespace type = types::parse;


	struct space_tag;
	x3::rule< space_tag > const space("space");

	struct space_lines_tag;
	x3::rule< space_lines_tag > const space_lines("space_lines");

	struct separator_tag;
	x3::rule< separator_tag > const separator("separator");

	struct comment_tag;
	x3::rule< comment_tag > const comment("comment");

	struct keyword_spaces_tag;
	x3::rule< keyword_spaces_tag > const
		keyword_spaces("keyword_spaces");

	struct value_spaces_tag;
	x3::rule< value_spaces_tag > const
		value_spaces("value_spaces");

	auto const space_def =
		lit(' ') | '\t'
	;

	auto const space_lines_def =
		*(*space >> eol)
	;

	auto const separator_def =
		*space >> (eol >> space_lines >> -comment)
	;

	auto const comment_def =
		*space >> (lit('#') | ';') >> *(char_ - eol) >> separator
	;

	auto const keyword_spaces_def =
		+(char_(' ') | char_('\t')) >> !(eol | '=')
	;

	auto const value_spaces_def =
		+(char_(' ') | char_('\t')) >> !(eol | eoi)
	;


	struct keyword_tag;
	x3::rule< keyword_tag, std::string > const keyword("keyword");

	struct value_tag;
	x3::rule< value_tag, std::string > const value("value");

	auto const keyword_def =
		(char_ - space - '=' - eol) >>
		*(keyword_spaces | +(char_ - space - eol - '='))
	;

	auto const value_def =
		(char_ - space - eol) >>
		*(value_spaces | +(char_ - space - eol))
	;


	struct sets_param_tag;
	x3::rule< sets_param_tag, type::parameter > const
		sets_param("sets_param");

	struct sets_param_prevent_tag;
	x3::rule< sets_param_prevent_tag > const
		sets_param_prevent("sets_param_prevent");

	struct sets_param_list_tag;
	x3::rule< sets_param_list_tag, std::vector< type::parameter > > const
		sets_param_list("sets_param_list");

	struct sets_set_tag;
	x3::rule< sets_set_tag, type::parameter_set > const
		sets_set("sets_set");

	struct sets_set_list_tag;
	x3::rule< sets_set_list_tag, type::parameter_sets > const
		sets_set_list("sets_set_list");

	struct sets_set_list_checked_tag;
	x3::rule< sets_set_list_checked_tag, type::parameter_sets > const
		sets_set_list_checked("sets_set_list_checked");

	struct sets_config_tag;
	x3::rule< sets_config_tag, type::parameter_sets > const
		sets_config("sets_config");

	auto const sets_param_def =
		("\t\t" >> sets_param_prevent) > keyword  >
		*space > '=' > *space > value > separator
	;

	auto const sets_param_prevent_def =
		x3::expect[!("parameter_set" >> *space >> '=')]
	;

	auto const sets_param_list_def =
		x3::expect[+sets_param]
	;

	auto const sets_set_def =
		'\t' > keyword > separator > sets_param_list
	;

	auto const sets_set_list_def =
		x3::expect[+sets_set]
	;

	auto const sets_set_list_checked_def =
		sets_set_list > &x3::expect["chain" > separator]
	;

	auto const sets_config_def =
		("parameter_set" > separator > sets_set_list_checked) |
		&x3::expect["chain" >> separator]
	;

	struct sets_param_tag: error_base{
		virtual const char* message()const override{
			return "a parameter '\t\tname = value\n' with name != "
				"'parameter_set'";
		}
	};

	struct sets_param_prevent_tag: error_base{
		virtual const char* message()const override{
			return "a parameter, but a parameter name "
				"('\t\tname = value\n') must not be 'parameter_set'";
		}
	};

	struct sets_param_list_tag: error_base{
		virtual const char* message()const override{
			return "at least one parameter line '\t\tname = value\n' "
				"with name != 'parameter_set'";
		}
	};

	struct sets_set_tag: error_base{
		virtual const char* message()const override{
			return "a parameter set line '\tname\n'";
		}
	};

	struct sets_set_list_tag: error_base{
		virtual const char* message()const override{
			return "at least one parameter set line '\tname\n'";
		}
	};

	struct sets_set_list_checked_tag: error_base{
		virtual const char* message()const override{
			return "a parameter set line '\tname\n' or a parameter definition "
				"('\t\tname = value\n') or keyword line 'chain\n'";
		}
	};

	struct sets_config_tag: error_base{
		template < typename Iter, typename Exception, typename Context >
		x3::error_handler_result on_error(
			Iter& first, Iter const& last,
			Exception const& x, Context const& context
		){
			msg_ = "keyword line 'parameter_set\n'";
			if(x.which() != "separator"){
				msg_ += " or keyword line 'chain\n'";
			}
			return error_base::on_error(first, last, x, context);
		}

		virtual const char* message()const override{
			return msg_.c_str();
		}

		std::string msg_;
	};


	struct params_tag;
	x3::rule< params_tag, type::module_parameters > const
		params("params");

	struct params_checked_tag;
	x3::rule< params_checked_tag, type::module_parameters > const
		params_checked("params_checked");

	struct set_ref_tag;
	x3::rule< set_ref_tag, std::string > const
		set_ref("set_ref");

	struct param_prevent_tag;
	x3::rule< param_prevent_tag > const
		param_prevent("param_prevent");

	struct param_tag;
	x3::rule< param_tag, type::parameter > const
		param("param");

	struct input_tag;
	x3::rule< input_tag, type::io > const input("input");

	struct output_tag;
	x3::rule< output_tag, type::io > const output("output");

	struct input_params_tag;
	x3::rule< input_params_tag, std::vector< type::io > >
		const input_params("input_params");

	struct output_params_tag;
	x3::rule< output_params_tag, std::vector< type::io > >
		const output_params("output_params");

	struct inputs_tag;
	x3::rule< inputs_tag, std::vector< type::io > > const
		inputs("inputs");

	struct outputs_tag;
	x3::rule< outputs_tag, std::vector< type::io > > const
		outputs("outputs");

	struct module_tag;
	x3::rule< module_tag, type::module > const
		module("module");

	struct chain_tag;
	x3::rule< chain_tag, type::chain > const chain("chain");

	struct chain_params_tag;
	x3::rule< chain_params_tag, std::vector< type::module > >
		const chain_params("chain_params");

	struct group_tag;
	x3::rule< group_tag, std::string > const group("group");

	struct id_generator_tag;
	x3::rule< id_generator_tag, std::string > const
		id_generator("id_generator");

	struct chains_params_tag;
	x3::rule< chains_params_tag, type::chains > const
		chains_params("chains_params");

	struct chains_tag;
	x3::rule< chains_tag, type::chains > const
		chains("chains");


	auto const set_ref_def =
		("\t\t\t\tparameter_set" >> *space >> '=')
		> *space > value > separator
	;

	auto const param_prevent_def =
		x3::expect[!("parameter_set" >> *space >> '=')]
	;

	auto const param_def =
		("\t\t\t\t" >> param_prevent) > keyword  >
		*space > '=' > *space > value > separator
	;

	auto const input_def =
		"\t\t\t\t" > (keyword >> *space) >
			('=' >> *space) > value > separator
	;

	auto const output_def =
		"\t\t\t\t" > (keyword >> *space) >
			('=' >> *space) > value > separator
	;

	auto const input_params_def =
		x3::expect[+input]
	;

	auto const output_params_def =
		x3::expect[+output]
	;

	auto const params_def =
		"\t\t\tparameter" > separator > params_checked
	;

	auto const params_checked_def =
		&x3::expect[set_ref | param] >>
		(*set_ref >> *param)
	;

	auto const inputs_def =
		("\t\t\t<-" > separator) >>
		input_params
	;

	auto const outputs_def =
		("\t\t\t->" > separator) >>
		output_params
	;

	auto const module_def =
		("\t\t" > keyword > separator) >>
		-params >>
		-inputs >>
		-outputs
	;

	auto const group_def =
		('=' >> *space) > value
	;

	auto const id_generator_def =
		("\t\tid_generator" >> *space) >
			('=' >> *space) > value > separator
	;

	auto const chain_params_def =
		x3::expect[+module]
	;

	auto const chain_def =
		('\t' > (keyword >> *space) > -group > separator) >>
		-id_generator >>
		chain_params
	;

	auto const chains_params_def =
		x3::expect[+chain]
	;

	auto const chains_def =
		(x3::expect["chain"] > separator) >>
		chains_params
	;


	struct config_tag;
	x3::rule< config_tag, type::config >
		const config("config");

	auto const config_def = x3::no_skip[x3::expect[
		space_lines >> -comment >>
		-sets_config >> chains
	]];


	struct params_tag: error_base{
		virtual const char* message()const override{
			return "keyword line '\t\t\tparameter\n'";
		}
	};

	struct params_checked_tag: error_base{
		virtual const char* message()const override{
			return "at least one parameter set reference line "
				"'\t\t\t\tparameter_set = name\n', where 'parameter_set' is a "
				"keyword and 'name' the name of the referenced parameter set "
				"or one parameter '\t\t\t\tname = value\n'";
		}
	};

	struct set_ref_tag: error_base{
		virtual const char* message()const override{
			return "a parameter set reference line "
				"'\t\t\t\tparameter_set = name\n', where 'parameter_set' is a "
				"keyword and 'name' the name of the referenced parameter set";
		}
	};

	struct modules_params_tag: error_base{
		virtual const char* message()const override{
			return "at least one module line '\tname = module\n'";
		}
	};

	struct param_prevent_tag: error_base{
		virtual const char* message()const override{
			return "another parameter, but a parameter name "
				"('\t\t\t\tname = value\n') must not be 'parameter_set'";
		}
	};

	struct param_tag: error_base{
		virtual const char* message()const override{
			return "a parameter '\t\t\t\tname = value\n' with name != "
				"'parameter_set'";
		}
	};

	struct input_tag: error_base{
		virtual const char* message()const override{
			return "input map '\t\t\t\tparameter = variable'";
		}
	};

	struct output_tag: error_base{
		virtual const char* message()const override{
			return "output map '\t\t\t\tparameter = variable'";
		}
	};

	struct input_params_tag: error_base{
		virtual const char* message()const override{
			return "at least one input map "
				"'\t\t\t\tparameter = variable'";
		}
	};

	struct output_params_tag: error_base{
		virtual const char* message()const override{
			return "at least one output map "
				"'\t\t\t\tparameter = variable'";
		}
	};

	struct inputs_tag: error_base{
		virtual const char* message()const override{
			return "keyword line '\t\t\t<-\n'";
		}
	};

	struct outputs_tag: error_base{
		virtual const char* message()const override{
			return "keyword line '\t\t\t->\n'";
		}
	};

	struct module_tag: error_base{
		virtual const char* message()const override{
			return "a module '\t\tmodule\n'";
		}
	};

	struct chains_params_tag: error_base{
		virtual const char* message()const override{
			return "at least one chain line '\tname [= group]\n'";
		}
	};

	struct group_tag: error_base{
		virtual const char* message()const override{
			return "a chain line with group '\tname = group\n'";
		}
	};

	struct id_generator_tag: error_base{
		virtual const char* message()const override{
			return "a id_generator line '\t\tid_generator = name\n', "
				"where id_generator is a keyword";
		}
	};

	struct chains_tag: error_base{
		virtual const char* message()const override{
			return "keyword line 'chain\n'";
		}
	};

	struct config_tag: error_base{
		virtual const char* message()const override{
			return "keyword line 'parameter_set\n' or keyword line "
				"'chain\n'";
		}
	};


	BOOST_SPIRIT_DEFINE(space)
	BOOST_SPIRIT_DEFINE(space_lines)
	BOOST_SPIRIT_DEFINE(separator)
	BOOST_SPIRIT_DEFINE(comment)
	BOOST_SPIRIT_DEFINE(keyword_spaces)
	BOOST_SPIRIT_DEFINE(value_spaces)
	BOOST_SPIRIT_DEFINE(keyword)
	BOOST_SPIRIT_DEFINE(value)
	BOOST_SPIRIT_DEFINE(sets_param)
	BOOST_SPIRIT_DEFINE(sets_param_prevent)
	BOOST_SPIRIT_DEFINE(sets_param_list)
	BOOST_SPIRIT_DEFINE(sets_set)
	BOOST_SPIRIT_DEFINE(sets_set_list)
	BOOST_SPIRIT_DEFINE(sets_set_list_checked)
	BOOST_SPIRIT_DEFINE(sets_config)
	BOOST_SPIRIT_DEFINE(param)
	BOOST_SPIRIT_DEFINE(param_prevent)
	BOOST_SPIRIT_DEFINE(params)
	BOOST_SPIRIT_DEFINE(params_checked)
	BOOST_SPIRIT_DEFINE(set_ref)
	BOOST_SPIRIT_DEFINE(input)
	BOOST_SPIRIT_DEFINE(output)
	BOOST_SPIRIT_DEFINE(inputs)
	BOOST_SPIRIT_DEFINE(outputs)
	BOOST_SPIRIT_DEFINE(input_params)
	BOOST_SPIRIT_DEFINE(output_params)
	BOOST_SPIRIT_DEFINE(module)
	BOOST_SPIRIT_DEFINE(chain_params)
	BOOST_SPIRIT_DEFINE(chain)
	BOOST_SPIRIT_DEFINE(group)
	BOOST_SPIRIT_DEFINE(id_generator)
	BOOST_SPIRIT_DEFINE(chains_params)
	BOOST_SPIRIT_DEFINE(chains)
	BOOST_SPIRIT_DEFINE(config)


	auto const grammar = config;


}


namespace disposer{


	types::parse::config parse(std::istream& is){
		namespace x3 = boost::spirit::x3;

		std::string str{
			std::istreambuf_iterator< char >{is},
			std::istreambuf_iterator< char >{}
		};

		types::parse::config config;

		auto iter = str.begin();
		auto end = str.end();

		x3::ascii::space_type space;

		try{
			bool const match =
				phrase_parse(iter, end, parser::grammar, space, config);

			if(!match || iter != end){
				auto const line_number = parser::line_count(str.begin(), iter);
				auto const before =
					parser::get_error_line_before(str.begin(), iter);
				auto const after = parser::get_error_line_after(iter, end);
				std::ostringstream os;
				os << "Syntax error at line " << line_number << ", pos "
					<< before.size() << ": '" << before << after
					<< "', incomplete parsing (programming error!)";
				throw std::runtime_error(os.str());
			}

			return config;
		}catch(parser::syntax_error const& e){
			auto const line_number = parser::line_count(str.begin(), e.pos());
			auto const before =
				parser::get_error_line_before(str.begin(), e.pos());
			auto const after = parser::get_error_line_after(e.pos(), end);

			std::ostringstream os;
			os << "Syntax error at line " << line_number << ", pos "
				<< before.size() << ": '" << before << after
				<< "', expected " << e.what();

			throw std::runtime_error(os.str());
		}
	}


	types::parse::config parse(std::string const& filename){
		std::ifstream is(filename.c_str());
		if(!is.is_open()){
			throw std::runtime_error("Can not open '" + filename + "'");
		}

		return parse(is);
	}



}
