#include "ObjectiveFunction.h"

#include "Placement.h"
#include "Point.h"
#include "cstdio"
#include <cmath>
#include <cstdlib>
#include <math.h>

Wirelength::Wirelength(Placement &placement, double gamma) : BaseFunction(placement.numModules()), placement_(placement), gamma_(gamma) {}
const double &Wirelength::operator()(const std::vector<Point2<double>> &input) {
    value_ = 0;
    clear_grad();
    for (int i = 0; i < placement_.numNets(); i++) {
        double sumX = 0, sumNX = 0, sumY = 0, sumNY = 0;
        for (int j = 0; j < placement_.net(i).numPins(); j++) {
            int moduleId = placement_.net(i).pin(j).moduleId();
            if (moduleId != -1) {
                sumX += exp((input[moduleId].x + placement_.net(i).pin(j).xOffset() + placement_.module(moduleId).width() / 2) / gamma_);
                sumNX += exp((-input[moduleId].x + placement_.net(i).pin(j).xOffset() + placement_.module(moduleId).width()) / gamma_);
                sumY += exp((input[moduleId].y + placement_.net(i).pin(j).yOffset() + placement_.module(moduleId).height() / 2) / gamma_);
                sumNY += exp((-input[moduleId].y + placement_.net(i).pin(j).yOffset() + placement_.module(moduleId).height() / 2) / gamma_);
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
                double pseudogradx = (exp((input[moduleId].x + placement_.net(i).pin(j).xOffset()) / gamma_) / sumX - exp((-input[moduleId].x + placement_.net(i).pin(j).xOffset()) / gamma_) / sumNX);
                grad_[moduleId].x += pseudogradx;
                double pseudogrady = (exp((input[moduleId].y + placement_.net(i).pin(j).yOffset()) / gamma_) / sumY - exp((-input[moduleId].y + placement_.net(i).pin(j).yOffset()) / gamma_) / sumNY);
                grad_[moduleId].y += pseudogrady;
            }

            // grad_[placement_.net(i).pin(j).moduleId()].x += (exp(placement_.net(i).pin(j).x()) / sumX - exp(-placement_.net(i).pin(j).x()) / sumNX) / gamma_;
            // grad_[placement_.net(i).pin(j).moduleId()].y += (exp(placement_.net(i).pin(j).y()) / sumY - exp(-placement_.net(i).pin(j).y()) / sumNY) / gamma_;
        }
    }
    // print value
    return value_;
}
void Wirelength::clear_grad() { std::fill(grad_.begin(), grad_.end(), 0); }
Density::Density(Placement &placement, int mode, double M, double wb) : BaseFunction(placement.numModules()), placement_(placement), mode(mode), M_(M), wb_(wb), temp_grad_(placement.numModules()) {
    row_num_ = (placement_.boundryTop() - placement_.boundryBottom()) / wb_ + 1;
    col_num_ = (placement_.boundryRight() - placement_.boundryLeft()) / wb_ + 1;
    bin_num_ = row_num_ * col_num_;
    cout << "init density" << endl;
    cout << "row_num_ " << row_num_ << endl;
    cout << "col_num_ " << col_num_ << endl;
    cout << "bin_num_ " << bin_num_ << endl;
    temp_grad_.resize(placement.numModules());
}
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
    double p = 0;
    if (d >= 2 * wb_ + w_or_h / 2) {
        if (axis == 0)
            temp_grad_[moduleId].x += 0;
        else
            temp_grad_[moduleId].y += 0;
        p = 0;
    } else if (d >= w_or_h / 2 + wb_ && d <= w_or_h / 2 + 2 * wb_) {
        double b = 2 / (wb_ * (w_or_h + 4 * wb_));
        if (axis == 0)
            temp_grad_[moduleId].x += 2 * b * (d - w_or_h / 2 - 2 * wb_) * sign;
        else
            temp_grad_[moduleId].y += 2 * b * d * sign;
        p = b * (d - w_or_h / 2 - 2 * wb_) * (d - w_or_h / 2 - 2 * wb_);
    } else {
        double a = 4 / ((w_or_h + 4 * wb_) * (w_or_h + 2 * wb_));
        if (axis == 0)
            temp_grad_[moduleId].x += 2 * a * d * sign;
        else
            temp_grad_[moduleId].y += 2 * a * d * sign;
        p = 1 - a * d * d;
    }
    return p;
}

double Density::smooth_density(Point2<double> pos, int moduleId, int idx) {
    Point2<double> coordinate = Idx_to_Coordinate(idx);
    double dx = (pos.x + placement_.module(moduleId).width() / 2 - coordinate.x);
    double dy = (pos.y + placement_.module(moduleId).height() / 2 - coordinate.y);
    double px = smooth_p(placement_.module(moduleId).width(), dx, moduleId, 0);
    double py = smooth_p(placement_.module(moduleId).height(), dy, moduleId, 1);
    return px * py * wb_ * wb_;
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
        // if(bin_density>1)
        //     printf("bin_density: %f\n", bin_density);
        // print calculated grad
    }
    return value_;
}
const std::vector<Point2<double>> &Density::Backward() { return grad_; }
void Density::clear_grad() { std::fill(grad_.begin(), grad_.end(), 0); }

ObjectiveFunction::ObjectiveFunction(Placement &placement, double lambda, double M, int mode, double gamma, double wb)
    : BaseFunction(placement.numModules()), placement_(placement), wirelength_(placement_, gamma), density_(placement_, mode, M, wb), lambda_(lambda) {
    cout << "init objectivefunction" << endl;
    density_.init(10);
}
const double &ObjectiveFunction::operator()(const std::vector<Point2<double>> &input) {
    // todo
    wirelength_(input);
    density_(input);
    value_ = wirelength_.value() + lambda_ * density_.value();
    printf("wirelength: %f\n", wirelength_.value());
    // print density value
    printf("density: %f\n", density_.value());
    return value_;
}
const std::vector<Point2<double>> &ObjectiveFunction::Backward() {
    // todo
    // print wirelength value

    for (int i = 0; i < placement_.numModules(); i++) {
        grad_[i].x = wirelength_.Backward().at(i).x + lambda_ * density_.Backward().at(i).x;
        grad_[i].y = wirelength_.Backward().at(i).y + lambda_ * density_.Backward().at(i).y;
    }
    for (int i = 0; i < placement_.numModules(); i++) {
        printf("grady: %f\n", grad_[i].y);
    }
    return grad_;
}