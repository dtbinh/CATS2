#ifndef CATS2_CONTROL_MODE_HPP
#define CATS2_CONTROL_MODE_HPP

#include "ControlTarget.hpp"
#include "ControlModeType.hpp"
#include "RobotControlPointerTypes.hpp"

#include <QtCore/QObject>
#include <QtCore/QSharedPointer>

class FishBot;
enum class ControlTargetType;

/*!
 * The parent class for the control mode.
 */
class ControlMode : public QObject
{
    Q_OBJECT
public:
    //! Constructor. It gets a pointer to the robot that is controlled by this
    //! control mode. The robot's state is used by those control modes that
    //! need to know it's current position and orientation.
    explicit ControlMode(FishBot* robot, ControlModeType::Enum type);
    //! Destructor.
    virtual ~ControlMode();

    //! Called when the control mode is activated. Used to reset mode's parameters.
    virtual void start() {}
    //! The step of the control mode, generates the target values.
    virtual ControlTargetPtr step() { return ControlTargetPtr(); }
    //! Called when the control mode is disactivated. Makes a cleanup if necessary.
    virtual void finish() {}

    //! Informs on what kind of control targets this control mode generates.
    virtual QList<ControlTargetType> supportedTargets();

signals:
    //! Requests to change the control mode.
    void requestControlModeChange(ControlModeType::Enum desiredControlMode);
    //! Sends out the current control mode status.
    void notifyControlModeStatus(QString status);

protected:
    //! A pointer to the robot that is controlled by this control mode.
    FishBot* m_robot;
    //! A type of the control mode.
    ControlModeType::Enum m_type;
};

#endif // CATS2_CONTROL_MODE_HPP
