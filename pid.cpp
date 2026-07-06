#include "pxt.h"

namespace PidController {
    class PidController {
    public:
        PidController(float Kp, float Ki, float Kd);
        float update(float setpoint, float measured_value, float dt);

        void setTunings(float Kp, float Ki, float Kd);
        void setOutputLimits(float min, float max);
        void reset();

    private:
        float Kp;
        float Ki;
        float Kd;

        float integral;
        float previous_error;

        float output_min;
        float output_max;
    };

    PidController::PidController(float Kp, float Ki, float Kd)
        : Kp(Kp), Ki(Ki), Kd(Kd), integral(0.0f), previous_error(0.0f),
          output_min(-1.0f), output_max(1.0f) {}

    void PidController::setTunings(float Kp, float Ki, float Kd) {
        this->Kp = Kp;
        this->Ki = Ki;
        this->Kd = Kd;
    }

    void PidController::setOutputLimits(float min, float max) {
        output_min = min;
        output_max = max;
    }

    void PidController::reset() {
        integral = 0.0f;
        previous_error = 0.0f;
    }

    /*
    * Updates the PID controller with the latest setpoint and measured value.
    * @param setpoint The desired value (e.g. desired rate in deg/s).
    * @param measured_value The actual value (e.g. gyro rate in deg/s).
    * @param dt The time step (seconds).
    * @return The PID output.
    */
    float PidController::update(float setpoint, float measured_value, float dt) {
        float error = setpoint - measured_value;
        integral += error * dt;
        float derivative = (error - previous_error) / dt;

        // update the PID output
        float output = (Kp * error) + (Ki * integral) + (Kd * derivative);

        // Clamp the output to the specified limits
        if (output > output_max) {
            output = output_max;
            integral -= error * dt; // Prevent integral windup
        } else if (output < output_min) {
            output = output_min;
            integral -= error * dt; // Prevent integral windup
        }

        previous_error = error;

        return output;
    }
}