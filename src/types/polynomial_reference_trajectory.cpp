#include "aerial_autonomy/types/polynomial_reference_trajectory.h"
#include "aerial_autonomy/common/conversions.h"
#include "aerial_autonomy/common/math.h"
#include <glog/logging.h>

constexpr double PolynomialReferenceTrajectory::gravity_magnitude_;

PolynomialReferenceTrajectory::PolynomialReferenceTrajectory(
    PositionYaw goal_state, PositionYaw start_state,
    PolynomialReferenceConfig config)
    : degree_(9), dimensions_(4), goal_state_(goal_state),
      start_state_(dimensions_), config_(config) {
  PositionYaw error = goal_state - start_state;
  start_state_ << start_state.x, start_state.y, start_state.z, start_state.yaw;
  Eigen::MatrixXd constraints(degree_ + 1, dimensions_);
  Eigen::MatrixXd basis(degree_ + 1, degree_ + 1);
  constraints.setZero();
  constraints(0, 0) = error.x;
  constraints(0, 1) = error.y;
  constraints(0, 2) = error.z;
  constraints(0, 3) = error.yaw;
  CHECK_GT(config_.tf(), 1e-2) << "Final time should be greater than 1e-2";
  basis.topRows(dimensions_ + 1) =
      findBasisMatrix(config_.tf(), degree_, dimensions_);
  basis.bottomRows(dimensions_ + 1) =
      findBasisMatrix(0.0, degree_, dimensions_);
  coefficients_ = basis.colPivHouseholderQr().solve(constraints);
}

Eigen::MatrixXd
PolynomialReferenceTrajectory::findBasisMatrix(double t, int degree,
                                               int dimensions) const {
  Eigen::MatrixXd basis(dimensions + 1, degree + 1);
  Eigen::VectorXd coeff(degree + 1);
  coeff.setOnes();
  Eigen::VectorXd t_exp(degree + 1);
  t_exp(0) = 1;
  for (int i = 1; i < degree + 1; ++i) {
    t_exp(i) = t_exp(i - 1) * t;
  }
  for (int row = 0; row < dimensions + 1; ++row) {
    for (int col = 0; col < degree + 1; ++col) {
      int col_row_diff = col - row;
      if (row >= 1 && col_row_diff >= 0) {
        coeff(col) = coeff(col) * (col_row_diff + 1);
      }
      if (col_row_diff >= 0) {
        basis(row, col) = t_exp(col_row_diff) * coeff(col);
      } else {
        basis(row, col) = 0;
      }
    }
  }
  return basis;
}

std::pair<Eigen::VectorXd, Eigen::VectorXd>
PolynomialReferenceTrajectory::atTime(double t) const {
  Eigen::VectorXd state(15);
  Eigen::VectorXd control(4);
  double t_clamped = math::clamp(t, 0, config_.tf());
  Eigen::MatrixXd basis = findBasisMatrix(t_clamped, degree_, dimensions_);
  Eigen::MatrixXd out = basis * coefficients_;
  Eigen::VectorXd position_yaw = start_state_ + out.row(0).transpose();
  Eigen::VectorXd velocity_yawrate = out.row(1);
  Eigen::VectorXd acceleration_yaw = out.row(2);
  Eigen::Vector3d acceleration = acceleration_yaw.segment(0, 3);
  // wrap yaw
  position_yaw(3) = math::angleWrap(position_yaw(3));
  // Compensate gravity
  acceleration[2] = acceleration[2] + gravity_magnitude_;
  // Get rp from acceleration
  auto roll_pitch =
      conversions::accelerationToRollPitch(position_yaw(3), acceleration);
  // Fill state
  state.segment(0, 3) = position_yaw.segment<3>(0);     // pos
  state.segment(6, 3) = velocity_yawrate.segment<3>(0); // vel
  ///\todo fill rp_rate correctly
  state.segment(9, 2).setZero();   // rp_rate
  state(11) = velocity_yawrate(3); // yaw_rate
  // rpy, rpy_cmd
  state(12) = state(3) = roll_pitch.first;
  state(13) = state(4) = roll_pitch.second;
  state(14) = state(5) = position_yaw(3);
  // Fill control
  control(0) = acceleration.norm() / gravity_magnitude_;
  control(1) = control(2) = 0; // rp_rate \\\todo fill correctly
  control(3) = velocity_yawrate(3);
  return std::make_pair(state, control);
}

Eigen::VectorXd PolynomialReferenceTrajectory::goal(double) {
  Eigen::VectorXd goal_state(15);
  goal_state.setZero();
  goal_state[0] = goal_state_.x;
  goal_state[1] = goal_state_.y;
  goal_state[2] = goal_state_.z;
  goal_state[5] = goal_state_.yaw;
  goal_state[14] = goal_state_.yaw;
  return goal_state;
}