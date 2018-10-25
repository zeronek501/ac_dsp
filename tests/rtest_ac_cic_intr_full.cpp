//***********************************************************************************
// Source:           rtest_ac_cic_intr_full.cpp                                     *
// Description:      C++ test bench                                                 *
//                                                                                  *
//***********************************************************************************

// To compile and run:
//   $MGC_HOME/bin/g++ -I$MGC_HOME/shared/include rtest_ac_cic_intr_full.cpp -o ac_cic_intr_full
//   ./ac_cic_intr_full

#include <ac_dsp/ac_cic_intr_full.h>
#include <ac_channel.h>
#include <fstream>
#include <ac_fixed.h>

#include "ac_cic_intr_full_param.h"

#include <iostream>
using namespace std;

#define CW_CHECK_STREAM(istr, fname, desc) if (istr.fail()) { cerr << "Could not open " << desc << " file '" << fname << endl; return(-1); }

// Define a custom absolute value function for double values. This is done because using the built-in "abs" function for doubles sometimes causes naming conflicts.
double custom_abs(double input)
{
  return input < 0 ? -input : input;
}

int main(int argc, char *argv[])
{
  unsigned err_cnt = 0;
  fstream inf, ref;
  cout << "=============================================================================" << endl;
  cout << "------------------------------ Running  rtest_ac_cic_intr_full.cpp ----------------------------------" << endl;
  cout << "=============================================================================" << endl;
  cout << "Testing class: ac_cic_intr_full" << endl;

  string input_file = "ac_cic_intr_full_input.txt";
  string output_ref = "ac_cic_intr_full_ref.txt";

  ac_channel < IN_TYPE_TB > fixed_in;
  ac_channel < OUT_TYPE_TB > fixed_out;
  ac_channel < long double > fixed_ref;

  double big_diff = 0, data_in, diff;
  long double ref_in;

  // Read input values
  cout << "Reading input vectors from: " << input_file << endl;
  inf.open(input_file.c_str(), fstream::in);
  CW_CHECK_STREAM(inf,input_file,"Input data");
#if 0
  do {
    inf >> data_in;
    if (! inf.eof()) fixed_in.write(data_in);
  } while (! inf.eof());
#else
  for (int inread=0;inread<1000;inread++) {
    inf >> data_in;
    fixed_in.write(data_in);
  }
#endif
  inf.close();

  // Read reference values
  cout << "Reading output reference from: " << output_ref << endl;
  ref.open(output_ref.c_str(), fstream::in);
  CW_CHECK_STREAM(ref,output_ref,"Matlab reference output");
  for (int refread=0;refread<N_TB;refread++) { ref >> ref_in; } // throw away first N_TB outputs
  do {
    long double ref_in;
    ref >> ref_in;
    if (!ref.eof()) fixed_ref.write(ref_in);
  } while (! ref.eof());
  ref.close();

  cout << "Number of inputs = " << fixed_in.debug_size() << endl;
  cout << "Number of reference outputs = " << fixed_ref.debug_size() << endl;

  ac_cic_intr_full < IN_TYPE_TB, OUT_TYPE_TB, R_TB, M_TB, N_TB > filter;
  filter.run(fixed_in, fixed_out);

  cout << "Number of DUT outputs = " << fixed_out.debug_size() << endl << endl;

  // Make sure that the design outputs more values than/the same number of values as the reference.
  if (fixed_out.debug_size() < fixed_ref.debug_size()) { 
    cout << "Fewer outputs produced : reference = " << fixed_ref.debug_size() << " values, DUT = " << fixed_out.debug_size() << " values." << endl;
//  err_cnt++;
  }

  double n_output = 0;

  while (fixed_out.available(1) && fixed_ref.available(1)) {
    n_output++;
    OUT_TYPE_TB fixed_data_out = fixed_out.read(); // get DUT output

    long double ref_in = fixed_ref.read(); // get reference output
    diff = custom_abs(ref_in - fixed_data_out.to_double());

    if (diff != 0) {
//    cout << "	@ERROR: Data mismatch on sample " << n_output << " : expected   " << ref_in << "  Received : " << fixed_data_out << endl;
      err_cnt++;
    }

    if (diff > big_diff) {
      big_diff = diff;
    }
  }

  if (err_cnt) {
    cout << "Test FAILED. Worst case absolute difference = " << big_diff << endl;
  } else {
    cout << "Test PASSED." << endl;
  }

  return(err_cnt);
}
