#pragma once

#include "aerial_autonomy/common/conversions.h"
#include "aerial_autonomy/common/math.h"
#include "aerial_autonomy/controller_connectors/base_controller_connector.h"
#include "aerial_autonomy/controllers/qrotor_backstepping_controller.h"
#include "aerial_autonomy/estimators/thrust_gain_estimator.h"
#include "aerial_autonomy/sensors/base_sensor.h"
#include "aerial_autonomy/types/roll_pitch_yawrate_thrust.h"
#include "qrotor_backstepping_controller_config.pb.h"

#include <Eigen/Cholesky>
#include <Eigen/Dense>
#include <chrono>
#include <glog/logging.h>
#include <parsernode/parser.h>
/**
* @brief A controller connector for trajectory-tracking backstepping controller
* for a quadrotor.
* Based on <Kobilarov, "Trajectory tracking of a class of underactuated systems
* with
* external disturbances", American Control Conference, 2013>
*/
class QrotorBacksteppingControllerConnector
    : public ControllerConnector<
          std::pair<double, QrotorBacksteppingState>, // SensorDataType
          std::shared_ptr<ReferenceTrajectory<ParticleState, Snap>>, // GoalType
          QrotorBacksteppingControl> { // ControlType
public:
  /**
  * @brief Constructor
  * @param drone_hardware Plugin used to send commands to UAV
  * @param controller Position controller that gives rpyt commands
  */
  QrotorBacksteppingControllerConnector(
      parsernode::Parser &drone_hardware,
      QrotorBacksteppingController &controller,
      ThrustGainEstimator &thrust_gain_estimator,
      QrotorBacksteppingControllerConfig config,
      SensorPtr<tf::StampedTransform> pose_sensor = nullptr)
      : ControllerConnector(controller, ControllerGroup::UAV),
        drone_hardware_(drone_hardware),
        thrust_gain_estimator_(thrust_gain_estimator), config_(config),
        t_0_(std::chrono::high_resolution_clock::now()), m_(config_.mass()),
        g_(config_.acc_gravity()), pose_sensor_(pose_sensor) {
    DATA_HEADER("qrotor_backstepping_controller_connector") << "roll"
                                                            << "pitch"
                                                            << "yaw"
                                                            << "roll_cmd"
                                                            << "pitch_cmd"
                                                            << "yaw_rate_cmd"
                                                            << "thrust"
                                                            << DataStream::endl;
  }
  /**
  * @brief Set goal to controller and clear estimator buffer
  *
  * @param goal empty goal
  */
  void setGoal(std::shared_ptr<ReferenceTrajectory<ParticleState, Snap>> goal);
  /**
  * @brief Get last command
  */
  geometry_msgs::Quaternion getLastCommand() { return rpyt_message_; }
  /**
  * @brief Swap out the internal sensor with provided sensor
  *
  * @param sensor Pose sensor to use
  */
  void useSensor(SensorPtr<tf::StampedTransform> sensor);

protected:
  /**
  * @brief Extracts position and velocity data from UAV to compute appropriate
  * rpyt commands
  *
  * @param sensor_data Current position and velocity of UAV
  *
  * @return true if sensor data can be extracted
  */
  virtual bool
  extractSensorData(std::pair<double, QrotorBacksteppingState> &sensor_data);
  /**
  * @brief Send rpyt commands to hardware
  *
  * @param controls rpyt commands to send to UAV
  */
  virtual void sendControllerCommands(QrotorBacksteppingControl control);

private:
  /**
  * @brief Base class typedef to simplify code
  */
  using BaseClass = ControllerConnector<
      std::pair<double, QrotorBacksteppingState>,
      std::shared_ptr<ReferenceTrajectory<ParticleState, Snap>>,
      QrotorBacksteppingControl>;
  /**
  * @brief Quad hardware to send commands
  */
  parsernode::Parser &drone_hardware_;
  /**
  * @brief Estimator for finding the gain between joystick thrust command and
  * the acceleration in body z direction
  */
  ThrustGainEstimator &thrust_gain_estimator_;
  /**
  * @brief Controller config
  */
  QrotorBacksteppingControllerConfig config_;
  /**
  * @brief Time at start
  */
  std::chrono::time_point<std::chrono::high_resolution_clock> t_0_;
  /**
  * @brief Mass
  */
  const double m_;
  /**
  * @brief Gravity
  */
  const double g_;
  /**
  * @brief Time when the last sendControllerCommands is called
  */
  std::chrono::time_point<std::chrono::high_resolution_clock> previous_time_;
  /**
  * @brief Variable to store pose sensor for quad data
  */
  SensorPtr<tf::StampedTransform> pose_sensor_;
  /**
  * @brief Variable to integrate & store omega
  */
  Eigen::Vector3d current_omega_;
  /**
  * @brief Variable to store current RPY
  */
  Eigen::Vector3d current_rpy_;
  /**
  * @brief Variable to integrate & store current thrust
  */
  double thrust_;
  /**
  * @brief Variable to integrate & store current thrust_dot
  */
  double thrust_dot_;
  /**
  * @brief Variable to integrate & store omega command
  */
  Eigen::Vector3d omega_command_;
  /**
  * @brief Variable to integrate & store rpyt command message
  */
  geometry_msgs::Quaternion rpyt_message_;
  /**
  * @brief Variable to store lower bound on control
  */
  Eigen::Vector4d lb_;
  /**
  * @brief Variable to store upper bound on control
  */
  Eigen::Vector4d ub_;
};
