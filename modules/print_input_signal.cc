/*
 * A skeleton for jill modules (jack clients using the jill framework). Creates
 * an input and output port, but doesn't do anything. To customize:
 *
 * 1. Replace "modname" with the name of your module.
 * 2. Add variables for configurable options in modname_options class
 * 3. Edit modname_options constructor to define commandline options
 * 4. Edit process() for the realtime logic.
 * 5. Edit main() for startup and shutdown.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Copyright (C) 2010-2013 C Daniel Meliza <dan || meliza.org>
 */
#include <iostream>
#include <signal.h>
#include <boost/shared_ptr.hpp>

#include "jill/logging.hh"
#include "jill/jack_client.hh"
#include "jill/program_options.hh"

#define PROGRAM_NAME "print_input_signal"

using namespace jill;
using std::string;
typedef std::vector<string> stringvec;

class print_input_signal_options : public program_options {

public:
	print_input_signal_options(string const &program_name);

	/** The server name */
	string server_name;
	/** The client name (used in internal JACK representations) */
	string client_name;

protected:

	virtual void print_usage();

}; // print_input_signal_options

static print_input_signal_options options(PROGRAM_NAME);
static boost::shared_ptr<jack_client> client;
jack_port_t *port_in;
static int ret = EXIT_SUCCESS;


static int running = 1;

int
process(jack_client *client, nframes_t nframes, nframes_t)
{
	sample_t *in = client->samples(port_in, nframes);
	
	for(int i=0;i<nframes;i++){
		std::cout<<in[i]<<" ";
	}
	
        return 0;
}


/** handle xrun events */
int
jack_xrun(jack_client *client, float delay)
{
        return 0;
}


/** handle server shutdowns */
void
jack_shutdown(jack_status_t code, char const *)
{
        ret = -1;
        running = 0;
}


/** handle POSIX signals */
void
signal_handler(int sig)
{
        ret = sig;
        running = 0;
}


int
main(int argc, char **argv)
{
	using namespace std;
	const char **ports;
	try {
                // parse options
		options.parse(argc,argv);

                // start client
                client.reset(new jack_client(options.client_name, options.server_name));

                // register ports
                port_in = client->register_port("in",JACK_DEFAULT_AUDIO_TYPE,
                                                JackPortIsInput, 0);

                // register signal handlers
		signal(SIGINT,  signal_handler);
		signal(SIGTERM, signal_handler);
		signal(SIGHUP,  signal_handler);

                // register jack callbacks
                client->set_shutdown_callback(jack_shutdown);
                client->set_process_callback(process);

		

                // activate client
                client->activate();

                while (running) {
                        usleep(100000);
                }

                client->deactivate();
		return ret;
	}

	/*
	 * These catch statements handle two kinds of exceptions.  The
	 * Exit exception is thrown to terminate the application
	 * normally (i.e. if the user asked for the app version or
	 * usage); other exceptions are typically thrown if there's a
	 * serious error, in which case the user is notified on
	 * stderr.
	 */
	catch (Exit const &e) {
		return e.status();
	}
	catch (std::exception const &e) {
                LOG << "ERROR: " << e.what();
		return EXIT_FAILURE;
	}

}


/** configure commandline options */
print_input_signal_options::print_input_signal_options(string const &program_name)
        : program_options(program_name)
{

        // this section is for general JILL options. try to maintain consistency
        // with other modules
        po::options_description jillopts("JILL options");
        jillopts.add_options()
                ("server,s",  po::value<string>(&server_name), "connect to specific jack server")
                ("name,n",    po::value<string>(&client_name)->default_value(_program_name),
                 "set client name")
                ("in,i",      po::value<stringvec>(), "add connection to input port")
                ("out,o",     po::value<stringvec>(), "add connection to output port");
        cmd_opts.add(jillopts);
        visible_opts.add(jillopts);

}


/** provide the user with some information about the ports */
void
print_input_signal_options::print_usage()
{
        std::cout << "Usage: " << _program_name << " [options]\n"
                  << visible_opts << std::endl
                  << "Ports:\n"
                  << " * in:        input port\n"
                  << std::endl;
}

