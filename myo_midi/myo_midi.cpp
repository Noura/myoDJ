//*****************************************//
//  myo_midi.cpp
//  by Simon Scott
//
//  Converts Myo pitch to MIDI messages
//
//*****************************************//

// General includes
#include <iostream>
#include <cstdlib>

// Includes for MIDI
#include "RtMidi.h"

// Includes for Myo
#include <myo/myo.hpp>
#include "myo.h"

// Platform-dependent sleep routines.
#if defined(__WINDOWS_MM__)
  #include <windows.h>
  #define SLEEP( milliseconds ) Sleep( (DWORD) milliseconds ) 
#else // Unix variants
  #include <unistd.h>
  #define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )
#endif

/**
 * This function should be embedded in a try/catch block in case of
 * an exception.  It offers the user a choice of MIDI ports to open.
 * It returns false if there are no ports available.
 */
bool chooseMidiPort( RtMidiOut *rtmidi )
{
  std::string portName;
  unsigned int i = 0, nPorts = rtmidi->getPortCount();
  if ( nPorts == 0 ) {
    std::cout << "No output ports available!" << std::endl;
    return false;
  }

  if ( nPorts == 1 ) {
    std::cout << "\nOpening " << rtmidi->getPortName() << std::endl;
  }
  else {
    for ( i=0; i<nPorts; i++ ) {
      portName = rtmidi->getPortName(i);
      std::cout << "  Output port #" << i << ": " << portName << '\n';
    }

    do {
      std::cout << "\nChoose a port number: ";
      std::cin >> i;
    } while ( i >= nPorts );
  }

  std::cout << "\n";
  rtmidi->openPort( i );

  return true;
}


/**
 * The main function
 */
int main( void )
{
	// Declare local variables
	RtMidiOut *midiout = 0;
	std::vector<unsigned char> message;
	bool tracking = false;
	int old_pitch = -1;

	// Create the data collector for the Myo bracelet
	DataCollector collector;

    // First, we create a Hub with our application identifier. Be sure not to use the com.example namespace when
    // publishing your application. The Hub provides access to one or more Myos.
    myo::Hub hub("com.example.my_midi");
    std::cout << "Attempting to find a Myo..." << std::endl;
        
    // Next, we attempt to find a Myo to use. If a Myo is already paired in Myo Connect, this will return that Myo
    // immediately.
    myo::Myo* myo = hub.waitForMyo(10000);
        
    // If waitForAnyMyo() returned a null pointer, we failed to find a Myo, so exit with an error message.
    if (!myo) {
        std::cout << "Unable to find a Myo!" << std::endl;
		return -1;
    }

    // We've found a Myo.
    std::cout << "Connected to a Myo armband!" << std::endl << std::endl;
        
    hub.addListener(&collector);

	// Create the RTMidi output object
	try {
		midiout = new RtMidiOut();
	}
	catch ( RtMidiError &error ) {
		error.printMessage();
		exit( EXIT_FAILURE );
	}

	// Select the MIDI port that we want to use
	try {
		if ( chooseMidiPort( midiout ) == false ) goto cleanup;
	}
	catch ( RtMidiError &error ) {
		error.printMessage();
		goto cleanup;
	}

	// Infinite loop
	while(1)
	{
		// Run the Myo event loop for a set number of milliseconds.
        //hub.run(5);

		// Check if the user made a hand gesture to toggle hand tracking
		//if(collector.fingersHaveSpread())
		//	tracking = !tracking;

		// Only send the latest arm pitch if we are currently tracking
		if(tracking)
		{
			int new_pitch = 0; //collector.my_pitch;
			if(new_pitch != old_pitch)
			{
				message.clear();

				// Control Change message format:
				// Channel 0: 176
				// Continuous controller 12 (effect controller 1): 12
				// Value from 0 to 127
				message.push_back(176);
				message.push_back(12);
				message.push_back(new_pitch);
				midiout->sendMessage( &message );
				
				std::cout << "Send MIDI message on channel 12: " << new_pitch << std::endl;
			}

			old_pitch = new_pitch;
		}
	}

	// Clean up
	cleanup:
		delete midiout;

	return 0;
}


