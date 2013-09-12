#include <iostream>
#include <signal.h>
#include <boost/shared_ptr.hpp>
#include <sstream>

#include "../jill/logging.hh"
#include "../jill/jack_client.hh"
#include "../jill/program_options.hh"

#define PROGRAM_NAME "jfilter"

using namespace jill;
using std::string;
typedef std::vector<string> svec;

class jfilter_options : public program_options {

public:

        jfilter_options(string const &program_name);

        string client_name;
        string server_name;
  
        svec input_ports;
        svec output_ports;
  
        int nports;
        float cutoff_frequency;
  

protected:
        virtual void print_usage();
};
  
static jfilter_options options(PROGRAM_NAME);
static boost::shared_ptr<jack_client> client;
//jack_port_t *port_in, *port_out;
static int ret = EXIT_SUCCESS;
static int running = 1;

void
filter(sample_t const * input, sample_t * output, nframes_t nframes)
{

        // static nframes_t x; 

        // // Numerator filter coefficients 
        // sample_t b[] = {0.292893218813453, -0.585786437626905, .292893218813453};
        // // Denominator filter coefficients
        // sample_t a[] = {1.000000000000000,  0.0, 0.171572875253810};                               

        // /*copying the end of input and output from the previous buffer to 
        //   the beginning of the input and output arrays */
  
        // memcpy(x, x + nframes, PAD_LENGTH * sizeof(sample_t));
        // memcpy(y, y + nframes, PAD_LENGTH * sizeof(sample_t));

        // /*setting the rest of the output array to 0 */
        // memset(y + PAD_LENGTH, (sample_t ) 0, (nframes + PAD_LENGTH) * sizeof(sample_t));

        // /*copying new buffer to input array*/
        // memcpy(x + PAD_LENGTH, input, nframes * sizeof(sample_t));
  
        // int i,n;
        // for (n = PAD_LENGTH; n < nframes + PAD_LENGTH; n++) {
        //         for (i = 0; i < FILTER_LENGTH; i++) {
        //                 y[n] +=  (b[i] *  x[n-i]) - (a[i] * y[n-i]);      
        //         }
        // }

        // /* copying filtered signal to output  */
        // memcpy(output, y + PAD_LENGTH, nframes*sizeof(sample_t)); 

}


int 
process (jack_nframes_t nframes, void *arg)
{

//   sample_t *in, *out;
  
//   in = jack_port_get_buffer (input_port, nframes );
//   out = jack_port_get_buffer (output_port, nframes );

//   filter(in, out, nframes);

//   //printf("%8.5f \t %8.5f \n", in[0], out[0]); 
  
//   return 0;      
}


//static jack_client client("filter");

void
jack_shutdown()
{
  
}


/** this is called by jack when calculating latency */
void
jack_latency (jack_latency_callback_mode_t mode, void *arg)
{
	// jack_latency_range_t range;
        // jack_port_get_latency_range (port_in, mode, &range);
	// if (mode == JackCaptureLatency) {
        //         // add latency between inputs and outputs
	// }
        // else {
        //         // add latency between output and input
	// }
        // jack_port_set_latency_range (port_out, mode, &range);
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



void 
create_ports(int nports, string const & base_name, string const & type,
                             unsigned long flags, unsigned long buffer_size=0) {
        using std::stringstream;        
        stringstream stream_idx;       
        string string_idx;
        string port_name;
        for (int i = 1; i <= nports; ++i) {
                stream_idx << i;
                string_idx = stream_idx.str();	      
                port_name = base_name + string_idx;
                client->register_port(port_name, type,flags, buffer_size);
                stream_idx.str("");             
        }
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


                // register input ports
                create_ports(options.nports, "in_", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput);
               
               
                // register output ports 
                create_ports(options.nports, "out_", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput);
                
                                        
                
       
                // register signal handlers
		signal(SIGINT,  signal_handler);
		signal(SIGTERM, signal_handler);
		signal(SIGHUP,  signal_handler);

                // register jack callbacks
                // client->set_shutdown_callback(jack_shutdown);
                // client->set_xrun_callback(jack_xrun);
                // client->set_process_callback(process);

                // uncomment if you need these callbacks
                // client->set_buffer_size_callback(jack_bufsize);
                // jack_set_latency_callback (client->client(), jack_latency, 0);

                // activate client
                client->activate();
         

                // connect ports
                if (options.count("in")) {
                        svec const & portlist = options.vmap["in"].as<svec>();
                        client->connect_ports(portlist.begin(), portlist.end(), "in");
                }
                if (options.count("out")) {
                        svec const & portlist = options.vmap["out"].as<svec>();
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


jfilter_options::jfilter_options(string const &program_name) 
: program_options(program_name)
{
        using namespace std;
        po::options_description jillopts("JILL options");
        jillopts.add_options()
                ("server,s",    po::value<string>(&server_name), "connect to specific jack server")
                ("name,n",      po::value<string>(&client_name)->default_value(PROGRAM_NAME), 
                 "set filtering client name")
                ("in,i",        po::value<vector<string> >(&input_ports), "add connections to input ports of jfilter")
                ("out,o",       po::value<vector<string> >(&output_ports), "add connections to output ports of jfilter")
                ("ports,p",     po::value<int>(&nports)->default_value(1), "number of jfilter ports to create.\n If less than number of connections, additional ports will be created.");
                                            
  
        options.nports =  max(options.nports, max(options.count("in"), options.count("out"))); 
        po::options_description opts("Filter options");
        opts.add_options()
                ("cutoff-frequency, f", po::value<float>(&cutoff_frequency)->default_value(100), 
                 "set cutoff frequency");

        cmd_opts.add(jillopts).add(opts);
        visible_opts.add(jillopts).add(opts);

}

void
jfilter_options::print_usage()
{
  std::cout << "Usage: " << _program_name << " [options]\n" 
            << visible_opts << std::endl
            << "Ports \n"
            << " * in: \t input port" << std::endl
            << " * out: \t output port with filtered signal" << std::endl;
}
