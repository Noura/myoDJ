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
bool chooseMidiOutPort(RtMidiOut *rtmidi, std::string midiPeerName)
{
  std::cout << "SELECT OUTPUT port to " << midiPeerName << ":" << std::endl;
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


bool chooseMidiInPort(RtMidiIn *rtmidi, std::string midiPeerName)
{
  std::cout << "SELECT INPUT port from " << midiPeerName << ":" << std::endl;
  std::string portName;
  unsigned int i = 0, nPorts = rtmidi->getPortCount();
  if ( nPorts == 0 ) {
    std::cout << "No input ports available!" << std::endl;
    return false;
  }

  if ( nPorts == 1 ) {
    std::cout << "\nOpening " << rtmidi->getPortName() << std::endl;
  }
  else {
    for ( i=0; i<nPorts; i++ ) {
      portName = rtmidi->getPortName(i);
      std::cout << "  Input port #" << i << ": " << portName << '\n';
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
 * Function to send a MIDI note
 */
void sendMIDINote(RtMidiOut* midiOut, bool noteOn, int chan, int key, int vel)
{
	std::vector<unsigned char> message;

	message.push_back((noteOn ? 144 : 128) + chan);
	message.push_back(key);
	message.push_back(vel);
	midiOut->sendMessage( &message );
}


/**
 * Function to send a MIDI continuous controller value
 */
void sendMIDIContinuousControl(RtMidiOut* midiOut, int chan, int controller_num, int value)
{
	std::vector<unsigned char> message;

	message.push_back(176 + chan);
	message.push_back(controller_num);
	message.push_back(value);
	midiOut->sendMessage( &message );
}


/**
 * The main function
 */
int main( void )
{
	// Declare local variables
	RtMidiOut *midiOutProc = 0;
	RtMidiOut *midiOutSerato = 0;
	RtMidiIn *midiIn = 0;
	myo::Hub* hub;
	myo::Myo* myo;
	std::vector<unsigned char> message_in;
	bool tracking = false;
	int old_pitch = -1;
	int serato_cc_num = 16;
	int processing_cc_num = 12;

	// Create the data collector for the Myo bracelet
	DataCollector collector;

	try{
		// First, we create a Hub with our application identifier. Be sure not to use the com.example namespace when
		// publishing your application. The Hub provides access to one or more Myos.
		hub = new myo::Hub("com.example.my_midi");
		std::cout << "Attempting to find a Myo..." << std::endl;
        
		// Next, we attempt to find a Myo to use. If a Myo is already paired in Myo Connect, this will return that Myo
		// immediately.
		myo = hub->waitForMyo(10000);
        
		// If waitForAnyMyo() returned a null pointer, we failed to find a Myo, so exit with an error message.
		if (!myo) {
			std::cout << "Unable to find a Myo!" << std::endl;
			return -1;
		}

		// We've found a Myo.
		std::cout << "Connected to a Myo armband!" << std::endl << std::endl;
        
		hub->addListener(&collector);
	}
	catch(std::runtime_error e) {
		std::cout << "Problem connecting to Myo bracelet. Exiting..." << std::endl;
		return -1;
	}

	// Create the RTMidi output object
	try {
		midiOutProc = new RtMidiOut();
		midiOutSerato = new RtMidiOut();
		midiIn = new RtMidiIn();
	}
	catch ( RtMidiError &error ) {
		error.printMessage();
		exit( EXIT_FAILURE );
	}

	// Select the MIDI port that we want to use
	try {
		if ( chooseMidiInPort(midiIn, "Processing") == false ) goto cleanup;
		if ( chooseMidiOutPort(midiOutProc, "Processing") == false ) goto cleanup;
		if ( chooseMidiOutPort(midiOutSerato, "Serato") == false ) goto cleanup;
	}
	catch ( RtMidiError &error ) {
		error.printMessage();
		goto cleanup;
	}

	// Infinite loop
	while(1)
	{
		// Run the Myo event loop for a set number of milliseconds.
        hub->run(5);

		// Check for messages from Processing
		midiIn->getMessage(&message_in);
		if(message_in.size() > 0)
		{
			std::cout << "MIDI message rcvd: " << int(message_in[0]) << ", " << int(message_in[1]) << ", " << int(message_in[2]) << std::endl;

			// Make sure it is a valid note on/off message
			if(message_in[0] == 145 || message_in[0] == 129)
			{
				// If a tracking on/off message
				if(message_in[1] == 0)
				{
					if(message_in[0] == 145)
						tracking = true;
					else
						tracking = false;
				}
				
				// Else a effect change message
				else
					serato_cc_num = 15 + message_in[1];
			}
		}

		// Check if the user made a hand gesture to toggle hand tracking
		if(collector.trackingGestureMade())
		{
			tracking = !tracking;

			// Send message: Chan 1 note on/off, note 0, velocity 0
			sendMIDINote(midiOutProc, tracking, 1, 0, 0);
		}

		// Only send the latest arm pitch if we are currently tracking
		if(tracking)
		{
			int new_pitch = collector.my_pitch;
			if(new_pitch != old_pitch)
			{
				// Send MIDI message on channel 1 to Serato
				sendMIDIContinuousControl(midiOutSerato, 1, serato_cc_num, new_pitch);

				// Send MIDI message on channel 1 to Processing to update display
				sendMIDIContinuousControl(midiOutProc, 1, processing_cc_num, new_pitch);

				std::cout << "Send MIDI message on CC num " << serato_cc_num << ": " << new_pitch << std::endl;
			}

			old_pitch = new_pitch;
		}
	}

	// Clean up
	cleanup:
		delete midiIn;
		delete midiOutProc;
		delete midiOutSerato;

	return 0;
}


