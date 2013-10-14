
#include "digital_filter.hh"
#include <boost/noncopyable.hpp>
#include <jack/jack.h>
#include <string>
#include <map>
#include <vector>
#include <cstring>
#include <iostream>
#include <algorithm>


using namespace jill;

  
digital_filter::digital_filter(): coef_in(std::vector<sample_t>(1,0)),
  coef_out(),
  pads_out(),
  pads_in()
{}



void
digital_filter::filter_buf(sample_t const * const in, sample_t * const out, 
                std::string port_name, nframes_t nframes) {

        memset(out, 0, nframes * sizeof(sample_t));

        if (!pads_in.count(port_name)) {
                pads_in[port_name].assign(pad_len(), 0);
        }
 
        if (is_iir() && !pads_out.count(port_name)) {
                pads_out[port_name].assign(pad_len(), 0);
        }

        if (is_iir()) {

                // ensure coefficient vectors are same size so index
                // doesn't go out of range
                if (coef_in.size() < coef_out.size()) {
                        coef_in.resize(coef_out.size());       
                }
                else if (coef_in.size() > coef_out.size() ) {
                        coef_out.resize(coef_in.size());
                } 
 
                for (nframes_t n = 0; n < nframes; n++) {
                        for (nframes_t i = 0; i <= pad_len(); i++) {
                                if (i == 0) {
                                        out[n] = coef_in[i] * in[n-i];
                                }
                                else if (i <= n) {
                                        out[n] += coef_in[i]*in[n-i] - coef_out[i] * out[n-i];      
                                }
                                else {
                                        out[n] += coef_in[i] * pads_in[port_name][n-i + pad_len()] -      
                                                coef_out[i] * pads_out[port_name][n-i + pad_len()];
                                }       
                        }
                        out[n] /= coef_out[0];
                }

                std::copy(in + nframes-pad_len(), in + nframes, pads_in[port_name].begin());
                std::copy(out + nframes-pad_len(), out + nframes, pads_out[port_name].begin());
        }
        else {
                for (nframes_t n = 0; n < nframes; n++) {
                        for (nframes_t i = 0; i <= pad_len(); i++) {
                                if (i <= n) {
                                        out[n] += coef_in[i] * in[n-i];    
                                }
                                else {
                                        out[n] += coef_in[i] *
                                                pads_in[port_name][n-i + pad_len()];
                                }
                        }
                        if (coef_out.size() > 0) {
                                out[n] /= coef_out[0];
                        }
                }
                std::copy(in + nframes-pad_len(), in + nframes, pads_in[port_name].begin());
        }     

}

void 
digital_filter::custom_coef(std::vector<sample_t> coef_in_arg, 
                            std::vector<sample_t> coef_out_arg = std::vector<sample_t>()) {        
        coef_in = coef_in_arg;
        coef_out = coef_out_arg;;
}


void 
digital_filter::reset_pads() {
        std::map<std::string, std::vector<sample_t> >::iterator it_in, it_out;
        if (is_iir()) {
                it_out = pads_out.begin();
        }
        for (it_in = pads_in.begin(); it_in != pads_out.end(); it_in++) {
                it_in->second.assign(it_in->second.size(), 0);
                if (is_iir()) {
                        it_out->second.assign(it_out->second.size(), 0);
                }
        }
}
       





