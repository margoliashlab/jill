#include <iostream>
#include "jill/jack_client.hh"
#include "jill/program_options.hh"


class jfilter_options : public program_options {

public:

  string client_name;
  string server_name;
  
  std::vector<string> input_ports;
  std::vector<string> output_ports;
  
protected:
  virtual void print_usage();
}
  

static jack_client client("filter");

void
jack_shutdown()
{
  
}


int
main(int argc, char **argv)
{
 
  port_in_1 = client->register_port("in_1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
  port_out_1 = client->register_port("out_1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
 
  client->set_process_callback(process);

  while (running) {
    usleep(10,000);
  }

  
}
