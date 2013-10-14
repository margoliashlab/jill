
#ifndef _FILTER_HH
#define _FILTER_HH 1

#include <boost/noncopyable.hpp>
#include <jack/jack.h>
#include <string>
#include <map>
#include <vector>


namespace jill {

class digital_filter : boost::noncopyable {
public:

        typedef jack_default_audio_sample_t sample_t;
        typedef jack_nframes_t nframes_t;

        digital_filter();

        
        void 
        filter_buf(sample_t const * const in, sample_t * const out, 
                        std::string port_name, nframes_t nframes);
                    
        void reset_pads(); 

        bool is_iir() {return coef_out.size() >= 1;}
        nframes_t pad_len() {return std::max(coef_in.size(), coef_in.size()) - 1;}

        void
        custom_coef(std::vector<sample_t>, std::vector<sample_t>);


private:
        
        std::vector<sample_t> coef_in;
        std::vector<sample_t> coef_out;
        
        std::map<std::string, std::vector<sample_t> > pads_out;
        std::map<std::string, std::vector<sample_t> > pads_in;

}; 

}

#endif
