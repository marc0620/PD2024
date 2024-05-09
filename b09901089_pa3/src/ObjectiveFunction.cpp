#include "ObjectiveFunction.h"

#include "Placement.h"
#include "Point.h"
#include "cstdio"
#include <cmath>
#include <cstdlib>
#include <math.h>

ExampleFunction::ExampleFunction(Placement &placement) : BaseFunction(1), placement_(placement) {
    printf("Fetch the information you need from placement database.\n");
    printf("For example:\n");
    printf("    Placement boundary: (%.f,%.f)-(%.f,%.f)\n", placement_.boundryLeft(), placement_.boundryBottom(), placement_.boundryRight(), placement_.boundryTop());
}

const double &ExampleFunction::operator()(const std::vector<Point2<double>> &input) {
    // Compute the value of the function
    value_ = 3. * input[0].x * input[0].x + 2. * input[0].x * input[0].y + 2. * input[0].y * input[0].y + 7.;
    input_ = input;
    return value_;
}

const std::vector<Point2<double>> &ExampleFunction::Backward() {
    // Compute the gradient of the function
    grad_[0].x = 6. * input_[0].x + 2. * input_[0].y;
    grad_[0].y = 2. * input_[0].x + 4. * input_[0].y;
    return grad_;
}
Wirelength::Wirelength(Placement &placement) : BaseFunction(placement.numModules()), placement_(placement) {}
const double &Wirelength::operator()(const std::vector<Point2<double>> &input) {
    value_ = 0;
    clear_grad();
    for (int i = 0; i < placement_.numNets(); i++) {
        double sumX = 0, sumNX = 0, sumY = 0, sumNY = 0;
        for (int j = 0; j < placement_.net(i).numPins(); j++) {
            int moduleId = placement_.net(i).pin(j).moduleId();
            if (moduleId != -1) {
                sumX += exp((input[moduleId].x + placement_.net(i).pin(j).xOffset()) / gamma_);
                sumNX += exp((-input[moduleId].x + placement_.net(i).pin(j).xOffset()) / gamma_);
                sumY += exp((input[moduleId].y + placement_.net(i).pin(j).yOffset()) / gamma_);
                sumNY += exp((-input[moduleId].y + placement_.net(i).pin(j).yOffset()) / gamma_);
            } else {
                sumX += exp(placement_.net(i).pin(j).x() / gamma_);
                sumNX += exp(-placement_.net(i).pin(j).x() / gamma_);
                sumY += exp(placement_.net(i).pin(j).y() / gamma_);
                sumNY += exp(-placement_.net(i).pin(j).y() / gamma_);
            }
        }
        value_ += gamma_ * (log(sumX) + log(sumNX) + log(sumY) + log(sumNY));
        for (int j = 0; j < placement_.net(i).numPins(); j++) {
            int moduleId = placement_.net(i).pin(j).moduleId();
            if (moduleId != -1) {
                grad_[moduleId].x +=
                    (exp((input[moduleId].x + placement_.net(i).pin(j).xOffset()) / gamma_) / sumX - exp((-input[moduleId].x + placement_.net(i).pin(j).xOffset()) / gamma_) / sumNX) / gamma_;
                grad_[moduleId].y +=
                    (exp((input[moduleId].y + placement_.net(i).pin(j).yOffset()) / gamma_) / sumY - exp((-input[moduleId].y + placement_.net(i).pin(j).yOffset()) / gamma_) / sumNY) / gamma_;
            }
            // grad_[placement_.net(i).pin(j).moduleId()].x += (exp(placement_.net(i).pin(j).x()) / sumX - exp(-placement_.net(i).pin(j).x()) / sumNX) / gamma_;
            // grad_[placement_.net(i).pin(j).moduleId()].y += (exp(placement_.net(i).pin(j).y()) / sumY - exp(-placement_.net(i).pin(j).y()) / sumNY) / gamma_;
        }
    }
    return value_;
}
void Wirelength::clear_grad() { std::fill(grad_.begin(), grad_.end(), 0); }
Density::Density(Placement &placement, int mode, double M) : BaseFunction(placement_.numModules()), placement_(placement), mode(mode), M_(M) {}
const std::vector<Point2<double>> &Wirelength::Backward() { return grad_; }

Point2<double> Density::Idx_to_Coordinate(long long idx) {
    Point2<double> coordinate;
    coordinate.x = idx % col_num_ * wb_ + wb_ / 2;
    coordinate.y = (idx / col_num_) * wb_ + wb_ / 2;
    return coordinate;
}

double Density::smooth_p(double w_or_h, double d, int moduleId, int axis) {
    int sign = d > 0 ? 1 : -1;
    d = abs(d);
    if (d >= 2 * wb_ + w_or_h / 2) {
        if (axis == 0)
            temp_grad_[moduleId].x += 0;
        else
            temp_grad_[moduleId].y += 0;
        return 0;
    } else if (d >= w_or_h / 2 + wb_ && d <= w_or_h / 2 + 2 * wb_) {
        double b = 2 / (wb_ * (w_or_h + 4 * wb_));
        if (axis == 0)
            temp_grad_[moduleId].x += 2 * b * (d - w_or_h / 2 - 2 * wb_) * sign;
        else
            temp_grad_[moduleId].y += 2 * b * d * sign;
        return b * (d - w_or_h / 2 - 2 * wb_) * (d - w_or_h / 2 - 2 * wb_);
    } else {
        double a = 4 / ((w_or_h + 4 * wb_) * (w_or_h + 2 * wb_));
        if (axis == 0)
            temp_grad_[moduleId].x += 2 * a * d * sign;
        else
            temp_grad_[moduleId].y += 2 * a * d * sign;
        return 1 - a * d * d;
    }
}

double Density::smooth_density(Point2<double> pos, int moduleId, int idx) {
    Point2<double> coordinate = Idx_to_Coordinate(idx);
    double dx = (pos.x + placement_.module(moduleId).width() / 2 - coordinate.x);
    double dy = (pos.y + placement_.module(moduleId).height() / 2 - coordinate.y);
    double px = smooth_p(placement_.module(moduleId).width(), dx, moduleId, 0);
    double py = smooth_p(placement_.module(moduleId).height(), dy, moduleId, 1);
    return px * py * placement_.module(moduleId).width() * placement_.module(moduleId).height();
}
void Density::clear_temp_grad() { std::fill(temp_grad_.begin(), temp_grad_.end(), Point2<double>(0, 0)); }
const double &Density::operator()(const std::vector<Point2<double>> &input) {
    value_ = 0;
    clear_grad();
    for (int i = 0; i < bin_num_; i++) {
        clear_temp_grad();
        double bin_density = 0;
        for (int j = 0; j < placement_.numModules(); j++) {
            bin_density += smooth_density(input[j], j, i);
        }
        if (mode == 0) {
            value_ += (bin_density - M_) * (bin_density - M_);
            for (int j = 0; j < placement_.numModules(); j++) {
                grad_[j].x += 2 * (bin_density - M_) * temp_grad_[j].x;
                grad_[j].y += 2 * (bin_density - M_) * temp_grad_[j].y;
            }
        } else {
            if (bin_density > M_) {
                value_ += (bin_density - M_) * (bin_density - M_);
                for (int j = 0; j < placement_.numModules(); j++) {
                    grad_[j].x += 2 * (bin_density - M_) * temp_grad_[j].x;
                    grad_[j].y += 2 * (bin_density - M_) * temp_grad_[j].y;
                }
            } else {
                // don't update the grad and value
            }
        }
    }
    return value_;
}
const std::vector<Point2<double>> &Density::Backward() { return grad_; }
void Density::clear_grad() { std::fill(grad_.begin(), grad_.end(), 0); }

ObjectiveFunction::ObjectiveFunction(Placement &placement, double lambda, double M, int mode)
    : BaseFunction(placement.numModules()), placement_(placement), wirelength_(placement_), density_(placement_, mode, M), lambda_(lambda) {}
const double &ObjectiveFunction::operator()(const std::vector<Point2<double>> &input) {
    // todo
    value_ = wirelength_.value() + lambda_ * density_.value();
    return value_;
}
const std::vector<Point2<double>> &ObjectiveFunction::Backward() {
    // todo
    for (int i = 0; i < placement_.numModules(); i++) {
        grad_[i].x = wirelength_.grad().at(i).x + lambda_ * density_.grad().at(i).x;
        grad_[i].y = wirelength_.grad().at(i).y + lambda_ * density_.grad().at(i).y;
    }
    return grad_;
}