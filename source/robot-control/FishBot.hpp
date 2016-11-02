#ifndef CATS2_FISH_BOT_HPP
#define CATS2_FISH_BOT_HPP

#include "RobotControlPointerTypes.hpp"
#include "ControlModeStateMachine.hpp"
#include "control-modes/ControlTarget.hpp"
#include "MotionPatternType.hpp"
#include "Navigation.hpp"

#include <AgentState.hpp>

#include <QtCore/QObject>

class AgentDataWorld;
class StateWorld;

/*!
 * The main class for the FishBot robot. It stores the robot's state, the
 * current control mode and locomotion pattern, and also provides an interface
 * to communicate with the physical robot.
 */
class FishBot : public QObject
{
    Q_OBJECT
public:
    //! Constructor.
    explicit FishBot(QString id);
    //! Destructor.
    virtual ~FishBot() final;

    //! Return the robot's name.
    QString name() const { return m_name; }

    //! Sets the robot's interface.
    void setRobotInterface(Aseba::DBusInterfacePtr robotInterface);
    //! Returns the robot's interface.
    Aseba::DBusInterfacePtr robotInterface() { return m_robotInterface; }
    //! Inititialises the robot's firmware. The robot's index is used to
    //! initilize the robot's id in its firmware.
    void setupConnection(int robotIndex);

public:
    //! Returns the supported control modes.
    QList<ControlModeType::Enum> supportedControlModes();
    //! Sets the control mode.
    void setControlMode(ControlModeType::Enum type);
    //! Return the control mode.
    ControlModeType::Enum currentControlMode() const { return m_controlStateMachine.currentControlMode(); }

    //! Checks that the current control modes can generate targets with
    //! different motion patterns.
    bool supportsMotionPatterns();
    //! Sets the motion pattern.
    void setMotionPattern(MotionPatternType::Enum type);
    //! Return the motion pattern.
    MotionPatternType::Enum currentMotionPattern() const { return m_navigation.motionPattern(); }

    //! Steps the control for the robot.
    void stepControl();

public:
    //! The target position received from the viewer; it's transfered further
    //! to the state machine.
    void goToPosition(PositionMeters position);

public:
    //! Sets the robot's state.
    void setState(StateWorld state) {m_state = state; }
    //! Returns the robot's state.
    StateWorld state() const { return m_state; }

    //! Receives data of all tracked robots, finds and sets the one corresponding
    //! to this robot and keeps the rest in case it's needed by the control mode.
    void setRobotsData(QList<AgentDataWorld> robotsPositions);
    //! Received states of all tracked fish, keeps them in case it's needed by
    //! the control mode.
    void setFishStates(QList<StateWorld> fishStates);

signals:
    //! Informs that the robot's control mode was modified.
    void notifyControlModeChanged(ControlModeType::Enum type);
    //! Informs that the robot's motion pattern was changed.
    void notifyMotionPatternChanged(MotionPatternType::Enum type);

public:
    //! Distance between robot's wheels.
    static constexpr double InterWheelsDistanceCm = 1.8;

private:
    //! The robot's id.
    QString m_id;
    //! The robot's name.
    QString m_name;
    //! The robot's state.
    StateWorld m_state;
    //! The interface to communicate with the robot. Shared by all robots.
    Aseba::DBusInterfacePtr m_robotInterface;

    //! The control loop state machine that generates the targets for the navigation.
    ControlModeStateMachine m_controlStateMachine;

    //! The data of other robots.
    QList<AgentDataWorld> m_otherRobotsData;

    //! The states of fish.
    QList<StateWorld> m_fishStates;

    //! Navigates the robot to a target.
    Navigation m_navigation;

    // TODO : to add the interface with the RiBot lure
};

#endif // CATS2_FISH_BOT_HPP