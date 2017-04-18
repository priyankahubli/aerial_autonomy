#pragma once

// Base robot system
#include <aerial_autonomy/robot_systems/base_robot_system.h>
// Arm hardware
#include <arm_parsers/generic_arm.h>
/// \todo Add controllers and controller connectors for visual servoing

/**
 * @brief Owns, initializes, and facilitates communication between different
 * hardware/software components.
 * Provides builtin set/get end effector pose, joint angles for a generic arm
*/
class ArmSystem : public BaseRobotSystem {

private:
  /**
  * @brief Hardware
  */
  GenericArm arm_hardware_;
  /// \todo Add controllers, controller connectors, config if needed

public:
  /**
  * @brief Constructor
  *
  * ArmSystem requires an arm hardware. It instantiates the connectors,
  * controllers
  *
  * @param arm_hardware input hardware to send commands back
  */
  ArmSystem(ros::NodeHandle &nh) : BaseRobotSystem(), arm_hardware_(nh) {}

  /**
  * @brief Public API call to get end effector transform
  */
  Eigen::Matrix4d getEndEffectorPose() {
    return arm_hardware_.getEndEffectorTransform();
  }

  /**
  * @brief Public API call to grip/ungrip an object
  *
  * @param grip_action true to grip an object and false to ungrip
  */
  void grip(bool grip_action) {
    if (grip_action) {
      //\todo Gowtham Change arm plugins to update gripping strategy
      int grip_position = 1000;
      arm_hardware_.grip(grip_position);
    } else {
      //\todo Gowtham Change arm plugins to update gripping strategy
      int grip_position = 2000;
      arm_hardware_.grip(grip_position);
    }
  }

  /**
  * @brief Power the arm on/off
  *
  * @param state True to switch on, and False to switch off.
  */
  void power(bool state) {
    if (state) {
      arm_hardware_.sendCmd("power on");
    } else {
      arm_hardware_.sendCmd("power off");
    }
  }

  /**
  * @brief Set the arm joints to a known folded configuration
  */
  void foldArm() { arm_hardware_.sendCmd("fold arm"); }

  /**
  * @brief Set the arm joints to a known L shaped configuration.
  */
  void rightArm() { arm_hardware_.sendCmd("right arm"); }

  /**
  * @brief Provide the current state of arm system
  * \todo Add status about arm powered on/off, and any error messages
  * can also add arm end effector pose if useful
  *
  * @return string representation of the arm system state
  */
  std::string getSystemStatus() const { return std::string(); }

  /**
  * @brief Verify the status of grip/power on/off commands
  *
  * @return True if the command is complete
  */
  bool getCommandStatus() const { return arm_hardware_.getCommandStatus(); }

  /**
  * @brief If arm is enabled return.
  *
  * @return True if arm enabled
  */
  bool enabled() {
    return arm_hardware_.state == ArmParser::ENABLED ? true : false;
  }
};