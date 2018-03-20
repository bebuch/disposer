#include <disposer/tool/ct_pretty_name.hpp>
#include <disposer/tool/type_index.hpp>

#define BOOST_TEST_MODULE disposer ct_pretty_name
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


BOOST_AUTO_TEST_CASE(basic){
	BOOST_TEST(name< std::int8_t > != name< char >);
	BOOST_TEST(name< std::uint8_t > != name< char >);
	BOOST_TEST(ptp("char") == "char");
	BOOST_TEST(ptp("float") == "float32");
	BOOST_TEST(ptp("double") == "float64");
}

BOOST_AUTO_TEST_CASE(self){
	BOOST_TEST(ptp("long long") == "long long");
	BOOST_TEST(ptp("long double") == "long double");

	BOOST_TEST(ptp(name< std::uint8_t >) == "uint8");
	BOOST_TEST(ptp(name< std::uint16_t >) == "uint16");
	BOOST_TEST(ptp(name< std::uint32_t >) == "uint32");
	BOOST_TEST(ptp(name< std::uint64_t >) == "uint64");
	BOOST_TEST(ptp(name< std::int8_t >) == "int8");
	BOOST_TEST(ptp(name< std::int16_t >) == "int16");
	BOOST_TEST(ptp(name< std::int32_t >) == "int32");
	BOOST_TEST(ptp(name< std::int64_t >) == "int64");
}

struct _float{};
struct _double{};
struct _char{};
struct _short{};
struct _int{};
struct _long{};
struct _signed{};
struct _unsigned{};
struct _const{};
struct _volatile{};

struct float_{};
struct double_{};
struct char_{};
struct short_{};
struct int_{};
struct long_{};
struct signed_{};
struct unsigned_{};
struct const_{};
struct volatile_{};

struct _float_{};
struct _double_{};
struct _char_{};
struct _short_{};
struct _int_{};
struct _long_{};
struct _signed_{};
struct _unsigned_{};
struct _const_{};
struct _volatile_{};

BOOST_AUTO_TEST_CASE(prevent_by_front){
	BOOST_TEST(ptp(name< _float >) == "_float");
	BOOST_TEST(ptp(name< _double >) == "_double");
	BOOST_TEST(ptp(name< _char >) == "_char");
	BOOST_TEST(ptp(name< _short >) == "_short");
	BOOST_TEST(ptp(name< _int >) == "_int");
	BOOST_TEST(ptp(name< _long >) == "_long");
	BOOST_TEST(ptp(name< _signed >) == "_signed");
	BOOST_TEST(ptp(name< _unsigned >) == "_unsigned");
	BOOST_TEST(ptp(name< _const >) == "_const");
	BOOST_TEST(ptp(name< _volatile >) == "_volatile");
}

BOOST_AUTO_TEST_CASE(prevent_by_back){
	BOOST_TEST(ptp(name< float_ >) == "float_");
	BOOST_TEST(ptp(name< double_ >) == "double_");
	BOOST_TEST(ptp(name< char_ >) == "char_");
	BOOST_TEST(ptp(name< short_ >) == "short_");
	BOOST_TEST(ptp(name< int_ >) == "int_");
	BOOST_TEST(ptp(name< long_ >) == "long_");
	BOOST_TEST(ptp(name< signed_ >) == "signed_");
	BOOST_TEST(ptp(name< unsigned_ >) == "unsigned_");
	BOOST_TEST(ptp(name< const_ >) == "const_");
	BOOST_TEST(ptp(name< volatile_ >) == "volatile_");
}

BOOST_AUTO_TEST_CASE(prevent_by_both){
	BOOST_TEST(ptp(name< _float_ >) == "_float_");
	BOOST_TEST(ptp(name< _double_ >) == "_double_");
	BOOST_TEST(ptp(name< _char_ >) == "_char_");
	BOOST_TEST(ptp(name< _short_ >) == "_short_");
	BOOST_TEST(ptp(name< _int_ >) == "_int_");
	BOOST_TEST(ptp(name< _long_ >) == "_long_");
	BOOST_TEST(ptp(name< _signed_ >) == "_signed_");
	BOOST_TEST(ptp(name< _unsigned_ >) == "_unsigned_");
	BOOST_TEST(ptp(name< _const_ >) == "_const_");
	BOOST_TEST(ptp(name< _volatile_ >) == "_volatile_");
}
