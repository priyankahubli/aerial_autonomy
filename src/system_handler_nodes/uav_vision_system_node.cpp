#include <aerial_autonomy/common/system_handler_node_utils.h>
#include <aerial_autonomy/state_machines/visual_servoing_state_machine.h>
#include <aerial_autonomy/system_handlers/uav_vision_system_handler.h>
#include <aerial_autonomy/visual_servoing_events.h>

/**
 * @brief Loads configutation file and starts system
 * @param argc Number of arguments
 * @param argv Arguments
 * @return Exit status
 */
int main(int argc, char **argv) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  google::InitGoogleLogging("aerial_autonomy_node");

  ros::init(argc, argv, "aerial_autonomy");
  ros::NodeHandle nh;
  createAndConfigureLogConfig(nh);
  BaseStateMachineConfig state_machine_config = createStateMachineConfig(nh);
  UAVSystemHandlerConfig uav_system_handler_config =
      createUAVSystemHandlerConfig(nh);

  UAVVisionSystemHandler<VisualServoingStateMachine,
                         visual_servoing_events::VisualServoingEventManager<
                             VisualServoingStateMachine>>
      uav_system_handler(uav_system_handler_config, state_machine_config);

  ros::spin();

  return 0;
}
