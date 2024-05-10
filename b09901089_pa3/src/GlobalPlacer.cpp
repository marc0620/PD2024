#include "GlobalPlacer.h"

#include <cstdio>
#include <vector>

#include "ObjectiveFunction.h"
#include "Optimizer.h"
#include "Placement.h"
#include "Point.h"

GlobalPlacer::GlobalPlacer(Placement &placement) : _placement(placement) {}

void GlobalPlacer::place() {
    ////////////////////////////////////////////////////////////////////
    // This section is an example for analytical methods.
    // The objective is to minimize the following function:
    //      f(x,y) = 3*x^2 + 2*x*y + 2*y^2 + 7
    //
    // If you use other methods, you can skip and delete it directly.
    ////////////////////////////////////////////////////////////////////

    // Set initial point

    // Initialize the optimizer

    // Perform optimization, the termination condition is that the number of iterations reaches 100
    // TODO: You may need to change the termination condition, which is determined by the overflow ratio.

    ////////////////////////////////////////////////////////////////////
    // Global placement algorithm
    ////////////////////////////////////////////////////////////////////

    // TODO: Implement your global placement algorithm here.
    const double kAlpha = 0.0001;   // Constant step size
    int mode = 0;
    double M = 0;
    double lambda = 0.001;   // weight param
    double density_flexibility = 1.1;
    double gamma = 100.0;
    double wb = 100;

    const size_t num_modules = _placement.numModules();   // You may modify this line.
    std::vector<Point2<double>> positions(num_modules);   // Optimization variables (positions of modules). You may modify this line.
    double area_sum = 0;
    std::vector<bool> fixed(num_modules, false);
    for (int i = 0; i < num_modules; i++) {
        if (_placement.module(i).isFixed()) {
            fixed[i] = true;
        } else {
            _placement.module(i).setPosition((_placement.boundryBottom() + _placement.boundryTop()) / 2, (_placement.boundryLeft() + _placement.boundryRight()) / 2);
        }
        positions[i].x = _placement.module(i).x();
        positions[i].y = _placement.module(i).y();
        area_sum += _placement.module(i).width() * _placement.module(i).height();
    }
    plotPlacementResult("initplacement", false);
    M = area_sum / (_placement.boundryRight() - _placement.boundryLeft()) * (_placement.boundryTop() - _placement.boundryBottom()) * density_flexibility;

    cout << "start" << endl;
    ObjectiveFunction objfunc(_placement, lambda, M, mode, gamma, wb);   // Objective function
    SimpleConjugateGradient optimizer(objfunc, positions, kAlpha, fixed);
    optimizer.Initialize();
    for (int i = 0; i < 100; i++) {
        optimizer.Step();
        printf("Iteration %d: %f\n", i, objfunc(positions));
        for (int j = 0; j < num_modules; j++) {
            _placement.module(j).setPosition(positions[j].x, positions[j].y);
        }
        if (i < 10) {
            char filename[20];
            snprintf(filename, 20, "output_%d.plt", i);
            plotPlacementResult(filename, false);
            cout << "density: " << objfunc.getDense() << "wirelength: " << objfunc.getWire() << endl;
        }
    }

    ////////////////////////////////////////////////////////////////////
    // Write the placement result into the database. (You may modify this part.)
    ////////////////////////////////////////////////////////////////////
    for (size_t i = 0; i < num_modules; i++) {
        _placement.module(i).setPosition(positions[i].x, positions[i].y);
    }
}

void GlobalPlacer::plotPlacementResult(const string outfilename, bool isPrompt) {
    ofstream outfile(outfilename.c_str(), ios::out);
    outfile << " " << endl;
    outfile << "set title \"wirelength = " << _placement.computeHpwl() << "\"" << endl;
    outfile << "set size ratio 1" << endl;
    outfile << "set nokey" << endl << endl;
    outfile << "plot[:][:] '-' w l lt 3 lw 2, '-' w l lt 1" << endl << endl;
    outfile << "# bounding box" << endl;
    plotBoxPLT(outfile, _placement.boundryLeft(), _placement.boundryBottom(), _placement.boundryRight(), _placement.boundryTop());
    outfile << "EOF" << endl;
    outfile << "# modules" << endl << "0.00, 0.00" << endl << endl;
    for (size_t i = 0; i < _placement.numModules(); ++i) {
        Module &module = _placement.module(i);
        plotBoxPLT(outfile, module.x(), module.y(), module.x() + module.width(), module.y() + module.height());
    }
    outfile << "EOF" << endl;
    outfile << "pause -1 'Press any key to close.'" << endl;
    outfile.close();

    if (isPrompt) {
        char cmd[200];
        sprintf(cmd, "gnuplot %s", outfilename.c_str());
        if (!system(cmd)) {
            cout << "Fail to execute: \"" << cmd << "\"." << endl;
        }
    }
}

void GlobalPlacer::plotBoxPLT(ofstream &stream, double x1, double y1, double x2, double y2) {
    stream << x1 << ", " << y1 << endl << x2 << ", " << y1 << endl << x2 << ", " << y2 << endl << x1 << ", " << y2 << endl << x1 << ", " << y1 << endl << endl;
}
