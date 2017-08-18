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

// #include <sys/time.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <stdexcept>

#include "jill/logging.hh"
#include "jill/jack_client.hh"
#include "jill/program_options.hh"

// https://github.com/lemire/Code-used-on-Daniel-Lemire-s-blog/tree/master/2013/08/14/cpp
// pick N distinct numbers at random
// #include "mersenne.h"
// #include "synthetic.h"

#define PROGRAM_NAME "jflip"

using namespace jill;
using std::string;
typedef std::vector<string> stringvec;

class jflip_options : public program_options {

public:
    jflip_options(string const &program_name);

    /** The server name */
    string server_name;
    /** The client name (used in internal JACK representations) */
    string client_name;


    /** A vector of inputs to connect to the client */
    std::vector<string> input_ports;
    /** A vector of outputs to connect to the client */
    std::vector<string> output_ports;
        /** The MIDI output channel */
        // midi::data_type output_chan;

    // float buffer_size_s;


protected:

    virtual void print_usage();

}; // jflip_options

static jflip_options options(PROGRAM_NAME);
static boost::shared_ptr<jack_client> client;
// boost::shared_ptr<dsp::crossing_trigger<sample_t> > trigger;
// boost::shared_ptr<dsp::buffered_data_writer> arf_thread;
jack_port_t *port_in, *port_out, *port_trig,*port_count;
static int ret = EXIT_SUCCESS;
static int running = 1;
static int stopping = 0;
static int xruns = 0;

/* data storage for event times */
// struct event_t {
//         nframes_t time;
//         int status;
// };
// dsp::ringbuffer<event_t> trig_times(128);


int
process(jack_client *client, nframes_t nframes, nframes_t)
{
    sample_t *in = client->samples(port_in, nframes);
    sample_t *out = client->samples(port_out, nframes);

    nframes_t inputSize = nframes;

    memcpy(out, in, nframes * sizeof(sample_t));
    /* rand() method */
    int i = 0;
    std::srand(std::time(NULL));
    while( i < inputSize) { 
        int coin = rand()%2;
        if (coin == 0) *(out+i) *= -1;
        i++;
    }

    /* use Lemire's picking number method */
    // UniformDataGenerator udg;
    // // randomly choose half of the signals to flip, chosen indices stored in "array"
    // vector<uint32_t> array = udg.generateUniformTree(inputSize/2,inputSize); 
    // if(array.size() != inputSize/2) throw runtime_error("bug");
    // for (nframes_t i = 0; i < inputSize/2; i++) {
    //     nframes_t idx = array.at(i);
    //     *(out+idx) *= -1; // flip the ones chosen
    // }

        return 0;
}


/** this is called by jack when calculating latency */
void
jack_latency (jack_latency_callback_mode_t mode, void *arg)
{
    jack_latency_range_t range;
        jack_port_get_latency_range (port_in, mode, &range);
    if (mode == JackCaptureLatency) {
                // add latency between inputs and outputs
    }
        else {
                // add latency between output and input
    }
        jack_port_set_latency_range (port_out, mode, &range);
}


/** handle changes to buffer size */
int
jack_bufsize(jack_client *client, nframes_t nframes)
{
        return 0;
}


/** handle xrun events */
int
jack_xrun(jack_client *client, float delay)
{
        __sync_add_and_fetch(&xruns, 1); // gcc specific
        // arf_thread->xrun();
        return 0;
}


/** handle server shutdowns */
void
jack_shutdown(jack_status_t code, char const *)
{
        ret = -1;
        LOG << "jackd shut the client down";
        __sync_add_and_fetch(&stopping, 1);
        // wait for at least one process loop; not strictly async safe
        usleep(2e6 * client->buffer_size() / client->sampling_rate());
        // if (arf_thread) {
        //         arf_thread->stop();
        // }
        running = 0;
}


/** handle POSIX signals */
void
signal_handler(int sig)
{
        ret = sig;
        // if (arf_thread) {
        //         arf_thread->stop();
        // }
        running = 0;
}


int
main(int argc, char **argv)
{
    using namespace std;
    try {
                // parse options
        options.parse(argc,argv);

                // start client
                client.reset(new jack_client(options.client_name, options.server_name));

                // register ports
                port_in = client->register_port("in",JACK_DEFAULT_AUDIO_TYPE,
                                                JackPortIsInput, 0);
                port_out = client->register_port("out", JACK_DEFAULT_AUDIO_TYPE,
                                                 JackPortIsOutput, 0);
                // port_trig = client->register_port("trig_out",JACK_DEFAULT_MIDI_TYPE,
                //                                 JackPortIsOutput, 0);

                // register signal handlers
        signal(SIGINT,  signal_handler);
        signal(SIGTERM, signal_handler);
        signal(SIGHUP,  signal_handler);

                // register jack callbacks
                client->set_shutdown_callback(jack_shutdown);
                client->set_xrun_callback(jack_xrun);
                client->set_process_callback(process);

                client->set_buffer_size_callback(jack_bufsize);

                // uncomment if you need these callbacks
                // client->set_buffer_size_callback(jack_bufsize);
                // jack_set_latency_callback (client->client(), jack_latency, 0);

                // activate client
                client->activate();
                // arf_thread->start();

                // connect ports
                if (options.count("in")) {
                        stringvec const & portlist = options.vmap["in"].as<stringvec>();
                        client->connect_ports(portlist.begin(), portlist.end(), "in");
                }
                if (options.count("out")) {
                        stringvec const & portlist = options.vmap["out"].as<stringvec>();
                        client->connect_ports("out", portlist.begin(), portlist.end());
                }

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
jflip_options::jflip_options(string const &program_name)
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
                ("out,o",     po::value<stringvec>(), "add connection to output port")
                // ("buffer",     po::value<float>(&buffer_size_s)->default_value(2.0), "minimum ringbuffer size (s)")
                ;
        cmd_opts.add(jillopts);
        // cfg_opts.add(jillopts);
        visible_opts.add(jillopts);

}


/** provide the user with some information about the ports */
void
jflip_options::print_usage()
{
        std::cout << "Usage: " << _program_name << " [options]\n"
                  << visible_opts << std::endl
                  << "Ports:\n"
                  << " * in:        input port\n"
                  << " * out:       output port\n"
                  << std::endl;
}

