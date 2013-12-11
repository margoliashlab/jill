#ifndef _DIGITAL_FILTER_HH
#define _DIGITAL_FILTER_HH 1

#include "transfer_function.hh"
#include <boost/noncopyable.hpp>
#include <boost/math/tools/polynomial.hpp>
#include <jack/jack.h>
#include <string>
#include <map>
#include <vector>
#include <algorithm>

namespace jill {
        

class digital_filter : boost::noncopyable {
public:


        typedef jack_default_audio_sample_t sample_t;
        typedef jack_nframes_t nframes_t;      

        typedef double COEF_t;
        typedef std::complex<COEF_t> complex_t;
        typedef boost::math::tools::polynomial<complex_t> complex_poly;
        typedef boost::math::tools::polynomial<COEF_t> poly;


        digital_filter();
        ~digital_filter(){}
       
        // filters single buffer from a port and stores pad for that port 
        void 
        filter_buf(sample_t const * const in, sample_t * const out, 
                        std::string port_name, nframes_t nframes);

        void reset_pads(); 
        
        bool is_iir() {return _coef_out.size() >= 1;}
        nframes_t pad_len() {return std::max(_coef_in.size(), _coef_out.size()) - 1;}
        
        std::vector<COEF_t> coef_in() {return _coef_in;}
        std::vector<COEF_t> coef_out() {return _coef_out;}
        
        void log_coefs();
        void log_filter(int N, std::vector<COEF_t> Wc, 
                        std::string filter_type, std::string filter_class);
        void custom_coef(std::vector<COEF_t>, std::vector<COEF_t>);       
        void butter(int N, std::vector<COEF_t> Wn, std::string filter_type, nframes_t fs);


protected:
                
        std::vector<COEF_t> _coef_in;
        std::vector<COEF_t> _coef_out;
        
        std::map<std::string, std::vector<COEF_t> > _pads_out;
        std::map<std::string, std::vector<COEF_t> > _pads_in;

        void _tf2coefficients(transfer_function H);
        COEF_t _prewarp(COEF_t Wn);
        COEF_t _warp(COEF_t Wn);
}; 

}

#endif
