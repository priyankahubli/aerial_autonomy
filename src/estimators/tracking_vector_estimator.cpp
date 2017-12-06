#include <aerial_autonomy/estimators/tracking_vector_estimator.h>
#include <aerial_autonomy/log/log.h>
#include <glog/logging.h>

TrackingVectorEstimator::TrackingVectorEstimator(
    TrackingVectorEstimatorConfig config,
    std::chrono::duration<double> propagation_step)
    : config_(config), filter_(6, 6, 0, CV_64F), zero_tolerance_(1e-6),
      initial_state_initialized_(false) {
  // Assuming x = [Marker direction, velocity]
  // Transition matrix
  filter_.transitionMatrix(cv::Rect(0, 0, 3, 3)) = cv::Mat_<double>::eye(3, 3);
  filter_.transitionMatrix(cv::Rect(3, 0, 3, 3)) =
      -propagation_step.count() * cv::Mat_<double>::eye(3, 3);
  filter_.transitionMatrix(cv::Rect(0, 3, 3, 3)) =
      cv::Mat_<double>::zeros(3, 3);
  filter_.transitionMatrix(cv::Rect(3, 3, 3, 3)) = cv::Mat_<double>::eye(3, 3);
  // Measurement matrix
  filter_.measurementMatrix = cv::Mat_<double>::eye(6, 6);
  // Check stdeviation vectors
  checkStdVector(config_.marker_process_stdev());
  checkStdVector(config_.velocity_process_stdev());
  checkStdVector(config_.marker_meas_stdev());
  checkStdVector(config_.velocity_meas_stdev());
  checkStdVector(config_.marker_initial_stdev());
  checkStdVector(config_.velocity_initial_stdev());
  // Noise matrices
  setCovarianceMatrix(filter_.processNoiseCov, config_.marker_process_stdev(),
                      config_.velocity_process_stdev());
  setCovarianceMatrix(filter_.measurementNoiseCov, config_.marker_meas_stdev(),
                      config_.velocity_meas_stdev());
  // Set initial state
  initializeState(tf::Vector3(0, 0, 0), tf::Vector3(0, 0, 0));
  DATA_HEADER("tracking_vector_estimator") << "Measured_Marker_x"
                                           << "Measured_Marker_y"
                                           << "Measured_Marker_z"
                                           << "Measured_Velocity_x"
                                           << "Measured_Velocity_y"
                                           << "Measured_Velocity_z"
                                           << "Marker_x"
                                           << "Marker_y"
                                           << "Marker_z"
                                           << "Velocity_x"
                                           << "Velocity_y"
                                           << "Velocity_z"
                                           << "Noise_x"
                                           << "Noise_y"
                                           << "Noise_z"
                                           << "Noise_vx"
                                           << "Noise_vy"
                                           << "Noise_vz" << DataStream::endl;
}

void TrackingVectorEstimator::initializeState(tf::Vector3 marker_direction,
                                              tf::Vector3 velocity) {
  filter_.statePre =
      (cv::Mat_<double>(6, 1) << marker_direction.x(), marker_direction.y(),
       marker_direction.z(), velocity.x(), velocity.y(), velocity.z());
  setCovarianceMatrix(filter_.errorCovPost, config_.marker_initial_stdev(),
                      config_.velocity_initial_stdev());
  filter_.statePre.copyTo(filter_.statePost);
  initial_state_initialized_ = true;
}

void TrackingVectorEstimator::estimate(tf::Vector3 marker_direction,
                                       tf::Vector3 velocity) {
  if (!initial_state_initialized_) {
    initializeState(marker_direction, velocity);
    return;
  }
  cv::Mat_<double> measurement =
      (cv::Mat_<double>(6, 1) << marker_direction.x(), marker_direction.y(),
       marker_direction.z(), velocity.x(), velocity.y(), velocity.z());
  filter_.predict();
  filter_.correct(measurement);
  // Log data
  tf::Vector3 marker_noise = getMarkerNoise();
  tf::Vector3 velocity_noise = getVelocityNoise();
  auto state = filter_.statePost;
  auto &data_stream = Log::instance()["tracking_vector_estimator"];
  data_stream << DataStream::startl;
  for (int i = 0; i < 6; ++i) {
    data_stream << measurement.at<double>(i);
  }
  for (int i = 0; i < 6; ++i) {
    data_stream << state.at<double>(i);
  }
  for (int i = 0; i < 3; ++i) {
    data_stream << marker_noise[i];
  }
  for (int i = 0; i < 3; ++i) {
    data_stream << velocity_noise[i];
  }
  data_stream << DataStream::endl;
}