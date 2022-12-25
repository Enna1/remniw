static int InterceptorFunctionCalled;

typedef int (*isdigit_type)(int);
namespace __interception { extern isdigit_type real_isdigit; };

typedef int (*isdigit_type)(int d);
namespace __interception { isdigit_type real_isdigit; }
extern "C" int isdigit(int d) __attribute__((weak, alias("__interceptor_" "isdigit"), visibility("default")));
extern "C" __attribute__((visibility("default"))) int __interceptor_isdigit(int d) {
  ++InterceptorFunctionCalled;
  return d >= '0' && d <= '9';
}

::__interception::InterceptFunction( "isdigit", (::__interception::uptr *) & __interception::real_isdigit, (::__interception::uptr) & (isdigit), (::__interception::uptr) & __interceptor_isdigit)