#include "InitiationController.hpp"

#include "FishBot.hpp"
#include "ControlArea.hpp"

#include <QtCore/QDebug>

/*!
 * Constructor.
 */
InitiationController::InitiationController(FishBot* robot,
                             ExperimentControllerSettingsPtr settings) :
    ExperimentController(robot, ExperimentControllerType::INITIATION),
    m_settings(),
    m_state(UNDEFINED),
    m_limitModelArea(false),
    m_inTargetRoom(false),
    m_targetAreaId(""),
    m_departureAreaId("")
{
    // NOTE : to get parameters specific for this controller we need to convert
    // the settings to the corresponding format
    InitiationControllerSettings* initiationControllerSettings =
            dynamic_cast<InitiationControllerSettings*>(settings.data());
    if (initiationControllerSettings != nullptr){
        // copy the parameters
        m_settings = initiationControllerSettings->data();
    } else {
        qDebug() << Q_FUNC_INFO << "Could not set the controller's settings";
    }

    // load the control map
    readControlMap(m_settings.controlAreasFileName());
}

/*!
 * Returns the control values for given position.
 */
ExperimentController::ControlData InitiationController::step()
{
    ControlData controlData;

    // a check for the valid robot pointer
    if (m_robot) {
        // check where the robot and fish are
        updateAreasOccupation();

        if (needToChangeRoom()) {
            controlData = changeRoom();
        } else {
            // swimming with fish
            updateState(SWIMMING_WITH_FISH);
            controlData = stateControlData();
        }
    }

    return controlData;
}

/*!
 * Called when the controller is activated. Used to reset parameters.
 */
void InitiationController::start()
{
    updateState(SWIMMING_WITH_FISH);
    m_departureTimer.clear();
    m_fishFollowCheckTimer.clear();
    m_inTargetRoom = false;
    m_targetAreaId = "";
    m_departureAreaId = "";
}

/*!
 * Updates the current state.
 */
void InitiationController::updateState(State state)
{
    if (m_state != state) {
        qDebug() << Q_FUNC_INFO
                 << QString("%1 state changed from %2 to %3")
                    .arg(m_robot->name())
                    .arg(stateToString(m_state))
                    .arg(stateToString(state));
        m_state = state;
        emit notifyControllerStatus(stateToString(m_state).toLower().replace("-", " "));
        // a specific check for the case when the robot is switching to the
        // model-based control mode
        if (m_state == SWIMMING_WITH_FISH)
            m_limitModelArea = true;
    }
}

/*!
 * Checks if the conditions are met to start the inititaion procedure.
 */
bool InitiationController::needToChangeRoom()
{
    // if the robot is changing the room right now
    if (changingRoom())
        return true; // continue

    // if it's the room with the majority of the fish?
    if ((m_fishNumberByArea[m_robotAreaId] >=
         (RobotControlSettings::get().numberOfAnimals() - m_settings.maximalFishNumberAllowedToStay()))
            && m_controlAreas.contains(m_robotAreaId)
            && (m_controlAreas[m_robotAreaId]->type() == ControlAreaType::ROOM)) {
        // if there is a preference for the room?
        if (m_controlAreas.contains(m_preferedAreaId)) {
            // if the robot is in the prefered room?
            if (m_robotAreaId == m_preferedAreaId) {
                // the robot is already in the prefered room and most of fish
                // are with it as well => do nothing
                return false;
            } else {
                // wait until the departure timer's signal
                return timeToDepart();
            }
        } else {
            // no preference for a room, so we can
            // wait until the departure timer's signal
            return timeToDepart();
        }
    } else {
        // do nothing, wait until the fish arrive
        return false;
    }
}

/*!
 * Runs the initiation state machine.
 */
ExperimentController::ControlData InitiationController::changeRoom()
{
    switch (m_state) {
    case SWIMMING_WITH_FISH:
        // find the room to go (target room) as another area of the type ROOM
        m_targetAreaId = "";
        m_departureAreaId = "";
        for (const auto& area : m_controlAreas.values()) {
            if ((area->id() != m_robotAreaId)
                    && (area->type() == ControlAreaType::ROOM)) {
                m_targetAreaId = area->id();
            }
        }
        // if managed to find where to go
        if (m_controlAreas.contains(m_targetAreaId)) {
            // save the original room
            m_departureAreaId = m_robotAreaId;
            // change the state to CHANGING_ROOM
            qDebug() << Q_FUNC_INFO << QString("%1 changes the room to %2 (%3)")
                        .arg(m_robot->name())
                        .arg(m_targetAreaId)
                        .arg(m_controlAreas[m_targetAreaId]->centroid().toString());
            updateState(CHANGING_ROOM);
            // we are not in the target room
            m_inTargetRoom = false;
            // start the check-that-fish-follow timer
            m_fishFollowCheckTimer.reset();
        } else {
            // couldn't decide where to go, continue doing fish
        }
        break;
    case CHANGING_ROOM:
        // send the status on the timer
        if (m_fishFollowCheckTimer.isSet()) {
            QString stateString = stateToString(m_state).toLower().replace("-", " ");
            double timeLeftSec = m_settings.fishFollowCheckTimeOutSec() -
                    m_fishFollowCheckTimer.runTimeSec();
            emit notifyControllerStatus(QString("%1 (%2s)")
                                        .arg(stateString)
                                        .arg(timeLeftSec, 0, 'f', 1));
        } else {
            QString stateString = stateToString(m_state).toLower().replace("-", " ");
            emit notifyControllerStatus(QString("%1 (follow)").arg(stateString));
        }
        // if we just arrived to the target room then we check right away that
        // the fish follow, if it's the case we swith to the model-based mode
        if (!m_inTargetRoom &&
                m_robot->state().position().isValid() &&
                m_robot->state().position()
                .closeTo(m_controlAreas[m_targetAreaId]->centroid()))
        {
            m_inTargetRoom = true;
            if (fishFollow()) {
                // fish follow, switch to the model-based control mode
                m_fishFollowCheckTimer.clear();
                qDebug() << Q_FUNC_INFO
                         << QString("Fish seems to follow %1, stay in the room")
                            .arg(m_robot->name());
                // switch to SWIMMING_WITH_FISH
                updateState(SWIMMING_WITH_FISH);
            }
        } else { // i.e. either we are still going to the target room or we are
            // there but the fish didn't follow right away, in any case we need
            // to check the fish-follow timer
            // first we check that the timer to check that fish follow is ticking
            if (m_fishFollowCheckTimer.isSet()) {
                // check the check-that-fish-follow timer
                if (m_fishFollowCheckTimer.isTimedOutSec(m_settings.fishFollowCheckTimeOutSec())) {
                    //if the fish follow the robot
                    if (fishFollow()) {
                        // fish follow, continue transition
                        m_fishFollowCheckTimer.clear();
                        qDebug() << Q_FUNC_INFO
                                 << QString("Fish seems to follow %1, continue moving")
                                        .arg(m_robot->name());
                    } else {
                        //  fish don't follow, switch to GOING_BACK mode
                        qDebug() << Q_FUNC_INFO
                                 << QString("Fish don't follow %1, returning back to %2")
                                        .arg(m_robot->name())
                                        .arg(m_departureAreaId);
                        updateState(GOING_BACK);
                    }
                } else {
                    // it's too early to check if fish follow, continue transition
                }
            } else {
                // the timer is off meaning that the fish follow the robot
                // if arrived to to target
                if (m_robot->state().position().closeTo(m_controlAreas[m_targetAreaId]->centroid()))
                {
                    qDebug() << Q_FUNC_INFO << QString("%1 arrived to the room %2")
                                .arg(m_robot->name())
                                .arg(m_targetAreaId);
                    // switch to SWIMMING_WITH_FISH
                    updateState(SWIMMING_WITH_FISH);
                }
            }
        }
        break;
    case GOING_BACK:
        // if returned to the original room
        if (m_robot->state().position().closeTo(m_controlAreas[m_departureAreaId]->centroid()))
        {
            qDebug() << Q_FUNC_INFO << QString("%1 returned to the departure room %2")
                        .arg(m_robot->name())
                        .arg(m_departureAreaId);
            // switch to SWIMMING_WITH_FISH
            updateState(SWIMMING_WITH_FISH);
        }
        break;
    default:
        break;
    }

    return stateControlData();
}

/*!
 * Checks the departure time.
 */
bool InitiationController::timeToDepart()
{
    if (m_settings.departureOnTimeOut()) {
        // timer started
        if (m_departureTimer.isSet()) {
            // if time out?
            if (m_departureTimer.isTimedOutSec(m_settings.departureTimeOutSec())) {
                qDebug() << Q_FUNC_INFO << "Changing the room on timeout";
                // change the room
                m_departureTimer.clear();
                return true;
            } else {
                // need to wait more => do nothing
                return false;
            }
        } else {
            // start the timer
            m_departureTimer.reset();
            return false;
        }
    } else if (m_settings.departureWhenInGroup()) {
        int fishAroundRobot = 0;
        PositionMeters robotPosition = m_robot->state().position();
        if (robotPosition.isValid()) {
            for (auto const& fishState : m_robot->fishStates()) {
                if (fishState.position().isValid() &&
                        (fishState.position().distance2DTo(robotPosition)
                         < m_settings.groupRadius())) {
                    fishAroundRobot++;
                }
            }
            // if enough fish are close to the robot
            if (fishAroundRobot >= m_settings.fishNumberAroundOnDeparture()) {
                qDebug() << Q_FUNC_INFO
                         << QString("Detected %1 fish, changing the room")
                            .arg(fishAroundRobot);
                return true;
            } else {
//                qDebug() << Q_FUNC_INFO << QString("Detected %1 fish").arg(fishAroundRobot);
                return false;
            }
        } else
            return false;
    }
    return false;
}

/*!
 * Checks that the fish follow.
 */
bool InitiationController::fishFollow()
{
    bool fishDoNotFollow = ((m_fishNumberByArea.contains(m_departureAreaId)) &&
                            (m_fishNumberByArea[m_departureAreaId] >
                             m_settings.maximalFishNumberAllowedToStay()));
    return !fishDoNotFollow;
}

/*!
 * Gets the control data that corresponds to the current state.
 */
ExperimentController::ControlData InitiationController::stateControlData()
{
    ControlData controlData;

    switch (m_state) {
    case SWIMMING_WITH_FISH:
        controlData.controlMode = ControlModeType::MODEL_BASED;
        controlData.motionPattern = MotionPatternType::FISH_MOTION;
        // if we need to limit the model
        if (m_limitModelArea &&
                m_controlAreas.contains(m_robotAreaId) &&
                m_controlAreas[m_robotAreaId]->type() == ControlAreaType::ROOM)
        {
            m_limitModelArea = false;
            controlData.data =
                QVariant::fromValue(m_controlAreas[m_robotAreaId]->annotatedPolygons());
        }
        break;
    case CHANGING_ROOM:
        if (m_controlAreas.contains(m_targetAreaId)) {
            controlData.controlMode = ControlModeType::GO_TO_POSITION;
            controlData.motionPattern = MotionPatternType::PID;
            controlData.data =
                QVariant::fromValue(m_controlAreas[m_targetAreaId]->centroid());
        }
        break;
    case GOING_BACK:
        if (m_controlAreas.contains(m_departureAreaId)) {
            controlData.controlMode = ControlModeType::GO_TO_POSITION;
            controlData.motionPattern = MotionPatternType::PID;
            controlData.data =
                QVariant::fromValue(m_controlAreas[m_departureAreaId]->centroid());
        }
        break;
    default:
        break;
    }

    return controlData;
}

/*!
 * Converts the state to the string for the output.
 */
QString InitiationController::stateToString(State state)
{
    switch (state) {
    case SWIMMING_WITH_FISH:
        return "Swimming-with-fish";
        break;
    case CHANGING_ROOM:
        return "Changing-room";
        break;
    case GOING_BACK:
        return "Going-back";
        break;
    case UNDEFINED:
    default:
        return "Undefined";
        break;
    }
}
