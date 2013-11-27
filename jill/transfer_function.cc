#include "transfer_function.hh"

using namespace jill;


transfer_function::transfer_function(poly b, poly a) {
        _numerator=b;
        _denominator=a;
        _is_analog_bool=true;
}

transfer_function::transfer_function(std::vector<complex_t> z, 
                                     std::vector<complex_t> p,
                                     sample_t k) {
        complex_t identity[0];
        identity[0] = complex_t(1,0);
        complex_poly num(identity, 0);
        complex_poly denom(identity, 0);
        for (int i = 0; i < std::max(z.size(), p.size()); ++i) {
                if (i < z.size()) {
                        complex_t values[2] = {-z[i],1};      
                        num *= complex_poly(values, 1);
                }
                if (i < p.size()) {
                        complex_t values[2] = {-p[i],1};
                        denom *= complex_poly(values, 1);
                }
        }

        num *= identity[0] * k;

        // coefficients should now be real
        //TODO - add exception if not real or close to real

        sample_t real_num[num.size()];
        sample_t real_denom[denom.size()];
        for (int i = 0; i < std::max(num.size(), 
                                     denom.size()); i++) {
                if (i < num.size()) {
      
                        real_num[i] = num[i].real();

                }
                if (i < denom.size()){
                        real_denom[i] = denom[i].real();
                }
        }

        _numerator = poly(real_num, num.degree());
        _denominator = poly(real_denom, denom.degree());
        _is_analog_bool = true;

}

void
transfer_function::normalize(){
        /* Normalizes so that constant term of denominator is 1*/
        _numerator *= (1.0/_denominator[0]);
        _denominator *= (1.0/_denominator[0]); 
}




void 
transfer_function::transform(poly transform_num, poly transform_denom){
       
       
       // denominator of transform raised to nth power is multipled by
       // with the numerator and denominator of the transfer function in
       // order to put the transfer function back into rational form
       int n = std::max(_numerator.degree(), _denominator.degree());
       
       poly new_num(0);
       poly new_denom(0);
       for (int k = 0; k < std::max(_numerator.size(), 
                                    _denominator.size()); k++) {
               poly kth_term(1);
               for (int i = 0; i < k; i++) {
                       kth_term *= transform_num;
               }
               for (int i = 0; i < n-k; i++) {
                       kth_term *= transform_denom;
               }      
               if (k < _numerator.size()) {
                       new_num += _numerator[k] * kth_term;
               }
               if (k < _denominator.size()) {
                       new_denom += _denominator[k] * kth_term;

               }
       }
       _numerator=new_num;
       _denominator=new_denom;
       normalize();     
}

void
transfer_function::bilinear() {
       
       const int n = 1; //degree of polynomials in transform 

       sample_t num[n+1] = {2, -2};
       sample_t denom[n+1] = {1, 1};
       transform(poly(num,n), poly(denom,n));

       _is_analog_bool=false;  
 }

void
transfer_function::lp2lp(sample_t Wn) {

        sample_t num[2] = {0, 1};
        sample_t denom[2] = {Wn, 0};
        transform(poly(num,1), poly(denom,1));
}


void
transfer_function::lp2hp(sample_t Wn) {

        sample_t num[2] = {Wn, 0};
        sample_t denom[2] = {0, 1};
        transform(poly(num, 1), poly(denom, 1));       
}

void 
transfer_function::lp2bp(sample_t W1, sample_t W2) {

        if (W1 > W2) std::swap(W1,W2);                    
 
        sample_t num[3] = {W1*W2, 0, 1};
        sample_t denom[2] = {0, W2-W1};
        transform(poly(num, 2), poly(denom, 1));

}


void 
transfer_function::lp2bs(sample_t W1, sample_t W2) {

        if (W1 > W2) std::swap(W1,W2);

        sample_t num[2] = {0, W2-W1};
        sample_t denom[3] = {W1*W2, 0, 1}; 
        transform(poly(num, 1), poly(denom, 2));

}

void 
transfer_function::transform_prototype(std::vector<sample_t> Wn, 
                                       std::string filter_type) {
                                               
        if (filter_type.compare("low-pass")==0) {
                lp2lp(Wn[0]);          
        }
        else if (filter_type.compare("high-pass")==0) {
                lp2hp(Wn[0]);
        }
        else if (filter_type.compare("band-pass")==0){
                lp2bp(Wn[0], Wn[1]);
        }
        else if (filter_type.compare("band-stop")==0) {
                lp2bs(Wn[0], Wn[1]);
        }
}
