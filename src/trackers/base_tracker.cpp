#include "aerial_autonomy/trackers/base_tracker.h"

bool BaseTracker::getTrackingVector(tf::Transform &pose) {
  std::tuple<uint32_t, tf::Transform> pose_tuple;
  if (!getTrackingVector(pose_tuple)) {
    return false;
  }
  pose = std::get<1>(pose_tuple);
  return true;
}

bool BaseTracker::getTrackingVector(uint32_t tracked_id, tf::Transform &pose) {
  if (!trackingIsValid()) {
    return false;
  }
  std::unordered_map<uint32_t, tf::Transform> tracking_vectors;
  if (!getTrackingVectors(tracking_vectors)) {
    return false;
  }
  auto find_id = tracking_vectors.find(tracked_id);
  if (find_id != tracking_vectors.end()) {
    pose = find_id->second;
    return true;
  }
  return false;
}

bool BaseTracker::getTrackingVector(std::tuple<uint32_t, tf::Transform> &pose) {
  if (!trackingIsValid()) {
    return false;
  }
  std::unordered_map<uint32_t, tf::Transform> tracking_vectors;
  if (!getTrackingVectors(tracking_vectors)) {
    return false;
  }

  if (!tracking_strategy_->getTrackingVector(tracking_vectors, pose)) {
    return false;
  }

  return true;
}

bool BaseTracker::initialize() {
  std::unordered_map<uint32_t, tf::Transform> tracking_vectors;
  if (!getTrackingVectors(tracking_vectors)) {
    return false;
  }
  return tracking_strategy_->initialize(tracking_vectors);
}
