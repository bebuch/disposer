#include <disposer/tool/ct_pretty_name.hpp>
#include <disposer/tool/type_index.hpp>

#define BOOST_TEST_MODULE dimension solve
#include <boost/test/included/unit_test.hpp>


using namespace disposer;

std::string ptp(std::string const& value){
	return purify_type_print(value);
}


template < typename T >
std::string name = type_index::type_id< T >().pretty_name();

using std::uint8_t;
using std::uint16_t;
using std::uint64_t;
using std::uint32_t;
using std::int8_t;
using std::int16_t;
using std::int64_t;
using std::int32_t;

BOOST_AUTO_TEST_CASE(self){
	BOOST_TEST(ptp("long long") == "long long");
	BOOST_TEST(ptp("long double") == "long double");
	BOOST_TEST(ptp("float") == "float32");
	BOOST_TEST(ptp("double") == "float64");

	BOOST_TEST(ptp(name< std::uint8_t >) == "uint8");
	BOOST_TEST(ptp(name< std::uint16_t >) == "uint16");
	BOOST_TEST(ptp(name< std::uint64_t >) == "uint64");
	BOOST_TEST(ptp(name< std::uint32_t >) == "uint32");
	BOOST_TEST(ptp(name< std::int8_t >) == "int8");
	BOOST_TEST(ptp(name< std::int16_t >) == "int16");
	BOOST_TEST(ptp(name< std::int64_t >) == "int64");
	BOOST_TEST(ptp(name< std::int32_t >) == "int32");
}

BOOST_AUTO_TEST_CASE(prevent_by_front){
	BOOST_TEST(ptp("_long long") == "_long " + ptp(name< long >));
	BOOST_TEST(ptp("_long double") == "_long float64");
	BOOST_TEST(ptp("_float") == "_float");
	BOOST_TEST(ptp("_double") == "_double");

	BOOST_TEST(ptp("_" + name< std::uint8_t >) == "_" + name< std::uint8_t >);
	BOOST_TEST(ptp("_" + name< std::uint16_t >) == \
"_unsigned " + ptp(name< std::int16_t >));
	BOOST_TEST(ptp("_" + name< std::uint64_t >) == \
"_unsigned " + ptp(name< std::int64_t >));
	BOOST_TEST(ptp("_" + name< std::uint32_t >) == \
"_unsigned " + ptp(name< std::int32_t >));
	BOOST_TEST(ptp("_" + name< std::int8_t >) == "_" + name< std::int8_t >);
	BOOST_TEST(ptp("_" + name< std::int16_t >) == "_" + name< std::int16_t >);
	BOOST_TEST(ptp("_" + name< std::int64_t >) == "_" + name< std::int64_t >);
	BOOST_TEST(ptp("_" + name< std::int32_t >) == "_" + name< std::int32_t >);
}

BOOST_AUTO_TEST_CASE(prevent_by_back){
	BOOST_TEST(ptp("long long_") == "int64 long_");
	BOOST_TEST(ptp("long double_") == "int64 double_");
	BOOST_TEST(ptp("float_") == "float_");
	BOOST_TEST(ptp("double_") == "double_");

	BOOST_TEST(ptp(name< std::uint8_t > + "_") == name< std::uint8_t > + "_");
	BOOST_TEST(ptp(name< std::uint16_t > + "_") == name< std::uint16_t > + "_");
	BOOST_TEST(ptp(name< std::uint64_t > + "_") == name< std::uint64_t > + "_");
	BOOST_TEST(ptp(name< std::uint32_t > + "_") == name< std::uint32_t > + "_");
	BOOST_TEST(ptp(name< std::int8_t > + "_") == name< std::int8_t > + "_");
	BOOST_TEST(ptp(name< std::int16_t > + "_") == name< std::int16_t > + "_");
	BOOST_TEST(ptp(name< std::int64_t > + "_") == name< std::int64_t > + "_");
	BOOST_TEST(ptp(name< std::int32_t > + "_") == name< std::int32_t > + "_");
}

BOOST_AUTO_TEST_CASE(prevent_by_both){
	BOOST_TEST(ptp("_long long_") == "_long long_");
	BOOST_TEST(ptp("_long double_") == "_long double_");
	BOOST_TEST(ptp("_float_") == "_float_");
	BOOST_TEST(ptp("_double_") == "_double_");

	BOOST_TEST(ptp("_" + name< std::uint8_t > + "_") == \
"_" + name< std::uint8_t > + "_");
	BOOST_TEST(ptp("_" + name< std::uint16_t > + "_") == \
"_" + name< std::uint16_t > + "_");
	BOOST_TEST(ptp("_" + name< std::uint64_t > + "_") == \
"_" + name< std::uint64_t > + "_");
	BOOST_TEST(ptp("_" + name< std::uint32_t > + "_") == \
"_" + name< std::uint32_t > + "_");
	BOOST_TEST(ptp("_" + name< std::int8_t > + "_") == \
"_" + name< std::int8_t > + "_");
	BOOST_TEST(ptp("_" + name< std::int16_t > + "_") == \
"_" + name< std::int16_t > + "_");
	BOOST_TEST(ptp("_" + name< std::int64_t > + "_") == \
"_" + name< std::int64_t > + "_");
	BOOST_TEST(ptp("_" + name< std::int32_t > + "_") == \
"_" + name< std::int32_t > + "_");
}
