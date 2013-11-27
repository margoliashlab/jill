#include <iostream>
#include <signal.h>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/function.hpp>
#include <boost/lambda/lambda.hpp>
#include <sstream>
#include <algorithm>

#include "../jill/digital_filter.hh"
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

        string filter_class;
        string filter_type;
        std::vector<sample_t> cutoff_frequencies;
        int order;

        std::vector<sample_t> numerator;
        std::vector<float> denominator;
        
        

protected:
        virtual void print_usage();
};
  



static jfilter_options options(PROGRAM_NAME);
static boost::shared_ptr<jack_client> client;
static plist_t ports_in, ports_out;
static int ret = EXIT_SUCCESS;
static int running = 1;


static digital_filter filter; 


int 
process (jack_client *client, nframes_t nframes, nframes_t)
{

        sample_t *in, *out;
  
        plist_t::const_iterator it_out = ports_out.begin();
        for (plist_t::const_iterator it_in = ports_in.begin(); it_in != ports_in.end(); it_in++) { 
                in = client->samples(*it_in, nframes);	  
                if (in == 0) continue;
                out = client->samples(*it_out, nframes);
                filter.filter_buf(in, out, jack_port_name(*it_in), nframes);
                it_out++;
        }
  
        return 0;      
}


/** this is called by jack when calculating latency */
void
jack_latency (jack_latency_callback_mode_t mode, void *arg)
{

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
        
        std::cout << "xrun: " << delay << std::endl;
        filter.reset_pads();
        return 0;
}


/** handle server shutdowns */
void
jack_shutdown(jack_status_t code, char const *msg)
{
        
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



plist_t 
create_ports(int nports, string const & base_name, string const & type,
             unsigned long flags, unsigned long buffer_size=0) {
        
	svec port_names;
	using std::stringstream;        
        stringstream stream_idx;       
        std::string port_string;
        jack_port_t* p;
        plist_t ports;

        for (int i = 0; i <= nports; ++i) {
                stream_idx << i + 1;
                port_string = base_name + stream_idx.str();  
		p = client->register_port(port_string, type, flags, buffer_size);
                ports.push_back(p);
                stream_idx.str("");             
        }
	return ports;
}



                   
int
main(int argc, char **argv)
{
	using namespace std;

	try {
                // parse options
                
		options.parse(argc,argv);

                
                // TODO raise exception for incompatible arguments
                if (options.count("numerator") && options.count("denominator") ) {
                        filter.custom_coef(options.numerator,
                                           options.denominator);
                                
                }               
                else {
                        
                        // vector<sample_t> Wn(options.cutoff_frequencies.size());

                        // nframes_t 
                        // boost::lambda::placeholder1_type X;
                        // std::for_each(options.cutoff_frequencies.begin(), 
                        //                options.cutoff_frequencies.end(), 
                        //               X / );
                        
                        filter.butter(options.order, 
				      options.cutoff_frequencies, 		
				      options.filter_type);
                } 
                

                // start client
                client.reset(new jack_client(options.client_name, options.server_name));


                // register input ports
                ports_in = create_ports(options.nports, "in_", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
               
               

                // register output ports 
                ports_out = create_ports(options.nports, "out_", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
		
                // const jack_port_t* p = client->get_port(ports_out[0]);                        
                // std::cout << jack_port_name(p) << std::endl;
                // std::cout << ports_out[0] << std::endl;
       
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
                
             
                if (options.count("in")) {
                        plist_t::const_iterator it_port = ports_in.begin();
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
                                        client->connect_port(*it_connect, 
                                                             jack_port_name(*it_port));
                        
                                }
                                it_port++;        
                        }
                }

                if (options.count("out")) {
                        plist_t::const_iterator it_port = ports_out.begin();
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
                                        client->connect_port(jack_port_name(*it_port), 
                                                             *it_connect);
                        
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
                ("numerator",   po::value<vector<sample_t> >(&numerator)->multitoken(), 
                 "Set custom numerator coefficients of filter.")
                ("denominator", po::value<vector<sample_t> >(&denominator)->multitoken(), 
                 "Set custom denominator coefficients of filter")        
                // ("class,c", po::value<string>(&filter_class)->default_value("butterworth"), "Class of filter.  Available classes: butterworth")
                ("type,t", po::value<string>(&filter_type)->default_value("low-pass"), "Filter type. Available types: low-pass, high-pass, band-pass, band-stop")
                ("cutoff-frequencies,f", po::value<vector<sample_t> >(&cutoff_frequencies)->multitoken(), "Cutoff frequencies")
                ("order,O", po::value<int>(&order), "Filter order (number of poles).");


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
