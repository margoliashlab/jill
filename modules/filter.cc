#include <iostream>
#include "../jill/jack_client.hh"
#include "../jill/program_options.hh"
#include "../jill/logging.hh"
#include "../jill/zmq.hh"
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include<typeinfo>
#define PROGRAM_NAME "jfilter"

using namespace jill;
using std::string;

class jfilter_options : public program_options {

public:

  jfilter_options(string const &program_name);

  string client_name;
  string server_name;
  
  std::vector<string> input_ports;
  std::vector<string> output_ports;
  
  float cutoff_frequency;
  

protected:
  virtual void print_usage();
};
  

// void
// filter(sample_t const * input, sample_t * output, jack_nframes_t nframes)
// {

//   // Numerator filter coefficients 
//   sample_t b[] = {0.292893218813453, -0.585786437626905, .292893218813453};
//   // Denominator filter coefficients
//   sample_t a[] = {1.000000000000000,  0.0, 0.171572875253810};                               

//   /*copying the end of input and output from the previous buffer to 
//     the beginning of the input and output arrays */
  
//   memcpy(x, x + nframes, PAD_LENGTH * sizeof(sample_t));
//   memcpy(y, y + nframes, PAD_LENGTH * sizeof(sample_t));

//   /*setting the rest of the output array to 0 */
//   memset(y + PAD_LENGTH, (sample_t ) 0, (nframes + PAD_LENGTH) * sizeof(sample_t));

//   /*copying new buffer to input array*/
//   memcpy(x + PAD_LENGTH, input, nframes * sizeof(sample_t));
  
//   int i,n;
//   for (n = PAD_LENGTH; n < nframes + PAD_LENGTH; n++) {
//     for (i = 0; i < FILTER_LENGTH; i++) {
//       y[n] +=  (b[i] *  x[n-i]) - (a[i] * y[n-i]);      
//     }
//   }

//   /* copying filtered signal to output  */
//   memcpy(output, y + PAD_LENGTH, nframes*sizeof(sample_t)); 

// }

// int
// process (jack_nframes_t nframes, void *arg)
// {

//   sample_t *in, *out;
  
//   in = jack_port_get_buffer (input_port, nframes );
//   out = jack_port_get_buffer (output_port, nframes );

//   filter(in, out, nframes);

//   //printf("%8.5f \t %8.5f \n", in[0], out[0]); 
  
//   return 0;      
// }

magit

//static jack_client client("filter");

void
jack_shutdown()
{
  
}

static jfilter_options options(PROGRAM_NAME);
static boost::shared_ptr<jack_client> client;


int
main(int argc, char **argv)
{

  options.parse(argc,argv);
  const std::type_info &t = typeid(options);
  std::cout << t.name() << std::endl;
  options.client_name = "filter";
  std::cout << options.client_name << std::endl;
            
  
  /*
  port_in_1 = client->register_port("in_1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
  port_out_1 = client->register_port("out_1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
  port_in_2 = client->register_port("in_2", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
  port_out_2 = client->register_port("out_2", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);

  client->set_process_callback(process);
  // */
  // while (running) {
  //   usleep(10,000);
  // }

  
}


jfilter_options::jfilter_options(string const &program_name) 
  : program_options(program_name)
{
  po::options_description jillopts("JILL options");
  jillopts.add_options()
    ("server,s",  po::value<string>(&server_name), "connect to specific jack server")
    ("name,n",    po::value<string>(&client_name)->default_value(_program_name), 
     "set filtering client name")
    ("in,i",      po::value<std::vector<string> >(&input_ports), "add connections to input ports")
    ("out,o",     po::value<std::vector<string> >(&input_ports), "add connections to output ports");
  
  po::options_description opts("Filter options");
  opts.add_options()
    ("cutoff,c", po::value<float>(&cutoff_frequency)->default_value(10), "set cutoff frequency");

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
