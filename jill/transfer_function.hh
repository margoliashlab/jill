#ifndef _TRANSFER_FUNCTION_HH
#define _TRANSFER__FUNCTION_HH 1

#include <boost/math/tools/polynomial.hpp>
#include <jack/jack.h>
#include <vector>
#include <complex>
#include <algorithm>
#include <string>

namespace jill {

class transfer_function {

public: 
 
        typedef jack_default_audio_sample_t sample_t;
        typedef jack_nframes_t nframes_t;
        typedef std::complex<sample_t> complex_t;
        typedef boost::math::tools::polynomial<complex_t> complex_poly;
        typedef boost::math::tools::polynomial<sample_t> poly;
       
        transfer_function(poly b, poly a);
        
        //constructor taking zeros, poles, and system, respectively as arguments
        transfer_function(std::vector<complex_t> z, 
                          std::vector<complex_t> p,
                          sample_t k);

        ~transfer_function(){}
                       
        poly num(){return _numerator;}
        poly denom(){return _denominator;}
        bool is_analog(){return _is_analog_bool;}


        void transform(poly transform_num, poly transform_denom);
 
        void normalize();
        void bilinear();
        
        /* Normalizes so that constant term of denominator is 1*/
        
        void lp2lp(sample_t Wn);
        void lp2hp(sample_t Wn);
        void lp2bp(sample_t W1, sample_t W2);
        void lp2bs(sample_t W1, sample_t W2);

        void transform_prototype(std::vector<sample_t> Wn, std::string filter_type);
        
   
protected:
        poly _numerator;
        poly _denominator;
        bool _is_analog_bool;
}; 

} //namespace jill

#endif
