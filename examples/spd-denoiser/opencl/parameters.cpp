#include "parameters.h"
#if (defined(_MSC_VER) || defined(__INTEL_COMPILER)) && defined(_WIN32)
#include "XGetopt/XGetopt.h"
#endif

//#include <string>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
using namespace std;

//funzioni per il cheking dei parametri
bool err_w(int w){
  return ((w < 3) || (w > MAX_WINDOW_SIZE) || (w % 2 == 0));
}

bool err_alfa(float a){
  return !(a<=2 && a>0);
}

bool err_beta(float b){
  return !(b <=10.0 && b>=0.5);
}

bool err_max_cycles(int c) {
  return !(c <= MAX_CYCLES && c >= 1);
}

// bool err_bmp(string &nomefile) {
//   unsigned long  l = nomefile.length();
//   return l<5 || (string("bmp").compare(nomefile.substr(l - 3, 3)) != 0);
// }

bool err_noise(int noise) {
  return !(noise >= 1 && noise <= 99);
}

void print_help(char *exe) {
#if defined(FROMFILE) || defined(BITMAP)
  cout << "Usage: " << exe << " [options] <file>" << endl
       << "Non-optional arguments:" << endl
       << "<file>\t\tinput filename" << endl;
#else
  cout << "Usage: " << exe << " [options]" << endl;
#endif
  cout << "Allowed options:" << endl
       << "-h\t\tthis help message" << endl
       << "-v\t\tverbose mode" << endl
       << "-s\t\tshow input and output bitmaps" << endl
       << "-n arg\t\tnoise-% (1 <= arg <= 99)" << endl
       << "-a arg\t\talpha (1 < arg <= 2) "
       << "[default: " << ALFA_DEFAULT << "]" << endl
       << "-b arg\t\tbeta (0.5 <= arg <= 10) "
       << "[default: " << BETA_DEFAULT << "]" << endl
       << "-w arg\t\tcontrol-window size (3 <= arg <= " << MAX_WINDOW_SIZE << ") "
       << "[default: " << MAX_WINDOW_SIZE << "]" << endl
       << "-c arg\t\tmax cycles of restoration (1 <= arg <= 2000) "
       << "[default: " << MAX_CYCLES_DEFAULT << "]" << endl
       << "-f\t\tfix n. of cycles to max cycles" << endl
       << "-o arg\t\toutput filename (<filename>." << OUTFILE_EXT << ")" << endl;
}

//parsing
void get_arguments(char *argv[], int argc, arguments &args) {
  args.alfa = ALFA_DEFAULT;
  args.beta = BETA_DEFAULT;
  args.w_max = MAX_WINDOW_SIZE;
  args.max_cycles = MAX_CYCLES_DEFAULT;
  args.fixed_cycles = false;
  args.verbose = false;
  args.user_out_fname = false;
  args.show_enabled = false;
  args.noise = 0;

#if (defined(_MSC_VER) || defined(__INTEL_COMPILER)) && defined(_WIN32)
  TCHAR *options = "a:b:w:c:fo:n:svh";
#else
  const char *options = "a:b:w:c:fo:n:svh";
#endif
  int opt;

#if (defined(_MSC_VER) || defined(__INTEL_COMPILER)) && defined(_WIN32)
  int opterr = 0;
#else
  int opterr = 0;
#endif

  while((opt = getopt(argc, argv, options)) != -1) {
    switch(opt) {
    case 'a': //alpha
      args.alfa = (float)atof(optarg);
      if(err_alfa(args.alfa)) {
	cerr << "ERROR in argument: a" << endl;
	print_help(argv[0]);
	exit(1);
      }
      break;
    case 'b': //beta
      args.beta = (float)atof(optarg);
      if(err_beta(args.beta)){
	cerr << "ERROR in argument: b" << endl;
	print_help(argv[0]);
	exit(1);
      }
      break;
    case 'w': //window size
      args.w_max = atoi(optarg);
      if(err_w(args.w_max)) {
	cerr << "ERROR in argument: w" << endl;
	print_help(argv[0]);
	exit(1);
      }
      break;
    case 'c': //cycles
      args.max_cycles = atoi(optarg);
      if(err_max_cycles(args.max_cycles)) {
	cerr << "ERROR in argument: c" << endl;
	print_help(argv[0]);
	exit(1);
      }
      break;
    case 'f': //fixed cycles
      args.fixed_cycles = true;
      break;
    case 'o': //output fname
      args.user_out_fname = true;
      args.out_fname.append(optarg); //file
      // if(err_bmp(args.out_fname)) {
      // 	cerr << "ERROR: output file not valid" << endl;
      // 	exit(1);
      // }
      break;
    case 'n': //noise-%
      args.add_noise = true;
      args.noise = atoi(optarg);
      if(err_noise(args.noise)) {
	cerr << "ERROR: noise-% not valid" << endl;
	exit(1);
      }
      break;
    case 'h': //help
      print_help(argv[0]);
      exit(0);
    case 'v': //verbose
      args.verbose = true;
      break;
    case 's': //show
      args.show_enabled = true;
      break;
    case '?': //parsing error
      cerr << "Illegal options" << endl;
      print_help(argv[0]);
      exit(1);
    }
  }
  //parse non-optional arguments
#if defined(FROMFILE) || defined(BITMAP) || defined(BITMAPCOLOR)
  if(optind < argc) {
    args.fname.append(argv[optind]); //file
    // if(err_bmp(args.fname)) {
    //   cerr << "ERROR: file not valid" << endl;
    //   exit(1);
    // }
  }
  else {
    cerr << "ERROR: no input file provided" << endl;
    exit(1);
  }
#endif
}
