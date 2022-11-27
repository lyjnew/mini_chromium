#include <iostream>
using namespace std;

#include "winbase\strings\string16.h"
#include "winbase\strings\string_number_conversions.h"
#include "winbase\strings\utf_string_conversions.h"
#include "winbase\memory\ref_counted.h"
#include "winbase\memory\scoped_refptr.h"
#include "winbase\functional\bind.h"
#include "winbase\functional\cancelable_callback.h"

class Foo : public winbase::RefCounted<Foo> {
 public:
   void Set(int v) { value_ = v; };

 private:
  friend class winbase::RefCounted<Foo>;
  ~Foo() {};

  int value_{1};
};

int main(int argc, char* argv[]) {
  std::string str = "3.1415926";
  winbase::string16 str2 = L"5.674";

  winbase::scoped_refptr<Foo> ptr = winbase::MakeRefCounted<Foo>();
  ptr->Set(5555);

  ptr.reset();

  winbase::Closure func = winbase::Bind(&Foo::Set, ptr.get(), 1);
  
  double d1, d2; 
  winbase::StringToDouble(str, &d1);
  winbase::StringToDouble(str2, &d2);
  return 0;
}