#include <iostream>
#include <complex>
#include <map>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lambda/lambda.hpp>

#include "program_options.hh"
#include "digital_filter.hh"
#include "logging.hh"
#include "logger.hh"


using namespace jill;



  
digital_filter::digital_filter(): _coef_in(std::vector<COEF_t>(1,0)),
  _coef_out(),
  _pads_out(),
  _pads_in()
{}


void
digital_filter::filter_buf(sample_t const * const in, sample_t * const out, 
                           std::string port_name, nframes_t nframes) {



        
        memset(out, 0, nframes * sizeof(sample_t));
        
        if (!_pads_in.count(port_name)) {
                _pads_in[port_name].assign(pad_len(), 0);
        }
 
        if (is_iir() && !_pads_out.count(port_name)) {
                _pads_out[port_name].assign(pad_len(), 0);
        }

        if (is_iir()) {
                
                //double precision output buffer so filter remains stable
                static const nframes_t MaxNFrames = 10000;
                COEF_t precise_out[MaxNFrames] = {0};

                // ensure coefficient vectors are same size so index
                // doesn't go out of range
                if (_coef_in.size() < _coef_out.size()) {
                        _coef_in.resize(_coef_out.size());       
                }
                else if (_coef_in.size() > _coef_out.size() ) {
                        _coef_out.resize(_coef_in.size());
                } 
 
                for (nframes_t n = 0; n < nframes; n++) {

                        for (nframes_t i = 0; i <= pad_len(); i++) {                           
                                if (i == 0) {
                                        precise_out[n] = _coef_in[i] * in[n-i];
                                }
                                else if (i <= n) {
                                        precise_out[n] += _coef_in[i]*in[n-i] - _coef_out[i] * precise_out[n-i];      
                                }
                                else {
                                        precise_out[n] += _coef_in[i] * _pads_in[port_name][n-i + pad_len()] -      
                                                _coef_out[i] * _pads_out[port_name][n-i + pad_len()];
                                }       
                        }
                        precise_out[n] /= _coef_out[0];

                }
                std::copy(precise_out + nframes-pad_len(), precise_out + nframes, _pads_out[port_name].begin());
                std::copy(precise_out, precise_out + nframes, out);
                std::copy(in + nframes-pad_len(), in + nframes, _pads_in[port_name].begin());
                
        }
        else {
                for (nframes_t n = 0; n < nframes; n++) {
                        for (nframes_t i = 0; i <= pad_len(); i++) {
                                if (i <= n) {
                                        out[n] += _coef_in[i] * in[n-i];    
                                }
                                else {
                                        out[n] += _coef_in[i] *
                                                _pads_in[port_name][n-i + pad_len()];
                                }
                        }
                        if (_coef_out.size() > 0) {
                                out[n] /= _coef_out[0];
                        }
                }
                std::copy(in + nframes-pad_len(), in + nframes, _pads_in[port_name].begin());
        }     

}

void 
digital_filter::custom_coef(std::vector<COEF_t> b, 
                            std::vector<COEF_t> a) {
        _coef_in = b;
        _coef_out = a;
        log_coefs();
}


void 
digital_filter::reset_pads() {
        
        std::map<std::string, std::vector<COEF_t> >::iterator it_in, it_out;
        if (is_iir()) {
                it_out = _pads_out.begin();
        }
        
        // fix the bug in the for loop
        for (it_in = _pads_in.begin(); it_in != _pads_in.end(); it_in++) {
                it_in->second.assign(it_in->second.size(), 0);
                if (is_iir()) {
                        it_out->second.assign(it_out->second.size(), 0);
                }
        }
}
       
digital_filter::COEF_t
digital_filter::_prewarp(COEF_t Wn) {

  const COEF_t pi = std::arg(complex_t(-1,0));	
  return 2.0*std::tan(pi*Wn/2.0);
}

digital_filter::COEF_t
digital_filter::_warp(COEF_t Wn) {        
        return 2.0*std::atan(Wn / 2.0);                
}

void
digital_filter::_tf2coefficients(transfer_function H) {

        _coef_in.resize(H.num().size());
        _coef_out.resize(H.denom().size());

        for (int i = 0; i < H.num().size(); i++) {
                _coef_in[i] = H.num()[i]; 
        }
        for (int i = 0; i < H.denom().size(); i++) {
                _coef_out[i] = H.denom()[i];
        }
} 

void 
digital_filter::butter(int N, std::vector<COEF_t> Wc, std::string filter_type, nframes_t fs) {
  
        
        const complex_t i(0,1);
        const COEF_t pi = arg(complex_t(-1,0));	
        
        // normalized cutoff_frequencies
        std::vector<COEF_t> Wn(Wc.size());

        const COEF_t nyquist = static_cast<COEF_t>(fs)/2.0; 
        std::transform(Wc.begin(), 
                       Wc.end(), 
                       Wn.begin(),
                       boost::lambda::_1 / nyquist);

        //find poles
        std::vector<complex_t> p(N,complex_t(0,0));
        for (int k = 0; k < N; k++) {
                p[k] = std::exp(i*pi*(COEF_t)((2.0*k+N+1)/(2.0*N)));
        }
               
        std::vector<complex_t> z; //no zeros       
        COEF_t k = 1; // prototype system gain
        transfer_function H(z,p,k);
        
        std::vector<COEF_t> prewarped(Wn.size(),0);        
        
        for (int i = 0; i < Wn.size(); i++) {
                prewarped[i] = _prewarp(Wn[i]);
        }
        
        H.transform_prototype(prewarped, filter_type);

        H.bilinear();
     
        _tf2coefficients(H);       

        log_filter(N, Wc, filter_type, "butterworth");
}

void
digital_filter::log_filter(int N, std::vector<COEF_t> Wc, std::string filter_type, std::string filter_class){
        
        //printing polynomials for convenience
        LOG << "filter class: " << filter_class;
        LOG << "type: " << filter_type;
        LOG << "frequency cutoff(s) (Hz): " << poly(&Wc[0], Wc.size()-1);
        LOG << "order: " << N;        
        
        log_coefs();
}

void
digital_filter::log_coefs() {

        //printing polynomials for convenience
        LOG << "Numerator filter coefficients set to " 
            << poly(&_coef_in[0], _coef_in.size()-1); 
        LOG << "Denominator filter coefficients set to " 
            << poly(&_coef_out[0],_coef_out.size()-1);
}        
                
 
