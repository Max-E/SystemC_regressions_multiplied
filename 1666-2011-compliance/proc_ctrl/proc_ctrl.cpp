//******************************************************************************
//
//  The following code is derived, directly or indirectly, from the SystemC
//  source code Copyright (c) 1996-2014 by all Contributors.
//  All Rights reserved.
//
//  The contents of this file are subject to the restrictions and limitations
//  set forth in the SystemC Open Source License (the "License");
//  You may not use this file except in compliance with such restrictions and
//  limitations. You may obtain instructions on how to receive a copy of the
//  License at http://www.accellera.org/. Software distributed by Contributors
//  under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF
//  ANY KIND, either express or implied. See the License for the specific
//  language governing rights and limitations under the License.
//******************************************************************************

// disable_enable.cpp -- test for 
//
//  Original Author: John Aynsley, Doulos Inc.
//
// MODIFICATION LOG - modifiers, enter your name, affiliation, date and
//
// $Log: disable_enable.cpp,v $
// Revision 1.2  2011/05/08 19:18:46  acg
//  Andy Goodrich: remove extraneous + prefixes from git diff.
//

// disable() should take effect only the next evaluation phase

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <systemc>

using namespace sc_core;
using namespace std;

struct Top: sc_module
{
  Top(sc_module_name _name)
  : count(0)
  {
    SC_THREAD(control);
    SC_THREAD(target);
      t = sc_get_current_process_handle();
      
    f0 = f1 = f2 = f3 = f4 = f5 = 0;
  }
  
  sc_process_handle t;
  sc_event ev;
  int count;
  int f0, f1, f2, f3, f4, f5;

  void control()
  {
    wait(SC_ZERO_TIME);
    
    count = 1;
    ev.notify();
    wait(10, SC_NS);
    
    count = 2;
    ev.notify();
    t.disable();
    wait(SC_ZERO_TIME);
    
    count = 3;
    ev.notify();
    wait(SC_ZERO_TIME);
    
    count = 4;
    t.enable();
    ev.notify();
    wait(10, SC_NS);

    count = 5;
    t.disable();
    t.reset();
    wait(10, SC_NS);
    
    count = 6;
    t.reset();
    wait(10, SC_NS);
   
    if (!strcmp (name(), "top0"))
    {
        wait(20, SC_NS);
        sc_stop();
    }
  }

  void target()
  {
    //cout << "Target called at " << sc_time_stamp() << " with count = " << count << endl;
    switch(count)
    {
        case  0: sc_assert(sc_time_stamp() == sc_time( 0, SC_NS)); f0 = 1; break;
        case  5: sc_assert(sc_time_stamp() == sc_time(20, SC_NS)); f4 = 1; break;
        case  6: sc_assert(sc_time_stamp() == sc_time(30, SC_NS)); f5 = 1; break;
        default: sc_assert(false);
    }
    for (;;)
    {
      wait(ev);
      //cout << "Target awoke at " << sc_time_stamp() << " with count = " << count << endl;
      switch(count)
      {
        case  1: sc_assert(sc_time_stamp() == sc_time( 0, SC_NS)); f1 = 1; break;
        case  2: sc_assert(sc_time_stamp() == sc_time(10, SC_NS)); f2 = 1; break;
        case  4: sc_assert(sc_time_stamp() == sc_time(10, SC_NS)); f3 = 1; break;
        default: sc_assert(false);
      }
    }
  }
  
  SC_HAS_PROCESS(Top);
};

int sc_main(int argc, char* argv[])
{
  Top *tops[128];
  for (int i = 0; i < 128; i++)
  {
    char name[32];
    snprintf (name, 32, "top%d", i);
    tops[i] = new Top (name);
  }
  
  sc_start();
  
  for (int i = 0; i < 128; i++)
  {
    sc_assert (tops[i]->f0);
    sc_assert (tops[i]->f1);
    sc_assert (tops[i]->f2);
    sc_assert (tops[i]->f3);
    sc_assert (tops[i]->f4);
    sc_assert (tops[i]->f5);
  }
  cout << endl << "Success" << endl;
  return 0;
}
  
