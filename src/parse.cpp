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
	disposer::types::parse::module,
	type_name,
	parameter_sets,
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

	struct keyword_tag;
	x3::rule< keyword_tag, std::string > const keyword("keyword");

	struct value_spaces_tag;
	x3::rule< value_spaces_tag > const
		value_spaces("value_spaces");

	struct value_tag;
	x3::rule< value_tag, std::string > const value("value");

	struct set_parameter_tag;
	x3::rule< set_parameter_tag, types::parse::parameter > const
		set_parameter("set_parameter");

	struct prevent_set_parameter_set_tag;
	x3::rule< prevent_set_parameter_set_tag > const
		prevent_set_parameter_set("prevent_set_parameter_set");

	struct parameter_set_tag;
	x3::rule< parameter_set_tag, types::parse::parameter_set > const
		parameter_set("parameter_set");

	struct parameter_set_params_tag;
	x3::rule< parameter_set_params_tag, std::vector< types::parse::parameter > >
		const parameter_set_params("parameter_set_params");

	struct parameter_sets_tag;
	x3::rule< parameter_sets_tag, types::parse::parameter_sets > const
		parameter_sets("parameter_sets");

	struct parameter_sets_params_tag;
	x3::rule< parameter_sets_params_tag, types::parse::parameter_sets > const
		parameter_sets_params("parameter_sets_params");

	struct module_parameter_tag;
	x3::rule< module_parameter_tag, types::parse::module > const
		module_parameter("module_parameter");

	struct module_parameter_sets_tag;
	x3::rule< module_parameter_sets_tag, std::vector< std::string > > const
		module_parameter_sets("module_parameter_sets");

	struct prevent_parameter_set_tag;
	x3::rule< prevent_parameter_set_tag > const
		prevent_parameter_set("prevent_parameter_set");

	struct parameter_tag;
	x3::rule< parameter_tag, types::parse::parameter > const
		parameter("parameter");

	struct input_tag;
	x3::rule< input_tag, types::parse::io > const input("input");

	struct output_tag;
	x3::rule< output_tag, types::parse::io > const output("output");

	struct input_params_tag;
	x3::rule< input_params_tag, std::vector< types::parse::io > >
		const input_params("input_params");

	struct output_params_tag;
	x3::rule< output_params_tag, std::vector< types::parse::io > >
		const output_params("output_params");

	struct inputs_tag;
	x3::rule< inputs_tag, std::vector< types::parse::io > > const
		inputs("inputs");

	struct outputs_tag;
	x3::rule< outputs_tag, std::vector< types::parse::io > > const
		outputs("outputs");

	struct module_tag;
	x3::rule< module_tag, types::parse::module > const
		module("module");

	struct chain_tag;
	x3::rule< chain_tag, types::parse::chain > const chain("chain");

	struct chain_params_tag;
	x3::rule< chain_params_tag, std::vector< types::parse::module > >
		const chain_params("chain_params");

	struct group_tag;
	x3::rule< group_tag, std::string > const group("group");

	struct id_generator_tag;
	x3::rule< id_generator_tag, std::string > const
		id_generator("id_generator");

	struct chains_tag;
	x3::rule< chains_tag, types::parse::chains > const
		chains("chains");

	struct chains_params_tag;
	x3::rule< chains_params_tag, types::parse::chains > const
		chains_params("chains_params");

	struct config_tag;
	x3::rule< config_tag, types::parse::config >
		const config("config");


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

	auto const keyword_def =
		(char_ - space - '=' - eol) >>
		*(keyword_spaces | +(char_ - space - eol - '='))
	;

	auto const value_spaces_def =
		+(char_(' ') | char_('\t')) >> !(eol | eoi)
	;

	auto const value_def =
		(char_ - space - eol) >>
		*(value_spaces | +(char_ - space - eol))
	;

	auto const prevent_set_parameter_set_def =
		x3::expect[!("parameter_set" >> *space >> '=')]
	;

	auto const set_parameter_def =
		("\t\t" >> prevent_parameter_set) > keyword  >
		*space > '=' > *space > value > separator
	;

	auto const parameter_set_def =
		('\t' > keyword > separator) >>
		parameter_set_params
	;

	auto const parameter_set_params_def =
		x3::expect[+set_parameter]
	;

	auto const parameter_sets_def =
		(("parameter_set" > separator) >> parameter_sets_params) |
		&x3::expect["chain" >> separator]
	;

	auto const parameter_sets_params_def =
		x3::expect[+parameter_set]
	;

	auto const module_parameter_def =
		("\t\t\tparameter" > separator) >>
		module_parameter_sets >> *module_parameter
	;

	auto const module_parameter_sets_def =
		*(
			("\t\t\t\tparameter_set" >> *space >> '=')
			> *space > value > separator
		) >> &x3::expect[
			("\t\t\t\t" >> keyword) |
			("\t\t\t<-") |
			("\t\t\t->") |
			("\t\t" >> keyword) |
			("\t" >> keyword) |
			(*space >> x3::eoi)
		]
	;

	auto const prevent_parameter_set_def =
		x3::expect[!("parameter_set" >> *space >> '=')]
	;

	auto const parameter_def =
		("\t\t\t\t" >> prevent_parameter_set) > keyword  >
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

	auto const chains_def =
		(x3::expect["chain"] > separator) >>
		chains_params
	;

	auto const chains_params_def =
		x3::expect[+chain]
	;

	auto const config_def = x3::no_skip[x3::expect[
		space_lines >> -comment >>
		-parameter_sets >> chains
	]];


	struct prevent_set_parameter_set_tag: error_base{
		virtual const char* message()const override{
			return "a parameter, but a parameter name "
				"('\t\tname = value\n') must not be 'parameter_set'";
		}
	};

	struct set_parameter_tag: error_base{
		virtual const char* message()const override{
			return "a parameter '\t\tname = value\n' with name != "
				"'parameter_set'";
		}
	};

	struct chains_tag: error_base{
		virtual const char* message()const override;
	};

	struct parameter_set_tag: error_base{
		virtual const char* message()const override{
			return "a parameter '\t\tname = value\n' with name != "
				"'parameter_set'";
		}
	};

	struct parameter_set_params_tag: error_base{
		virtual const char* message()const override{
			return "at least one parameter line '\t\tname = value\n' "
				"with name != 'parameter_set'";
		}
	};

	struct parameter_sets_tag: error_base{
		template < typename Iter, typename Exception, typename Context >
		x3::error_handler_result on_error(
			Iter& first, Iter const& last,
			Exception const& x, Context const& context
		){
			msg_ = "keyword line 'parameter_set\n'";
			if(x.which() != "separator"){
				msg_ += " or ";
				msg_ += chains_tag().message();
			}
			return error_base::on_error(first, last, x, context);
		}

		virtual const char* message()const override{
			return msg_.c_str();
		}

		std::string msg_;
	};

	struct parameter_sets_params_tag: error_base{
		virtual const char* message()const override{
			return "at least one parameter set line '\tname\n'";
		}
	};

	struct module_parameter_tag: error_base{
		virtual const char* message()const override{
			return "keyword line '\t\t\tparameter\n'";
		}
	};

	struct module_parameter_sets_tag: error_base{
		template < typename Iter, typename Exception, typename Context >
		x3::error_handler_result on_error(
			Iter& first, Iter const& last,
			Exception const& x, Context const& context
		){
			msg_ = "a parameter_set reference "
				"'\t\tparameter_set = name\n', where 'parameter_set' "
				"is a keyword and 'name' the name of the referenced "
				"parameter set";
			if(x.which() != "value"){
				msg_ += " or ";
				msg_ += module_parameter_tag().message();
				msg_ += " or ";
				msg_ += chains_tag().message();
			}
			return error_base::on_error(first, last, x, context);
		}

		virtual const char* message()const override{
			return msg_.c_str();
		}

		std::string msg_;
	};

	struct modules_params_tag: error_base{
		virtual const char* message()const override{
			return "at least one module line '\tname = module\n'";
		}
	};

	struct prevent_parameter_set_tag: error_base{
		virtual const char* message()const override{
			return "a parameter, but a parameter name "
				"('\t\t\t\tname = value\n') must not be 'parameter_set'";
		}
	};

	struct parameter_tag: error_base{
		virtual const char* message()const override{
			return "a parameter '\t\t\t\tname = value\n' with name != "
				"'parameter_set'";
		}
	};

	const char* chains_tag::message()const{
		return "keyword line 'chain\n'";
	}

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
			return "keyword line '\t\t\t->\n'";
		}
	};

	struct outputs_tag: error_base{
		virtual const char* message()const override{
			return "keyword line '\t\t\t<-\n'";
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

	struct config_tag: error_base{
		virtual const char* message()const override{
			return "keyword line 'parameter_set\n' or keyword line "
				"'chain\n'";
		}
	};


	BOOST_SPIRIT_DEFINE(
		space,
		space_lines,
		separator,
		comment,
		keyword_spaces,
		keyword,
		value_spaces,
		value,
		prevent_set_parameter_set,
		set_parameter,
		prevent_parameter_set,
		parameter,
		parameter_set,
		parameter_set_params,
		parameter_sets,
		parameter_sets_params,
		module_parameter,
		module_parameter_sets,
		input,
		output,
		inputs,
		outputs,
		input_params,
		output_params,
		module,
		chain_params,
		chain,
		group,
		id_generator,
		chains_params,
		chains,
		config
	);


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
