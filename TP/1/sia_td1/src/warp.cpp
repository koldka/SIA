/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob

    Nori is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3
    as published by the Free Software Foundation.

    Nori is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <warp.h>
#include <vector.h>

Point2f Warp::squareToUniformSquare(const Point2f &sample) {
    return sample;
}

float Warp::squareToUniformSquarePdf(const Point2f &sample) {
    return ((sample.array() >= 0).all() && (sample.array() <= 1).all()) ? 1.0f : 0.0f;
}

Point2f Warp::squareToUniformDisk(const Point2f &sample) {

        double k1  = sample.array()[0];
        double k2  = sample.array()[1];
        double r   = sqrt(k1);
        double phi = 2*M_PI*k2;

        Point2f res = Point2f(r*cos(phi),r*sin(phi));
        return res;
}

float Warp::squareToUniformDiskPdf(const Point2f &p) {
    //throw RTException("Warp::squareToUniformDiskPdf() is not yet implemented!");
    double x  = p.array()[0];
    double y  = p.array()[1];
    double res = 0;
    if(x*x+y*y<=1)
    {
        res = INV_PI;
    }

    return res;
}

Vector3f Warp::squareToUniformHemisphere(const Point2f &sample) {

    double k1  = sample.array()[0];
    double k2  = sample.array()[1];
    double phi   = 2*M_PI*k1;
    double delta = acos(k2);

    Vector3f res = Vector3f(sin(delta)*cos(phi),sin(delta)*sin(phi),cos(delta));
    return res;

}

float Warp::squareToUniformHemispherePdf(const Vector3f &v) {
    double x  = v.array()[0];
    double y  = v.array()[1];
    double z  = v.array()[2];
    double res = 0;
    if(z>0) {
        res = INV_TWOPI;
    }
    return res;

}

Vector3f Warp::squareToCosineHemisphere(const Point2f &sample) {
    //throw RTException("Warp::squareToCosineHemisphere() is not yet implemented!");
    double k1  = sample.array()[0];
    double k2  = sample.array()[1];
    double phi   = 2*M_PI*k1;
    double delta = acos(sqrt(1-k2));

    Vector3f res = Vector3f(sin(delta)*cos(phi),sin(delta)*sin(phi),cos(delta));
    return res;
}

float Warp::squareToCosineHemispherePdf(const Vector3f &v) {
    //throw RTException("Warp::squareToCosineHemispherePdf() is not yet implemented!");
    double x  = v.array()[0];
    double y  = v.array()[1];
    double z  = v.array()[2];
    double res = 0;
    if(z>0) {
        res = INV_PI*z;
    }
    return res;
}
