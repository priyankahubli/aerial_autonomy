#pragma once
/// States and actions corresponding to pick and place application
/// using visual servoing.
#include <aerial_autonomy/actions_guards/arm_functors.h>
#include <aerial_autonomy/actions_guards/arm_states_actions.h>
#include <aerial_autonomy/actions_guards/pick_place_functors.h>
#include <aerial_autonomy/actions_guards/visual_servoing_states_actions.h>
#include <aerial_autonomy/arm_events.h>
#include <aerial_autonomy/pick_place_events.h>
#include <boost/msm/front/euml/operator.hpp>
#include <boost/msm/front/functor_row.hpp>

/**
* @brief Class to provide typedefs for all basic uav states and actions
*
* @tparam LogicStateMachineT The backend of logic state machine
*/
template <class LogicStateMachineT>
struct PickPlaceStatesActions : VisualServoingStatesActions<LogicStateMachineT>,
                                ArmStatesActions<LogicStateMachineT> {
  /**
   * @brief Logical and functor between two guard functions
   *
   * @tparam G1 First guard functor
   * @tparam G2 Second guard functor
   */
  template <class G1, class G2>
  using bAnd = boost::msm::front::euml::And_<G1, G2>;

  /**
   * @brief namespace for states and actions in visual servoing
   */
  using vsa = VisualServoingStatesActions<LogicStateMachineT>;
  /**
   * @brief namespace for states and actions for basic uav actions
   */
  using usa = UAVStatesActions<LogicStateMachineT>;
  /**
   * @brief namespace for states and actions for arm functors
   */
  using asa = ArmStatesActions<LogicStateMachineT>;

  // Pre takeoff, land states
  /**
  * @brief State before takeoff
  */
  using ArmPreTakeoffFolding = ArmPreTakeoffFolding_<LogicStateMachineT>;
  /**
  * @brief State before landing
  */
  using ArmPreLandingFolding = ArmPreLandingFolding_<LogicStateMachineT>;
  // PickPlace State
  /**
  * @brief State while picking an object
  */
  using PickState = PickState_<LogicStateMachineT>;
  /**
  * @brief State while positioning the uav for placing
  */
  using PlaceState = PlaceState_<LogicStateMachineT>;
  /**
  * @brief State while positioning the uav for pre-placing
  */
  using PrePlaceState = PrePlaceState_<LogicStateMachineT>;
  /**
  * @brief State when reaching a relative pose visual servoing goal using rpyt
  * controller Uses reset instead of abort
  */
  using PrePickState = PrePickState_<LogicStateMachineT>;
  /**
  * @brief State when reaching a relative pose visual servoing goal
  *
  * Uses reset instead of abort
  */
  using RelativePoseVisualServoing = VisualServoing_<LogicStateMachineT, Reset>;
  // Transition Actions
  /**
  * @brief Action to take when starting rpyt relative pose visual servoing
  */
  template <int GoalIndex>
  using RelativePoseVisualServoingTransitionAction_ =
      RelativePoseVisualServoingTransitionActionFunctor_<LogicStateMachineT,
                                                         GoalIndex>;

  /**
  * @brief Action sequence that ungrips then goes home
  */
  using UngripGoHome = base_functors::bActionSequence<
      boost::mpl::vector<ArmGripActionFunctor_<LogicStateMachineT, false>,
                         typename vsa::GoHomeTransitionAction>>;
  /**
   * @brief Action sequence to ungrip object and go to home location and
   * fold the arm to right angle
   */
  using RightArmUngripGoHome = base_functors::bActionSequence<
      boost::mpl::vector<UngripGoHome, typename asa::ArmRightFold>>;

  /**
  * @brief Action sequence that ungrips then goes home
  */
  using ArmRightFoldGoHome = base_functors::bActionSequence<boost::mpl::vector<
      typename asa::AbortArmController, typename asa::ArmRightFold,
      typename vsa::GoHomeTransitionAction>>;

  /**
  * @brief Action sequence to abort UAV controller and move arm to right
  * angle
  */
  using AbortUAVControllerArmRightFold =
      base_functors::bActionSequence<boost::mpl::vector<
          typename usa::UAVControllerAbort, typename asa::ArmRightFold>>;
  /**
  * @brief Action sequence to abort UAV and arm controllers and move arm to
  * right angle
  */
  using AbortUAVArmControllerArmRightFold =
      base_functors::bActionSequence<boost::mpl::vector<
          typename usa::UAVControllerAbort, typename asa::AbortArmController,
          typename asa::ArmRightFold>>;
  /**
  * @brief Action sequence to abort UAV controller and move arm to right
  * angle
  */
  using AbortUAVControllerArmFold =
      base_functors::bActionSequence<boost::mpl::vector<
          typename usa::UAVControllerAbort, typename asa::ArmFold>>;

  /**
  * @brief Check tracking is valid and arm is enabled for pick
  */
  using PickTransitionGuard =
      bAnd<typename vsa::RelativePoseVisualServoingTransitionGuard,
           ArmTrackingGuardFunctor_<LogicStateMachineT>>;
  /**
   * @brief Check prepick goal index is specified
   */
  using PrePickTransitionGuard =
      bAnd<PickTransitionGuard, CheckGoalIndex_<LogicStateMachineT, 2>>;
  // \todo Check if tracked object is in workspace

  /**
  * @brief Move arm to pick pose and move to waypoint in front of object
  */
  using PrePickTransitionAction = base_functors::bActionSequence<
      boost::mpl::vector<ArmPoseTransitionActionFunctor_<LogicStateMachineT, 0>,
                         typename vsa::ResetRelativePoseVisualServoing,
                         RelativePoseVisualServoingTransitionActionFunctor_<
                             LogicStateMachineT, 2>>>;

  /**
  * @brief Move arm to pick pose and move to tracked object
  *
  * Do not set home since we are using this as an intermediate step
  * after pre-pick
  */
  using PickTransitionAction =
      base_functors::bActionSequence<boost::mpl::vector<
          ArmPoseTransitionActionFunctor_<LogicStateMachineT, 0>,
          typename vsa::ResetRelativePoseVisualServoing,
          RelativePoseVisualServoingTransitionActionFunctor_<LogicStateMachineT,
                                                             0, false>,
          SetNoisePolynomialReference_<LogicStateMachineT, true>>>;

  /**
  * @brief Action to take when starting placing object at either drop-off.
  */
  using PlaceVisualServoingTransitionAction =
      base_functors::bActionSequence<boost::mpl::vector<
          ArmPoseTransitionActionFunctor_<LogicStateMachineT, 0, false>,
          typename vsa::ResetRelativePoseVisualServoing,
          RelativePoseVisualServoingTransitionActionFunctor_<LogicStateMachineT,
                                                             1>>>;
  /**
  * @brief Action to take when starting placing object at either drop-off.
  */
  using PrePlaceVisualServoingTransitionAction = base_functors::bActionSequence<
      boost::mpl::vector<typename vsa::ResetRelativePoseVisualServoing,
                         RelativePoseVisualServoingTransitionActionFunctor_<
                             LogicStateMachineT, 3>>>;
  // \todo Matt add guard to check if relative pose visual servoing goal exists

  /**
  * @brief Guard to set and check that the id to track is available
  * before beginning visual servoing
  */
  using PrePlaceVisualServoingTransitionGuard =
      bAnd<EventIdVisualServoingGuardFunctor_<LogicStateMachineT>,
           CheckGoalIndex_<LogicStateMachineT, 3>>;

  using PlaceVisualServoingTransitionGuard =
      CheckGoalIndex_<LogicStateMachineT, 1>;

  /**
  * @brief Action sequence to abort UAV controller and arm controller
  * angle
  */
  using AbortUAVArmController =
      base_functors::bActionSequence<boost::mpl::vector<
          typename usa::UAVControllerAbort, typename asa::AbortArmController>>;
  /**
  * @brief State for following waypoints after picking object
  */
  using ReachingPostPickWaypoint =
      ReachingPostPickWaypoint_<LogicStateMachineT, 0, 0>;
  /**
  * @brief Base state for following waypoints after picking object.
  *  Used for setting state config
  */
  using ReachingPostPickWaypointBase =
      FollowingWaypointSequence_<LogicStateMachineT, 0, 0, ObjectId>;

  /**
   * @brief State to wait for picking
   */
  using WaitingForPick = WaitingForPick_<LogicStateMachineT>;

  /**
  * @brief State for following waypoints after placing object
  */
  using ReachingPostPlaceWaypoint =
      FollowingWaypointSequence_<LogicStateMachineT, 2, 3>;
  /**
  * @brief State for resetting visual servoing
  */
  struct ResetVisualServoing : ReachingGoal_<LogicStateMachineT> {};
};
