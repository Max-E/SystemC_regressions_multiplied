
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

// child_proc_control.cpp -- test for 
//
//  Original Author: John Aynsley, Doulos Inc.
//
// MODIFICATION LOG - modifiers, enter your name, affiliation, date and
//
// $Log: child_proc_control.cpp,v $
// Revision 1.2  2011/05/08 19:18:46  acg
//  Andy Goodrich: remove extraneous + prefixes from git diff.
//

// Process control methods applied to child processes

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <systemc>

using namespace sc_core;
using std::cout;
using std::endl;

struct Top: sc_module
{
  Top(sc_module_name _name)
  : count(0)
  , n(155)
  , proc_count(0)
  {
    SC_THREAD(ctrl);
    SC_THREAD(observer);
 
    f0 = f1 = 0;

    given_birth = new bool[n];   

    c0 = new int[n];
    c1 = new int[n];
    c2 = new int[n];
    c3 = new int[n];
    c4 = new int[n];
    c5 = new int[n];
    
    for (int i = 0; i < n; i++)
    {
      given_birth[i] = false;
      c0[i] = c1[i] = c2[i] = c3[i] = c4[i] = c5[i] = 0;
    }
  }
  
  int count;
  int f0, f1;
  const int n;
  int proc_count;
  bool *given_birth;
  int *c0, *c1, *c2, *c3 ,*c4, *c5;
  
  sc_event ev;
  sc_process_handle ph;
  sc_event_and_list reset_event_list;
  sc_event_and_list terminated_event_list;
  
  sc_mutex given_birth_mutex, proc_count_mutex;
  
  void ctrl()
  {
    count = 1;
    ph = sc_spawn(sc_bind(&Top::parent_proc, this));
    wait(10, SC_NS);
    
    count = 2;
    ph.reset(SC_INCLUDE_DESCENDANTS);
    wait(10, SC_NS);
    
    count = 3;
    ev.notify();
    wait(10, SC_NS);
    
    count = 4;
    ph.suspend(SC_INCLUDE_DESCENDANTS);
    wait(10, SC_NS);
    
    count = 5;
    ev.notify();
    wait(10, SC_NS);
    
    count = 6;
    ph.resume(SC_INCLUDE_DESCENDANTS);
    wait(10, SC_NS);
    
    count = 7;
    ph.kill(SC_INCLUDE_DESCENDANTS);
    wait(10, SC_NS);
  }
  
  void observer()
  {
    wait(SC_ZERO_TIME);
    
    wait(reset_event_list);
    sc_assert(sc_time_stamp() == sc_time(10, SC_NS));
    f0 = 1;
    
    wait(terminated_event_list);
    sc_assert(sc_time_stamp() == sc_time(60, SC_NS));
    f1 = 1;
  }
  
  void parent_proc()
  {
    sc_process_handle h;
    int level = 2;
    for (int i = 0; i < 5; i++)
    {
      proc_count_mutex.lock ();
      if (proc_count < n)
      {
        int new_count = proc_count++;
        proc_count_mutex.unlock ();
        h = sc_spawn(sc_bind(&Top::child_proc, this, new_count, level));
        reset_event_list      &= h.reset_event();
        terminated_event_list &= h.terminated_event();
      }
      else
      {
        proc_count_mutex.unlock ();
      }
    }
  }
  
  void child_proc(int i, int level)
  {
    //cout << "Child " << i << " called at " << sc_time_stamp() << endl;
    if (level > 0)
    {
      bool unlocked = false;
      given_birth_mutex.lock ();
      if ( !given_birth[i] )
      {
        sc_process_handle h;
        for (int j = 0; j < 5; j++)
        {
          proc_count_mutex.lock ();
          if (proc_count < n)
          {
            given_birth[i] = true;
            given_birth_mutex.unlock ();
            unlocked = true;
            int new_count = proc_count++;
            proc_count_mutex.unlock ();
            h = sc_spawn(sc_bind(&Top::child_proc, this, new_count, level-1));
            reset_event_list      &= h.reset_event();
            terminated_event_list &= h.terminated_event();
          }
          else
          {
            proc_count_mutex.unlock ();
          }
        }
      }
      if (!unlocked)
        given_birth_mutex.unlock ();
    }
    switch(count)
    {
      case  1: sc_assert(sc_time_stamp() == sc_time( 0, SC_NS)); c0[i] = 1; break;
      case  2: sc_assert(sc_time_stamp() == sc_time(10, SC_NS)); c2[i] = 1; break;
      default: sc_assert(false); break;
    }
    while(true)
    {
      try {
        wait(ev);
      }
      catch (const sc_unwind_exception& e) {
        switch(count)
        {
          case  2: sc_assert(sc_time_stamp() == sc_time(10, SC_NS)); 
                   sc_assert( e.is_reset() ); c1[i] = 1; break;
          case  7: sc_assert(sc_time_stamp() == sc_time(60, SC_NS));
                   sc_assert( !e.is_reset() ); c5[i] = 1; break;
          default: sc_assert(false); break;
        }
        sc_assert( sc_is_unwinding() );
        throw e;
      }
      switch(count)
      {
        case  3: sc_assert(sc_time_stamp() == sc_time(20, SC_NS)); c3[i] = 1; break;
        case  6: sc_assert(sc_time_stamp() == sc_time(50, SC_NS)); c4[i] = 1; break;
        default: sc_assert(false); break;
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
      sc_assert( tops[i]->proc_count == tops[i]->n );
      
      sc_assert( tops[i]->f0 );
      sc_assert( tops[i]->f1 );

      for (int j = 0; j < tops[i]->n; j++)
      {
        sc_assert( tops[i]->c0[j] );
        sc_assert( tops[i]->c1[j] );
        sc_assert( tops[i]->c2[j] );
        sc_assert( tops[i]->c3[j] );
        sc_assert( tops[i]->c4[j] );
        sc_assert( tops[i]->c5[j] );
      }
  }
  
  cout << endl << "Success" << endl;
  return 0;
}
  
