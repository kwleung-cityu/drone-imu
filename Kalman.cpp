/* 
This file is part of the MakeCode project porting from the original Kalman filter implementation by 
Kristian Lauszus, TKJ Electronics.
Web: https://github.com/TKJElectronics/KalmanFilter
*/

/*
 Use float for the Kalman filter calculations to maintain precision and avoid rounding errors. 
 The Kalman filter is sensitive to numerical stability, and using float ensures that the calculations are performed 
 with sufficient accuracy. 
 nRF52 series microcontrollers have hardware support for FPU (Floating Point Unit), 
 making float a suitable choice for this application.
 To force using hardware FPU, you can use the following compiler flags: -mfloat-abi=hard -mfpu=fpv4-sp-d16
 Use `float` over `double` because the nRF52 series microcontrollers have a 32-bit architecture:
 declaration of `double` will be treated as `float` by the compiler, 
 and using `double` may lead to unnecessary overhead without any precision gain.
 Also declare float variables with suffix `f` to ensure they are treated as float literals, 
 e.g., `0.001f` instead of `0.001`.
*/

#include "pxt.h"

namespace KalmanFilter {
    class Kalman {
    public:
        Kalman();
        float getAngle(float newAngle, float newRate, float dt); // The angle should be in degrees and the rate should be in degrees per second and the delta time in seconds
        void setAngle(float angle); // Used to set angle, this should be set as the starting angle
        float getRate(); // Return the unbiased rate

        // These are used to tune the Kalman filter
        void setQangle(float Q_angle);
        void setQbias(float Q_bias);
        void setRmeasure(float R_measure);
        float getQangle();
        float getQbias();
        float getRmeasure();

    private:
        float Q_angle; // Process noise variance for the accelerometer
        float Q_bias;  // Process noise variance for the gyro bias
        float R_measure; // Measurement noise variance

        float angle; // The angle calculated by the Kalman filter
        float bias;  // The gyro bias calculated by the Kalman filter
        float rate;  // Unbiased rate calculated from the rate and the calculated bias

        float P[2][2]; // Error covariance matrix
    };

    Kalman::Kalman() {
        Q_angle = 0.001f;
        Q_bias = 0.003f;
        R_measure = 0.03f;

        angle = 0.0f;
        bias = 0.0f;
        rate = 0.0f;

        P[0][0] = 0.0f;
        P[0][1] = 0.0f;
        P[1][0] = 0.0f;
        P[1][1] = 0.0f;
    }
    // Singleton instance of the Kalman filter
    // instance() is cleaner and safer for growth:
    // avoids duplicating static Kalman filter in multiple wrappers
    static Kalman& instance() {
        static Kalman filter;
        return filter;
    }

    float Kalman::getAngle(float newAngle, float newRate, float dt) {
        rate = newRate - bias;
        angle += dt * rate;

        P[0][0] += dt * (dt * P[1][1] - P[0][1] - P[1][0] + Q_angle);
        P[0][1] -= dt * P[1][1];
        P[1][0] -= dt * P[1][1];
        P[1][1] += Q_bias * dt;

        float S = P[0][0] + R_measure;
        float K[2];
        K[0] = P[0][0] / S;
        K[1] = P[1][0] / S;

        float y = newAngle - angle;
        angle += K[0] * y;
        bias += K[1] * y;

        float P00_temp = P[0][0];
        float P01_temp = P[0][1];

        P[0][0] -= K[0] * P00_temp;
        P[0][1] -= K[0] * P01_temp;
        P[1][0] -= K[1] * P00_temp;
        P[1][1] -= K[1] * P01_temp;

        return angle;
    }
    void Kalman::setAngle(float angle) { this->angle = angle; }
    float Kalman::getRate() { return this->rate; }

    // These are used to tune the Kalman filter
    void Kalman::setQangle(float Q_angle) { this->Q_angle = Q_angle; }
    void Kalman::setQbias(float Q_bias) { this->Q_bias = Q_bias; }
    void Kalman::setRmeasure(float R_measure) { this->R_measure = R_measure; }
    float Kalman::getQangle() { return this->Q_angle; }
    float Kalman::getQbias() { return this->Q_bias; }
    float Kalman::getRmeasure() { return this->R_measure; }

    //% shim=KalmanFilter::getAngle with the angle in degrees, the rate in degrees per second, and the delta time in seconds
    float getAngle(float newAngle, float newRate, float dt) {
        return instance().getAngle(newAngle, newRate, dt);
    }

    //% shim=KalmanFilter::setAngle
    void setAngle(float angle) {
        instance().setAngle(angle);
    }

    //% shim=KalmanFilter::getRate
    float getRate() {
        return instance().getRate();
    }

    //% shim=KalmanFilter::setQangle
    void setQangle(float Q_angle) {
        instance().setQangle(Q_angle);
    }

    //% shim=KalmanFilter::setQbias
    void setQbias(float Q_bias) {
        instance().setQbias(Q_bias);
    }

    //% shim=KalmanFilter::setRmeasure
    void setRmeasure(float R_measure) {
        instance().setRmeasure(R_measure);
    }

    //% shim=KalmanFilter::getQangle
    float getQangle() {
        return instance().getQangle();
    }

    //% shim=KalmanFilter::getQbias
    float getQbias() {
        return instance().getQbias();
    }

    //% shim=KalmanFilter::getRmeasure
    float getRmeasure() {
        return instance().getRmeasure();
    }   
}
// end of namespace KalmanFilter
