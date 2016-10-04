#ifndef _AMANZI_EXCEPTIONS_H_
#define _AMANZI_EXCEPTIONS_H_

#include <string>
#include <stdlib.h>

namespace Exceptions {

// A Base exception class for exceptions generated by the Amanzi code.
class Amanzi_exception : public std::exception {
};

// Enumerates possible ways to handle exceptions.
enum Exception_action 
{ 
  RAISE, 
  ABORT 
};

// Functions for setting and querying the current exception behavior.
void set_exception_behavior_raise();
void set_exception_behavior_abort();
void set_exception_behavior(Exception_action);
Exception_action exception_behavior();

}  // namespace Exceptions


namespace {
Exceptions::Exception_action behavior = Exceptions::RAISE;
}


namespace Exceptions {

// Either raise the given exception or abort, depending on the value of behavior.
template <typename E>
void amanzi_throw(const E& exception)
{
  if (behavior == Exceptions::RAISE)
      throw exception;
  else
      abort ();
}

} // exceptions error

#endif 
