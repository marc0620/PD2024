#include "floorplanner.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

int main(int argc, char **argv) {
  fstream input_blk, input_net, output;
  double alpha;

  if (argc == 5) {
    alpha = stod(argv[1]);
  } else {
    cerr << "Usage: ./Floorplanner <alpha> <input block file> " << "<input net file> <output file>" << endl;
    exit(1);
  }
  floorplanner fp(alpha, argv[2], argv[3], argv[4]);
  fp.init();
  return 0;
}