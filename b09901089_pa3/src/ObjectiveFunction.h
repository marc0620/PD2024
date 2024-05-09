#define _GLIBCXX_USE_CXX11_ABI 0   // Align the ABI version to avoid compatibility issues with `Placment.h`
#ifndef OBJECTIVEFUNCTION_H
#define OBJECTIVEFUNCTION_H

#include <vector>

#include "Placement.h"
#include "Point.h"

/**
 * @brief Base class for objective functions
 */
class BaseFunction {
  public:
    /////////////////////////////////
    // Conssutructors
    /////////////////////////////////

    BaseFunction(const size_t &input_size) : grad_(input_size) {}

    /////////////////////////////////
    // Accessors
    /////////////////////////////////

    const std::vector<Point2<double>> &grad() const { return grad_; }
    const double &value() const { return value_; }

    /////////////////////////////////
    // Methods
    /////////////////////////////////

    // Forward pass, compute the value of the function
    virtual const double &operator()(const std::vector<Point2<double>> &input) = 0;

    // Backward pass, compute the gradient of the function
    virtual const std::vector<Point2<double>> &Backward() = 0;

  protected:
    /////////////////////////////////
    // Data members
    /////////////////////////////////

    std::vector<Point2<double>> grad_;   // Gradient of the function
    double value_;                       // Value of the function
};

/**
 * @brief Example function for optimization
 *
 * This is a simple example function for optimization. The function is defined as:
 *      f(t) = 3*t.x^2 + 2*t.x*t.y + 2*t.y^2 + 7
 */
class ExampleFunction : public BaseFunction {
  public:
    /////////////////////////////////
    // Constructors
    /////////////////////////////////

    ExampleFunction(Placement &placement);

    /////////////////////////////////
    // Methods
    /////////////////////////////////

    const double &operator()(const std::vector<Point2<double>> &input) override;
    const std::vector<Point2<double>> &Backward() override;

  private:
    /////////////////////////////////
    // Data members
    /////////////////////////////////

    std::vector<Point2<double>> input_;   // Cache the input for backward pass
    Placement &placement_;
};

/**
 * @brief Wirelength function
 */
class Wirelength : public BaseFunction {
    // TODO: Implement the wirelength function, add necessary data members for caching
  public:
    /////////////////////////////////
    // Methods
    /////////////////////////////////
    void clear_grad();
    Wirelength(Placement &placement);
    const double &operator()(const std::vector<Point2<double>> &input) override;
    const std::vector<Point2<double>> &Backward() override;
    void set_gamma(const double &gamma) { gamma_ = gamma; }

  private:
    Placement &placement_;
    double gamma_ = 1.0;
};

/**
 * @brief Density function
 */
class Density : public BaseFunction {
    // TODO: Implement the density function, add necessary data members for caching
  public:
    /////////////////////////////////
    // Methods
    /////////////////////////////////
    Density(Placement &placement, int mode, double M_);
    void clear_grad();
    void clear_temp_grad();
    void init(const int &wb) {
        wb_ = wb;
        row_num_ = (placement_.boundryTop() - placement_.boundryBottom()) / wb_ + 1;
        col_num_ = (placement_.boundryRight() - placement_.boundryLeft()) / wb_ + 1;
        bin_num_ = row_num_ * col_num_;
    }
    double smooth_p(double w_or_h, double d, int moduleId, int axis);   // axis = 0 for x, 1 for y
    // double smooth_p_d(double w_or_h, double d, int moduleId, int axis);
    double smooth_density(Point2<double> pos, int moduleId, int idx);
    const double &operator()(const std::vector<Point2<double>> &input) override;
    const std::vector<Point2<double>> &Backward() override;
    Point2<double> Idx_to_Coordinate(long long idx);

  private:
    Placement &placement_;
    double wb_ = 10;
    int row_num_;
    int col_num_;
    long long bin_num_;
    double M_;
    int mode;   // 0 for bi-directional, 1 for uni-
    vector<Point2<double>> temp_grad_;
};

/**
 * @brief Objective function for global placement
 */
class ObjectiveFunction : public BaseFunction {
    // TODO: Implement the objective function for global placement, add necessary data
    // members for caching
    //
    // Hint: The objetive function of global placement is as follows:
    //       f(t) = wirelength(t) + lambda * density(t),
    // where t is the positions of the modules, and lambda is the penalty weight.
    // You may need an interface to update the penalty weight (lambda) dynamically.
  public:
    /////////////////////////////////
    // Methods
    /////////////////////////////////
    ObjectiveFunction(Placement &placement, double lambda, double M, int mode);
    void setlambda(const double &lambda) { lambda_ = lambda; }
    const double &operator()(const std::vector<Point2<double>> &input) override;
    const std::vector<Point2<double>> &Backward() override;

  private:
    Wirelength wirelength_;
    Density density_;
    Placement placement_;
    double lambda_ = 1.0;
};

#endif   // OBJECTIVEFUNCTION_H
