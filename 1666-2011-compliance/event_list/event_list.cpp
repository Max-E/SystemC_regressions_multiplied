
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

// event_list.cpp -- test for 
//
//  Original Author: John Aynsley, Doulos, Inc.
//
// MODIFICATION LOG - modifiers, enter your name, affiliation, date and
//
// $Log: event_list.cpp,v $
// Revision 1.3  2011/09/05 21:23:30  acg
//  Philipp A. Hartmann: eliminate compiler warnings.
//
// Revision 1.2  2011/05/08 19:18:46  acg
//  Andy Goodrich: remove extraneous + prefixes from git diff.
//

// Event lists. A thread waits on a list of events built dynamically from a multiport

#include <systemc>

using namespace sc_core;
using std::cout;
using std::endl;

struct Mod: sc_module
{
  sc_port<sc_signal_in_if<int>, 0> p; // Multiport
  
  Mod(sc_module_name _name)
  {
    SC_THREAD(T1);
    SC_THREAD(T2);
  }
  
  void T1()
  {
    wait(all_events());
    for (int i = 0; i < 6; i++)
    {
      sc_assert (sc_time_stamp () == sc_time ((i + 1) * 10, SC_NS));
      if (i == 0)
        sc_assert (p[0]->read () == 1);
      else
        sc_assert (p[0]->read () == 2);
      if (i >= 3)
        sc_assert (p[1]->read () == 2);
      else if (i >= 2)
        sc_assert (p[1]->read () == 1);
      else
        sc_assert (p[1]->read () == 0);
      if (i >= 3)
        sc_assert (p[2]->read () == i - 3);
      else
        sc_assert (p[2]->read () == 0);
      wait(all_events());
    }
    sc_assert (false);
  }
  void T2()
  {
    wait( p[0]->default_event() | p[1]->default_event() );
    for (int i = 0; i < 4; i++)
    {
      sc_assert (sc_time_stamp () == sc_time ((i + 1) * 10, SC_NS));
      if (i == 0)
        sc_assert (p[0]->read () == 1);
      else
        sc_assert (p[0]->read () == 2);
      if (i == 3)
        sc_assert (p[1]->read () == 2);
      else if (i == 2)
        sc_assert (p[1]->read () == 1);
      else
        sc_assert (p[1]->read () == 0);
      sc_assert (p[2]->read () == 0);
      wait( p[0]->default_event() | p[1]->default_event() );
    }
    sc_assert (false);
  }
  sc_event_or_list all_events() const
  {
    sc_assert( p.size() == 3 );
    
    sc_event_or_list or_list;
    for (int i = 0; i < p.size(); i++)
      or_list |= p[i]->default_event();
      
    sc_assert( or_list.size() == 3 );
    return or_list;
  }  
 
  SC_HAS_PROCESS(Mod);
};

struct Top: sc_module
{
  Top(sc_module_name _name)
  : finished(false)
  , count(0)
  {
    m = new Mod("m");
    m->p.bind(sig1);
    m->p.bind(sig2);
    m->p.bind(sig3);
    SC_THREAD(T);
    SC_METHOD(M);
  }
  
  ~Top()
  {
    sc_assert( finished ); 
    sc_assert( count == 4 );
    sc_assert( sc_get_status() == SC_PAUSED );
    sc_assert( sc_is_running() == true );
  }
  
  sc_signal<int> sig1, sig2, sig3;
  Mod* m;
  
  sc_event e1, e2, e3, e4;
  bool finished;
  
  void T()
  {
    sig1.write(0);
    sig2.write(0);
    sig3.write(0);

    wait(10, SC_NS);
    sig1.write(1);
    wait(10, SC_NS);
    sig1.write(2);
   
    wait(10, SC_NS);
    sig2.write(1);
    wait(10, SC_NS);
    sig2.write(2);
   
    wait(10, SC_NS);
    sig3.write(1);
    wait(10, SC_NS);
    sig3.write(2);

    sc_time start;
    start = sc_time_stamp();
   
    e1.notify(10, SC_NS);
    e2.notify(20, SC_NS);
    
    sc_event_and_list list1 = e1 & e2;
    sc_assert( list1.size() == 2 );
    
    wait( list1 );
    sc_assert( sc_time_stamp() - start == sc_time(20, SC_NS) );
    
    start = sc_time_stamp();
    e1.notify(11, SC_NS);
    e2.notify(21, SC_NS);
    
    sc_event_and_list list2;
    list2 &= e1;
    list2 &= e2;
    sc_assert( list2.size() == 2 );
    
    wait( list2);
    sc_assert( sc_time_stamp() - start == sc_time(21, SC_NS) );
    
    start = sc_time_stamp();
    e1.notify(14, SC_NS);
    e2.notify(24, SC_NS);
    
    sc_event_and_list list3 = list2 & e2;
    sc_assert( list3.size() == 2 );
    
    wait( list3 );
    sc_assert( sc_time_stamp() - start == sc_time(24, SC_NS) );
    
    start = sc_time_stamp();
    e1.notify(100, SC_NS);
    e2.notify(200, SC_NS);
    
    sc_event_and_list list4;
    list4 = list3 & e2;
    sc_assert( list4.size() == 2 );
    wait( list4 );
    sc_assert( sc_time_stamp() - start == sc_time(200, SC_NS) );
    
    start = sc_time_stamp();
    e1.notify(101, SC_NS);
    e2.notify(202, SC_NS);
    
    sc_event_and_list list5 = list3 & list4;
    sc_assert( list5.size() == 2 );
    wait( list5 );
    sc_assert( sc_time_stamp() - start == sc_time(202, SC_NS) );
    
    sc_event_and_list list6 = e1;
    sc_assert( list6.size() == 1 );
    
    start = sc_time_stamp();
    e1.notify(500, SC_NS);
    wait(list6);
    sc_assert( sc_time_stamp() - start == sc_time(500, SC_NS) );
        
   
    start = sc_time_stamp();
    e1.notify(1000, SC_NS);
    wait(e1 | e2 | e3);
    sc_assert( sc_time_stamp() - start == sc_time(1000, SC_NS) );
    
    start = sc_time_stamp();
    e1.notify(1002, SC_NS);
    e2.notify(1001, SC_NS);
    e3.notify(1000, SC_NS);
    
    wait(e1 & e2 & e3);
    sc_assert( sc_time_stamp() - start == sc_time(1002, SC_NS) );
    
    start = sc_time_stamp();
    e1.notify(2000, SC_NS);
    
    sc_event_or_list list7;
    list7 = e1 | e2;
    list7 = list7 | e3;
    list7 |= e4;
    wait(list7);
    sc_assert( sc_time_stamp() - start == sc_time(2000, SC_NS) );
    
    start = sc_time_stamp();
    e1.notify(3000, SC_NS);
    list7 = e2 | e3;
    list7 = e1 | list7;
    wait(list7);
    sc_assert( sc_time_stamp() - start == sc_time(3000, SC_NS) );
    sc_assert (sc_time_stamp () == sc_time (8029, SC_NS));
    
    list1 = e1;
    list2 = e2;
    e1.notify(SC_ZERO_TIME);
    wait(list1);
    e2.notify(SC_ZERO_TIME);
    wait(list2);
    
    list1.swap(list2);
    
    e1.notify(SC_ZERO_TIME);
    wait(list2);
    e2.notify(SC_ZERO_TIME);
    wait(list1);
  
    finished = true;  
  }
  
  sc_event v1, v2, v3, v4;
  sc_event_or_list  or_list1,  or_list2;
  sc_event_and_list and_list1, and_list2;
  int count;
  
  void M()
  {
    switch (count)
    {
      case 0:
      {
        sc_assert( sc_time_stamp() == sc_time(0, SC_NS) );
        or_list1 = v1 | v2;
        or_list2 = v3 | v4;
        v2.notify(10, SC_NS);
        next_trigger(or_list1 | or_list2);
        break;
      }
      case 1:
      {
        sc_assert( sc_time_stamp() == sc_time(10, SC_NS) );
        and_list1 = v1 & v2;
        and_list2 = v3 & v4;
        v1.notify(1, SC_NS);
        v2.notify(2, SC_NS);
        v3.notify(3, SC_NS);
        v4.notify(4, SC_NS);
        next_trigger(and_list1 & and_list2);
        break;
      }
      case 2:
      {
        sc_assert( sc_time_stamp() == sc_time(14, SC_NS) );
        and_list1 = v1 & v2 & v2 & v1;
        sc_assert( and_list1.size() == 2 );
        v1.notify(1, SC_NS);
        v2.notify(2, SC_NS);
        next_trigger(and_list1);
        break;
      }
      case 3:
      {
        sc_assert( sc_time_stamp() == sc_time(16, SC_NS) );
        break;
      }
    }
    ++count;
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
  
  sc_assert( sc_get_status() == SC_PAUSED );
  sc_assert( sc_is_running() );
  sc_assert( sc_pending_activity() == false );
  
  for (int i = 0; i < 128; i++)
  {
    delete tops[i];
  }
  
  cout << endl << "Success" << endl;
  return 0;
}
  
