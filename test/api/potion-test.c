//
// potion-test.c
// tests of the Potion C api
//
// (c) 2008 why the lucky stiff, the freelance professor
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "potion.h"
#include "internal.h"
#include "CuTest.h"

PN num = PN_NUM(490);
Potion *P;

void potion_test_nil(CuTest *T) {
  CuAssert(T, "nil isn't a nil type", PN_TYPE(PN_NIL) == PN_TNIL);
  CuAssert(T, "nil is a ref", !PN_IS_PTR(PN_NIL));
  CuAssert(T, "nil nil? is false",
    PN_TRUE == potion_send(PN_NIL, potion_str(P, "nil?")));
}

void potion_test_bool(CuTest *T) {
  CuAssert(T, "true isn't a bool type", PN_TYPE(PN_TRUE) == PN_TBOOLEAN);
  CuAssert(T, "true is a ref", !PN_IS_PTR(PN_TRUE));
  CuAssert(T, "false isn't a bool type", PN_TYPE(PN_FALSE) == PN_TBOOLEAN);
  CuAssert(T, "false is a ref", !PN_IS_PTR(PN_FALSE));
}

void potion_test_int1(CuTest *T) {
  PN zero = PN_NUM(0);
  CuAssert(T, "zero isn't zero", PN_INT(zero) == 0);
  CuAssert(T, "zero isn't a number", PN_IS_NUM(zero));
  CuAssert(T, "zero is a ref", !PN_IS_PTR(zero));
  CuAssert(T, "zero bad add",
    490 == PN_INT(potion_send(zero, potion_str(P, "+"), num)));
}

void potion_test_int2(CuTest *T) {
  PN pos = PN_NUM(10891);
  CuAssert(T, "positive numbers invalid", PN_INT(pos) == 10891);
  CuAssert(T, "positive not a number", PN_IS_NUM(pos));
  CuAssert(T, "positive is a ref", !PN_IS_PTR(pos));
  CuAssert(T, "positive bad add",
    11381 == PN_INT(potion_send(pos, potion_str(P, "+"), num)));
}

void potion_test_int3(CuTest *T) {
  PN neg = PN_NUM(-4343);
  CuAssert(T, "negative numbers invalid", PN_INT(neg) == -4343);
  CuAssert(T, "negative not a number", PN_IS_NUM(neg));
  CuAssert(T, "negative is a ref", !PN_IS_PTR(neg));
  CuAssert(T, "negative bad add",
    -3853 == PN_INT(potion_send(neg, potion_str(P, "+"), num)));
}

void potion_test_decimal(CuTest *T) {
  PN dec = potion_decimal(P, "14466", 5);
  CuAssert(T, "decimal not a number", PN_TYPE(dec) == PN_TNUMBER);
}

void potion_test_str(CuTest *T) {
  CuAssert(T, "string isn't a string", PN_IS_STR(PN_string));
  CuAssert(T, "string isn't a ref", PN_IS_PTR(PN_string));
  CuAssert(T, "string length isn't working",
    6 == PN_INT(potion_send(PN_string, potion_str(P, "length"))));
}

void potion_test_empty(CuTest *T) {
  PN empty = PN_TUP0();
  CuAssert(T, "empty isn't a tuple", PN_IS_TUPLE(empty));
  CuAssert(T, "empty isn't a ref", PN_IS_PTR(empty));
  CuAssertIntEquals(T, "tuple length is off",
    PN_INT(potion_send(empty, potion_str(P, "length"))), 0);
}

void potion_test_tuple(CuTest *T) {
  PN tup = potion_tuple_with_size(P, 3);
  PN_TUPLE_AT(tup, 0) = PN_NIL;
  PN_TUPLE_AT(tup, 1) = PN_string;
  PN_TUPLE_AT(tup, 2) = tup;
  CuAssert(T, "tuple isn't a tuple", PN_IS_TUPLE(tup));
  CuAssert(T, "tuple isn't a ref", PN_IS_PTR(tup));
  CuAssertIntEquals(T, "tuple length is off",
    PN_INT(potion_send(tup, potion_str(P, "length"))), 3);
}

void potion_test_sig(CuTest *T) {
  // test the simple parser entry point yy_sig, not the compiler transformation potion_sig_compile
  PN sig = potion_sig(P, "num1=N,num2=N");
  CuAssert(T, "signature isn't a tuple", PN_IS_TUPLE(sig));
  CuAssertIntEquals(T, "len=2", 2, PN_INT(PN_TUPLE_LEN(sig)));
  CuAssertIntEquals(T, "arity=2", 2, potion_sig_arity(P, sig));
  CuAssertStrEquals(T, "num1=N,num2=N", //roundtrip
		    PN_STR_PTR(potion_sig_string(P,0,sig)));
  CuAssertStrEquals(T, "(num1, 78, num2, 78)",
		    PN_STR_PTR(potion_send(sig, PN_string)));
  CuAssertStrEquals(T, "num1",
		    PN_STR_PTR(potion_send(PN_TUPLE_AT(sig,0), PN_string)));
  CuAssertIntEquals(T, "num1=N", 'N',
		    PN_INT(PN_TUPLE_AT(sig,1)));
  CuAssertStrEquals(T, "num2",
		    PN_STR_PTR(potion_send(PN_TUPLE_AT(sig,2), PN_string)));
  CuAssertIntEquals(T, "num2=N", 'N',
		    PN_INT(PN_TUPLE_AT(sig,3)));

  sig = potion_sig(P, "x=N,y=N|r=N");
  CuAssert(T, "signature isn't a tuple", PN_IS_TUPLE(sig));
  CuAssertStrEquals(T, "(x, 78, y, 78, 124, r, 78)",
		    PN_STR_PTR(potion_send(sig, PN_string)));
  CuAssertStrEquals(T, "x=N,y=N|r=N",
		    PN_STR_PTR(potion_sig_string(P,0,sig)));
  {
    // roundtrips
    char *sigs[] = {
      "", "x,y", "x", "x=N", "x,y", "x=N,y=o",
      "x|y", "x|y,z", "x=o|y,z", "x|y=o", "x=N,y=N|r=N", /*optional */
      "x:=1", "|x:=1", "x|y:=0", /* defaults */
      "x,y.z", /* the dot */
    };
    int size = sizeof(sigs)/sizeof(char *);
    int i;
    for (i=0; i< size; i++) {
      CuAssertStrEquals(T, sigs[i],
			PN_STR_PTR(potion_sig_string(P,0,potion_sig(P, sigs[i]))));
    }
  }
}

void potion_test_proto(CuTest *T) {
  // test compiler transformation potion_sig_compile, not just yy_sig
  vPN(Closure) f1 = PN_CLOSURE(potion_eval(P, potion_str(P, "(x,y):x+y."), POTION_JIT));
  vPN(Closure) f2 = PN_CLOSURE(PN_FUNC(PN_CLOSURE_F(f1), "x=N,y=N"));
  CuAssertIntEquals(T, "arity f1", 2, potion_sig_arity(P, f1->sig));
  CuAssertIntEquals(T, "arity f2", 2, potion_sig_arity(P, f2->sig));
  //vPN(Source) src = (vPN(Source)) potion_parse(P, potion_str(P, "(x,y):x+y."), "<eval>");
  //CuAssert(T, "parse fun", PN_TYPE(src) == PN_TSOURCE);
  {
    //vPN(Proto) proto = PN_PROTO(potion_send(src, PN_compile, PN_NIL, PN_NIL));
    //vPN(Tuple) sig = proto->sig;
  }
}

void potion_test_eval(CuTest *T) {
  PN add = potion_eval(P, potion_str(P, "(x, y): x + y."), POTION_JIT);
  PN_F addfn = PN_CLOSURE_F(add); // c callback
  PN num = addfn(P, add, 0, PN_NUM(3), PN_NUM(5));
  CuAssertIntEquals(T, "calling closure as c func failed",
    PN_INT(num), 8);
}

void potion_test_allocated(CuTest *T) {
  void *scanptr = (void *)((char *)P->mem->birth_lo + PN_ALIGN(sizeof(struct PNMemory), 8));
  while ((PN)scanptr < (PN)P->mem->birth_cur) {
    if (((struct PNFwd *)scanptr)->fwd != POTION_FWD && ((struct PNFwd *)scanptr)->fwd != POTION_COPIED) {
      CuAssert(T, "wrong type for allocated object", ((struct PNObject *)scanptr)->vt <= PN_TUSER);
    }
    scanptr = (void *)((char *)scanptr + potion_type_size(P, scanptr));
    CuAssert(T, "allocated object goes beyond GC pointer", (PN)scanptr <= (PN)P->mem->birth_cur);
  }
}

CuSuite *potion_suite() {
  CuSuite *S = CuSuiteNew();
  SUITE_ADD_TEST(S, potion_test_nil);
  SUITE_ADD_TEST(S, potion_test_bool);
  SUITE_ADD_TEST(S, potion_test_int1);
  SUITE_ADD_TEST(S, potion_test_int2);
  SUITE_ADD_TEST(S, potion_test_int3);
  SUITE_ADD_TEST(S, potion_test_decimal);
  SUITE_ADD_TEST(S, potion_test_str);
  SUITE_ADD_TEST(S, potion_test_empty);
  SUITE_ADD_TEST(S, potion_test_tuple);
  SUITE_ADD_TEST(S, potion_test_sig);
  SUITE_ADD_TEST(S, potion_test_proto);
  SUITE_ADD_TEST(S, potion_test_eval);
  SUITE_ADD_TEST(S, potion_test_allocated);
  return S;
}

int main(void) {
  POTION_INIT_STACK(sp);
  int count;
  P = potion_create(sp);
  CuString *out = CuStringNew();
  CuSuite *suite = potion_suite();
  CuSuiteRun(suite);
  CuSuiteSummary(suite, out);
  CuSuiteDetails(suite, out);
  printf("%s\n", out->buffer);
  count = suite->failCount;
  CuSuiteFree(suite);
  CuStringFree(out);
  return count;
}
