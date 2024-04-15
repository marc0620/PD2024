#include "floorplanner.h"
#include <cstddef>
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
  clock_t start = clock();
  floorplanner fp(alpha, argv[2], argv[3], argv[4]);
  fp.init();
  fp.plotresult("beforepack.svg", fp.getBlkNum() - 1);
  fp.pack();
  fp.plotresult("afterpack.svg", fp.getBlkNum() - 1);
  fp.SA();
  fp.revert();
  fp.pack();
  fp.plotresult("result.svg", fp.getBlkNum() - 1);
  clock_t end = clock();
  fp.eval();
  fp.writeOutput((double)(end - start) / CLOCKS_PER_SEC);
  return 0;
}
