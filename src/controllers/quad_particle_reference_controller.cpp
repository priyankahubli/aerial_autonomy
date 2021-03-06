#include "aerial_autonomy/controllers/quad_particle_reference_controller.h"
#include "aerial_autonomy/common/conversions.h"
#include "aerial_autonomy/types/quad_particle_reference_trajectory.h"
#include <tf/tf.h>

QuadParticleReferenceController::QuadParticleReferenceController(
    ParticleReferenceConfig config)
    : config_(config) {}

bool QuadParticleReferenceController::runImplementation(
    std::pair<PositionYaw, tf::Transform> sensor_data, PositionYaw goal,
    ReferenceTrajectoryPtr<Eigen::VectorXd, Eigen::VectorXd> &control) {
  tf::Transform goal_tf;
  conversions::positionYawToTf(goal, goal_tf);
  tf::Transform desired_pose = sensor_data.second * goal_tf;
  double desired_yaw = tf::getYaw(desired_pose.getRotation());
  PositionYaw desired_pose_yaw(desired_pose.getOrigin().getX(),
                               desired_pose.getOrigin().getY(),
                               desired_pose.getOrigin().getZ(), desired_yaw);
  control.reset(
      new QuadParticleTrajectory(desired_pose_yaw, sensor_data.first, config_));
  return true;
}
