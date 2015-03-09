#include "TypeTraitsTest.h"
#include "ITOTypeTraits.h"
#include "Tracer.h"

using namespace std;

void TypeTraitsTest::TraitsTest() {
	StartTrace(TypeTraitsTest.TraitsTest);
	{
		typedef String TestType;
		ASSERTM("expected non-pointer type", coast::typetraits::TypeTraits<TestType>::isPointer == false);
		ASSERTM("expected non-const type", coast::typetraits::TypeTraits<TestType>::isConst == false);
		ASSERTM("expected coast::typetraits::NullType",
				typeid(coast::typetraits::TypeTraits<TestType>::PointeeType) == typeid(coast::typetraits::NullType));
		ASSERTM("expected String-type", typeid(coast::typetraits::TypeTraits<TestType>::NonConstType) == typeid(String));
		ASSERTM("expected non String-ptr-type",
				coast::typetraits::TypeTraits<coast::typetraits::TypeTraits<TestType>::NonConstType>::isPointer == false);
		ASSERTM("expected String-type", typeid(coast::typetraits::fooTypeTraits<TestType>::PlainType) == typeid(String));
	}
	{
		typedef const String TestType;
		ASSERTM("expected non-pointer type", coast::typetraits::TypeTraits<TestType>::isPointer == false);
		ASSERTM("expected const type", coast::typetraits::TypeTraits<TestType>::isConst == true);
		ASSERTM("expected coast::typetraits::NullType",
				typeid(coast::typetraits::TypeTraits<TestType>::PointeeType) == typeid(coast::typetraits::NullType));
		ASSERTM("expected String-type", typeid(coast::typetraits::TypeTraits<TestType>::NonConstType) == typeid(String));
		ASSERTM("expected non String-ptr-type",
				coast::typetraits::TypeTraits<coast::typetraits::TypeTraits<TestType>::NonConstType>::isPointer == false);
		ASSERTM("expected String-type", typeid(coast::typetraits::fooTypeTraits<TestType>::PlainType) == typeid(String));
	}
	{
		typedef String *TestType;
		ASSERTM("expected pointer type", coast::typetraits::TypeTraits<TestType>::isPointer == true);
		ASSERTM("expected non-const type", coast::typetraits::TypeTraits<TestType>::isConst == false);
		ASSERTM("expected String-type", typeid(coast::typetraits::TypeTraits<TestType>::PointeeType) == typeid(String));
		ASSERTM("expected String-ptr-type", typeid(coast::typetraits::TypeTraits<TestType>::NonConstType) == typeid(String *));
		ASSERTM("expected String-ptr-type", coast::typetraits::TypeTraits<coast::typetraits::TypeTraits<TestType>::NonConstType>::isPointer == true);
		ASSERTM("expected String-type", typeid(coast::typetraits::fooTypeTraits<TestType>::PlainType) == typeid(String));
	}
	{
		typedef const String *TestType;
		ASSERTM("expected pointer type", coast::typetraits::TypeTraits<TestType>::isPointer == true);
		ASSERTM("expected const type", coast::typetraits::TypeTraits<TestType>::isConst == false);
		ASSERTM("expected String-type", typeid(coast::typetraits::TypeTraits<TestType>::PointeeType) == typeid(String));
		ASSERTM("expected String-ptr-type", typeid(coast::typetraits::TypeTraits<TestType>::NonConstType) == typeid(const String *));
		ASSERTM("expected String-ptr-type", coast::typetraits::TypeTraits<coast::typetraits::TypeTraits<TestType>::NonConstType>::isPointer == true);
		ASSERTM("expected String-type", typeid(coast::typetraits::fooTypeTraits<TestType>::PlainType) == typeid(String));
	}
	{
		typedef String * const TestType;
		ASSERTM("expected const type", coast::typetraits::TypeTraits<TestType>::isConst == true);
		ASSERTM("expected String-ptr-type", typeid(coast::typetraits::TypeTraits<TestType>::NonConstType) == typeid(String *));
		ASSERTM("expected String-ptr-type", coast::typetraits::TypeTraits<coast::typetraits::TypeTraits<TestType>::NonConstType>::isPointer == true);
		ASSERTM("expected String-type", typeid(coast::typetraits::fooTypeTraits<TestType>::PlainType) == typeid(String));
	}
	{
		typedef String &TestType;
		ASSERTM("expected non-pointer type", coast::typetraits::TypeTraits<TestType>::isPointer == false);
		ASSERTM("expected non-const type", coast::typetraits::TypeTraits<TestType>::isConst == false);
		ASSERTM("expected coast::typetraits::NullType",
				typeid(coast::typetraits::TypeTraits<TestType>::PointeeType) == typeid(coast::typetraits::NullType));
		ASSERTM("expected String-ref-type", typeid(coast::typetraits::TypeTraits<TestType>::NonConstType) == typeid(String &));
		ASSERTM("expected non String-ptr-type",
				coast::typetraits::TypeTraits<coast::typetraits::TypeTraits<TestType>::NonConstType>::isPointer == false);
		ASSERTM("expected String-type", typeid(coast::typetraits::fooTypeTraits<TestType>::PlainType) == typeid(String));
	}
	{
		typedef const String &TestType;
		ASSERTM("expected non-pointer type", coast::typetraits::TypeTraits<TestType>::isPointer == false);
		ASSERTM("expected const type", coast::typetraits::TypeTraits<TestType>::isConst == true);
		ASSERTM("expected coast::typetraits::NullType",
				typeid(coast::typetraits::TypeTraits<TestType>::PointeeType) == typeid(coast::typetraits::NullType));
		ASSERTM("expected String-ref-type", typeid(coast::typetraits::TypeTraits<TestType>::NonConstType) == typeid(String &));
		ASSERTM("expected non String-ptr-type",
				coast::typetraits::TypeTraits<coast::typetraits::TypeTraits<TestType>::NonConstType>::isPointer == false);
		ASSERTM("expected String-type", typeid(coast::typetraits::fooTypeTraits<TestType>::PlainType) == typeid(String));
	}
}

template<typename InnerType>
class ClassTraits {
public:
	typedef typename coast::typetraits::fooTypeTraits<InnerType>::ConstPlainTypeRef ConstPlainTypeRef;
	typedef typename coast::typetraits::fooTypeTraits<InnerType>::PlainTypePtr PlainTypePtr;

	ClassTraits(ConstPlainTypeRef aParam) :
			fField(aParam) {
		StartTrace1(ClassTraits.ClassTraits, "ConstPlainTypeRef");
		SomeMethod(aParam);
	}
	ClassTraits(PlainTypePtr pParam) :
			fField(pParam) {
		StartTrace1(ClassTraits.ClassTraits, "PlainTypePtr");
		SomeMethod(pParam);
	}

	void SomeMethod(ConstPlainTypeRef aParam) {
		StartTrace1(ClassTraits.SomeMethod, "ConstPlainTypeRef")
		;
	}

	void SomeMethod(PlainTypePtr pParam) {
		StartTrace1(ClassTraits.SomeMethod, "PlainTypePtr")
		;
	}

	const type_info &GetTypeInfo() const {
		return typeid(fField);
			}

		private:
			InnerType fField;
		};

		void TypeTraitsTest::ClassTraitsTest() {
			StartTrace(TypeTraitsTest.ClassTraitsTest);
			{
				typedef const String &TestType;
				String aTestString;
				ClassTraits<TestType> aClass(aTestString);
				ASSERT(aClass.GetTypeInfo() == typeid(TestType));
			}
			{
				typedef String *TestType;
				String aTestString;
				ClassTraits<TestType> aClass(&aTestString);
				ASSERT(aClass.GetTypeInfo() == typeid(TestType));
			}
		}

		// builds up a suite of testcases, add a line for each testmethod
		void TypeTraitsTest::runAllTests(int argc, char const *argv[]) {
			cute::suite s;
			s.push_back(CUTE_SMEMFUN(TypeTraitsTest, TraitsTest));
			s.push_back(CUTE_SMEMFUN(TypeTraitsTest, ClassTraitsTest));
			cute::ide_listener<> lis;
			cute::makeRunner(lis, argc, argv)(s, "AllTests");
		}
