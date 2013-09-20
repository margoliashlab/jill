#include <iostream>
#include <signal.h>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/function.hpp>
#include <sstream>

#include "../jill/logging.hh"
#include "../jill/jack_client.hh"
#include "../jill/program_options.hh"

#define PROGRAM_NAME "jfilter"


using namespace jill;
using std::string;
typedef std::vector<string> svec;
typedef jack_client::port_list_type plist_t;

class jfilter_options : public program_options {

public:

        jfilter_options(string const &program_name);

        string client_name;
        string server_name;
  
        svec input_ports;
        svec output_ports;
  

        int nports;
        // float cutoff_frequency;

        std::vector<float> numerator;
        std::vector<float> denominator;
        
  

protected:
        virtual void print_usage();
};
  

static const int MaxBufferSize = 10000;


static jfilter_options options(PROGRAM_NAME);
static boost::shared_ptr<jack_client> client;
static svec ports_in, ports_out;
static int ret = EXIT_SUCCESS;
static int running = 1;

static sample_t *x, *y;


// default numerator filter coefficients stored in vector b
sample_t  numerator_array[] = {0.063713475989648, 0.0, -0.127426951979296, 0.0, 0.063713475989648};
static std::vector<sample_t> b(numerator_array, numerator_array + sizeof(numerator_array)/sizeof(sample_t) ); 

// default filter coefficients stored in vector a
sample_t denominator_array[] =  {1.0, -3.148598245944364, 3.730068744095790, -2.006303752961768, 0.424952625169780};
static std::vector<sample_t> a(denominator_array, denominator_array + sizeof(denominator_array)/sizeof(sample_t) );

void
filter(sample_t const * input, sample_t * output, nframes_t nframes)
{


                             

      
  
        static const int PadLength = a.size() - 1;
        

        /*copying the end of input and output from the previous buffer to 
        the beginning of the input and output arrays */

        memcpy(x, x + nframes, PadLength * sizeof(sample_t));
        memcpy(y, y + nframes, PadLength * sizeof(sample_t));

        /*setting the rest of the output array to 0 */
        memset(y + PadLength, (sample_t ) 0, (nframes + PadLength) * sizeof(sample_t));

        /*copying new buffer to input array*/
        memcpy(x + PadLength, input, nframes * sizeof(sample_t));
  
        int i,n;
        for (n = PadLength; n < nframes + PadLength; n++) {
                for (i = 0; i < PadLength + 1; i++) {
                        y[n] +=  (b[i] *  x[n-i]) - (a[i] * y[n-i]);      
                }
        }

        // copying filtered signal to output
        memcpy(output, y + PadLength, nframes*sizeof(sample_t)); 
        //memcpy(output, input, nframes*sizeof(sample_t));
}


int 
process (jack_client *client, nframes_t nframes, nframes_t)
{

  sample_t *in, *out;
  
  svec::const_iterator it_out = ports_out.begin();
  for (svec::const_iterator it_in = ports_in.begin(); it_in != ports_in.end(); it_in++) { 
  	  in = client->samples(*it_in, nframes);	  
          if (in == 0) continue;
  	  out = client->samples(*it_out, nframes);
  	  filter(in, out, nframes);
  	  it_out++;
  }
  
  return 0;      
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

        // sample_t xpad[PadLength];
        // sample_t ypad[PadLength];
  
        // /* saves state of filter */
        // int old_buffer_size = sizeof(x)/sizeof(sample_t) - PadLength;
        // memcpy(xpad, x + old_buffer_size, PadLength * sizeof(sample_t));
        // memcpy(ypad, y + old_buffer_size, PadLength * sizeof(sample_t));

        // /*reallocates memory for pointers x and y to account for changes in buffer size */
        // realloc(x, (PadLength + nframes) * sizeof(sample_t));
        // realloc(y, (PadLength + nframes) * sizeof(sample_t));
  
        // /* copies state of filter back to x and y */
        // memcpy(x + nframes, xpad, PadLength * sizeof(sample_t));
        // memcpy(y + nframes, ypad, PadLength * sizeof(sample_t)); 
  

        // printf("Buffer size changed to %d samples", nframes);
        return 0;
}


/** handle xrun events */
int
jack_xrun(jack_client *client, float delay)
{
        
        std::cout << "xrun: " << delay << std::endl;
        return 0;
}


/** handle server shutdowns */
void
jack_shutdown(jack_status_t code, char const *msg)
{
        free(x);
        free(y);
        ret = -1;
        running = 0;
}

// jack_client::ShutdownCallback shutdown_callback = &jack_shutdown;
//shutdown_callback = &jack_shutdown;

/** handle POSIX signals */
void
signal_handler(int sig)
{
        ret = sig;
        running = 0;
}



svec 
create_ports(int nports, string const & base_name, string const & type,
                             unsigned long flags, unsigned long buffer_size=0) {
        
	svec port_names;
	using std::stringstream;        
        stringstream stream_idx;       
        std::string port_string;

        for (int i = 0; i <= nports; ++i) {
                stream_idx << i + 1;
                port_string = base_name + stream_idx.str();  
		client->register_port(port_string, type, flags, buffer_size);
                port_names.push_back(port_string);
                stream_idx.str("");             
        }
	return port_names;
}



                   
int
main(int argc, char **argv)
{
	using namespace std;

	try {
                // parse options
		options.parse(argc,argv);

                if (options.count("numerator") && options.count("denominator") ) {
                        b = options.numerator;
                        a = options.denominator;
                                
                }


                // start client
                client.reset(new jack_client(options.client_name, options.server_name));


                // register input ports
                ports_in = create_ports(options.nports, "in_", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
               
               
                // register output ports 
                ports_out = create_ports(options.nports, "out_", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
		
                                        
                
       
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
                nframes_t buffer_size = client->buffer_size();
                
		x = (sample_t *) calloc(MaxBufferSize, sizeof(sample_t));
		y = (sample_t *) calloc(MaxBufferSize, sizeof(sample_t));

             
                if (options.count("in")) {
                        svec::const_iterator it_port = ports_in.begin();
                        svec const & in_connections = options.vmap["in"].as<svec>();                        
                        for (svec::const_iterator it_connect = in_connections.begin(); 
                             it_connect != in_connections.end(); it_connect++) {

                                jack_port_t *p = client->get_port(*it_connect);                               
                                if (p==0) {
                                        LOG << "error registering port: source port \""
                                        << *it_connect << "\" does not exist";
                                        throw Exit(-1);
                                }
                                else if (!(jack_port_flags(p) & JackPortIsOutput)) {
                                        LOG << "error registering port: source port \""
                                            << *it_connect << "\" is not an output port";
                                        throw Exit(-1);
                                }
                                else {
                                client->connect_port(*it_connect, *it_port);
                        
                                }
                                it_port++;        
                        }
                }

                if (options.count("out")) {
                        svec::const_iterator it_port = ports_out.begin();
                        svec const & out_connections = options.vmap["out"].as<svec>();                        
                        for (svec::const_iterator it_connect = out_connections.begin(); 
                             it_connect != out_connections.end(); it_connect++) {

                                jack_port_t *p = client->get_port(*it_connect);                               
                                if (p==0) {
                                        LOG << "error registering port: destination port \""
                                        << *it_connect << "\" does not exist";
                                        throw Exit(-1);
                                }
                                else if (!(jack_port_flags(p) & JackPortIsInput)) {
                                        LOG << "error registering port: destination port \""
                                            << *it_connect << "\" is not an input port";
                                        throw Exit(-1);
                                }
                                else {
                                        client->connect_port(*it_port, *it_connect);
                        
                                }
                                it_port++;        
                        }
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
                ("numerator,b",   po::value<vector<float> >(&numerator)->multitoken(), 
                 "set numerator coefficients of filter. Unless both numerator and denominator filter coefficients are given as arguments, the default filter will be a 2nd order butterworth filter with pass band between 100 Hz and 3000 Hz, for a sampling rate of 30 kHz")
                ("denominator,a", po::value<vector<float> >(&denominator)->multitoken(), 
                 "set denominator coefficients of filter");
               
                // ("cutoff-frequency, f", po::value<float>(&cutoff_frequency)->default_value(100), 
                //  "set cutoff frequency")


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
