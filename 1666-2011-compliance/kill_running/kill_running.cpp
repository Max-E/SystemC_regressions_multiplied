
//******************************************************************************
//
//  The following code is derived, directly or indirectly, from the SystemC
//  source code Copyright (c) 1996-2015 by all Contributors.
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

// kill_running.cpp -- test for killing a running process, especially in a
// concurrent scenario.
//  Original Author: Max Eliaser, Oregon State University

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <systemc>

using namespace sc_core;
using std::cout;
using std::endl;

#define NUM_PADDING 15

#define NUM_VICTIM_KILLER_PAIRS 4

struct Top: sc_module
{
	SC_CTOR (Top)
	{
		for (int i = 0; i < NUM_VICTIM_KILLER_PAIRS; i++)
		{
			for (int j = 0; j < NUM_PADDING; j++)
				sc_spawn (sc_bind (&Top::pad, this));
			v[i] = sc_spawn (sc_bind (&Top::victim, this));
		}
		
		for (int i = 0; i < NUM_VICTIM_KILLER_PAIRS; i++)
		{
			for (int j = 0; j < NUM_PADDING; j++)
				sc_spawn (sc_bind (&Top::pad, this));
			sc_spawn (sc_bind (&Top::killer, this, i));
		}
	}
	
	void pad (void)
	{
	}
	
	void victim (void)
	{
		while (true)
		{
			printf ("victim starting\n");
			sleep (2);
			printf ("victim stopping\n");
			wait (sc_time (1, SC_NS));
		}
	}
	
	void killer (int id)
	{
		sleep (1);
		printf ("killer about to kill\n");
		v[id].kill ();
		printf ("killer killing\n");
	}
	
  	sc_process_handle v[NUM_VICTIM_KILLER_PAIRS];
};

int sc_main(int argc, char* argv[])
{
	Top top("top");
	
	sc_start(10, SC_NS);
	
	return 0;
}
