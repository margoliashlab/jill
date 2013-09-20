#include <signal.h>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/function.hpp>
#include <iostream>

#include "../jill/logging.hh"
#include "../jill/jack_client.hh"
#include "../jill/program_options.hh"

#define PROGRAM_NAME "jpop"

using namespace jill;
using std::string;
typedef std::vector<string> stringvec;

class jpop_options : public program_options {

public:
	jpop_options(string const &program_name);

	/** The server name */
	string server_name;
	/** The client name (used in internal JACK representations) */
	string client_name;
        
        /* threshold for below which signals are zeroed out */
        float threshold;

protected:

	virtual void print_usage();

}; // jpop_options

static jpop_options options(PROGRAM_NAME);
static boost::shared_ptr<jack_client> client;
jack_port_t *port_in, *port_out;
static int ret = EXIT_SUCCESS;
static int running = 1;


int
process(jack_client *client, nframes_t nframes, nframes_t time)
{
	sample_t *in = client->samples(port_in, nframes);
	sample_t *out = client->samples(port_out, nframes);

        memset(out, (sample_t ) 0, nframes * sizeof(sample_t));
        for (int i = 0; i < nframes; i++) {
                if (in[i] > options.threshold && !options.count("reverse")) {
                        out[i] = in[i];               
                }    
                else if (in[i] < options.threshold && options.count("reverse")) {
                        out[i] = in[i];
                }        
        }
                


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
	try {
                // parse options
		options.parse(argc,argv);

                cout <<   options.count("reverse")<< endl;
                // start client
                client.reset(new jack_client(options.client_name, options.server_name));

                // register ports
                port_in = client->register_port("in",JACK_DEFAULT_AUDIO_TYPE,
                                                JackPortIsInput, 0);
                port_out = client->register_port("out", JACK_DEFAULT_AUDIO_TYPE,
                                                 JackPortIsOutput, 0);

                // register signal handlers
		signal(SIGINT,  signal_handler);
		signal(SIGTERM, signal_handler);
		signal(SIGHUP,  signal_handler);

                // register jack callbacks
                client->set_shutdown_callback(jack_shutdown);
                client->set_xrun_callback(jack_xrun);
                client->set_process_callback(process);

                // uncomment if you need these callbacks
                // client->set_buffer_size_callback(jack_bufsize);
                // jack_set_latency_callback (client->client(), jack_latency, 0);

                // activate client
                client->activate();

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
jpop_options::jpop_options(string const &program_name)
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



        po::options_description opts("module options");
        opts.add_options()
                ("threshold,t", po::value<float>(&threshold)->default_value(.75),
                 "threshold of signal value below which samples are set to 0")
                ("reverse,r", "reverse");
  


        cmd_opts.add(opts);
        visible_opts.add(opts);
}


/** provide the user with some information about the ports */
void
jpop_options::print_usage()
{
        std::cout << "Usage: " << _program_name << " [options]\n"
                  << visible_opts << std::endl
                  << "Ports:\n"
                  << " * in:        input port\n"
                  << " * out:       output port\n"
                  << std::endl;
}

