
#ifndef _FILTER_HH
#define _FILTER_HH 1

#include <boost/noncopyable.hpp>
#include <boost/math/tools/polynomial.hpp>
#include <jack/jack.h>
#include <string>
#include <map>
#include <vector>
#include <complex>
#include <algorithm>

namespace jill {

        typedef jack_default_audio_sample_t sample_t;
        typedef jack_nframes_t nframes_t;
        typedef std::complex<sample_t> complex;
        typedef boost::math::tools::polynomial<complex> complex_poly;
        typedef boost::math::tools::polynomial<sample_t> poly;

class transfer_function {

public: 
 
        typedef jack_default_audio_sample_t sample_t;
        typedef jack_nframes_t nframes_t;
        typedef std::complex<sample_t> complex;
        typedef boost::math::tools::polynomial<complex> complex_poly;
        typedef boost::math::tools::polynomial<sample_t> poly;
       
        transfer_function(poly b, poly a);
        
        //constructor taking zeros, poles, and system, respectively as arguments
        transfer_function(std::vector<complex> z, 
                          std::vector<complex> p,
                          sample_t k);

        ~transfer_function(){;}
                       
        poly num(){return _numerator;}
        poly denom(){return _denominator;}
        bool is_analog(){return _is_analog_bool;}

        void bilinear(nframes_t fs);
        
        void lp2lp(sample_t Wn_old, sample_t Wn_new);

        /* Normalizes so that constant term of denominator is 1*/
        void normalize();
 
   
protected:
        poly _numerator;
        poly _denominator;
        bool _is_analog_bool;
}; 

class digital_filter : boost::noncopyable {
public:



        typedef jack_default_audio_sample_t sample_t;
        typedef jack_nframes_t nframes_t;
        typedef std::complex<sample_t> complex;
        typedef boost::math::tools::polynomial<complex> complex_poly;
        typedef boost::math::tools::polynomial<sample_t> poly;
        digital_filter();

        
        void 
        filter_buf(sample_t const * const in, sample_t * const out, 
                        std::string port_name, nframes_t nframes);
                    
        void reset_pads(); 

        void butter();

        bool is_iir() {return _coef_out.size() >= 1;}
        nframes_t pad_len() {return std::max(_coef_in.size(), _coef_out.size()) - 1;}

        void
        custom_coef(std::vector<sample_t>, std::vector<sample_t>);


protected:
        
        std::vector<sample_t> _coef_in;
        std::vector<sample_t> _coef_out;
        
        std::map<std::string, std::vector<sample_t> > _pads_out;
        std::map<std::string, std::vector<sample_t> > _pads_in;

}; 

}

#endif
