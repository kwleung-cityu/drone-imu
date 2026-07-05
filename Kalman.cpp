/* This file is part of the MakeCode project porting from 
the original Kalman filter implementation by Kristian Lauszus, TKJ Electronics.
The original code is licensed under the GNU General Public License version 2 (GPL2).
*/

#include "pxt.h"

namespace KalmanFilter {
    class Kalman {
    public:
        Kalman();
        float getAngle(float newAngle, float newRate, float dt);

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

    //% shim=KalmanFilter::getAngle
    float getAngle(float newAngle, float newRate, float dt) {
        static Kalman filter;
        return filter.getAngle(newAngle, newRate, dt);
    }
}