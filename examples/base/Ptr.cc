//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some example code for using emp::Ptr

#include <iostream>
#include <string>

// EMP_TRACK_MEM must be defined in order to use the extra capabilities of emp::Ptr.
// Normally this would be done at the command line with -DEMP_TRACK_MEM
#ifndef EMP_TRACK_MEM
#define EMP_TRACK_MEM
#endif

#include "../../base/Ptr.h"
#include "../../base/vector.h"

struct TestBase {
  int x;
  TestBase(int _x) : x(_x) { ; }
  virtual ~TestBase() { ; }
  virtual int Val() { return x; }
};
struct TestDerived : public TestBase {
  TestDerived(int _x) : TestBase(_x) { ; }
  int Val(){return 2*x;}
};

int main()
{
  std::string test_str = "Test String!";
  std::cout << test_str << std::endl;

  emp::Ptr<std::string> test_ptr(&test_str);   // false -> don't track!
  std::cout << "From Ptr: " << *test_ptr << std::endl;
  std::cout << "  with size = " << test_ptr->size() << std::endl;

  {
    emp::Ptr<std::string> test_ptr2(&test_str);
  }

  emp::Ptr<int> int_ptr;
  int_ptr.New(123456);
  std::cout << "*int_ptr = " << *int_ptr << std::endl;
  int_ptr.Delete();
  std::cout << "Deleted int_ptr." << std::endl;


  // Examples with base and derived classes.
  emp::Ptr<TestBase> ptr_base = new TestDerived(5);
  emp::Ptr<TestDerived> ptr_derived(ptr_base.Cast<TestDerived>());



  // Examples with arrays!
  int_ptr.NewArray(20);                    // Reuse int_ptr, this time as an array.
  for (size_t i = 0; i < 20; i++) {
    int_ptr[i] = (int) (i+100);
  }
  std::cout << "Array contents:";
  for (size_t i = 0; i < 20; i++) {
    std::cout << " " << int_ptr[i];
  }
  std::cout << std::endl;
  int_ptr.DeleteArray();


  emp::vector< emp::Ptr<char> > ptr_v(26);
  for (size_t i = 0; i < 26; i++) {
    ptr_v[i] = emp::NewPtr<char>((char)('A' + (char) i));
  }
  ptr_v.resize(100);
  for (size_t i = 0; i < 26; i++) {
    ptr_v[i].Delete();
  }


  // -- Interactions between Ptr and emp::vector --

  // create a vector of pointers
  emp::vector<emp::Ptr<char>> v_ptr(26);
  for (size_t i = 0; i < v_ptr.size(); i++) {
    v_ptr[i] = emp::NewPtr<char>('A' + i);
  }
  // print the pointer contents.
  std::cout << "Chars: ";
  for (size_t i = 0; i < v_ptr.size(); i++) {
    std::cout << *v_ptr[i];
  }
  std::cout << std::endl;

  emp::vector<emp::Ptr<char>> v_ptr2;
  std::swap(v_ptr, v_ptr2);

  // and delete the pointers.
  for (size_t i = 0; i < v_ptr2.size(); i++) {
    v_ptr2[i].Delete();
  }


  std::cout << "End of main()." << std::endl;
}