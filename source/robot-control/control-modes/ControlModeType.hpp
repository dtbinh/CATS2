#ifndef CATS2_CONTROL_MODE_TYPE_HPP
#define CATS2_CONTROL_MODE_TYPE_HPP

#include <QtCore/QString>

/*!
 * Provides an enum for control mode type.
 */
class ControlModeType
{
public:
    enum Enum {
        IDLE,
        MANUAL,
        GO_TO_POSITION,
        GO_STRAIGHT,
        MODEL_BASED,
        TRAJECTORY,
        UNDEFINED
    };

    //! Gets the type of the control mode from the settings' string.
    static Enum fromSettingsString(QString modeName)
    {
        if (modeName.toLower() == "idle")
            return IDLE;
        else if (modeName.toLower() == "manual")
            return MANUAL;
        else if (modeName.toLower() == "gotoposition")
            return GO_TO_POSITION;
        else if (modeName.toLower() == "gostraight")
            return GO_STRAIGHT;
        else if (modeName.toLower() == "modelbased")
            return MODEL_BASED;
        else if (modeName.toLower() == "trajectory")
            return TRAJECTORY;
        else
            return UNDEFINED;
    }

    //! Returns that control mode type string in human friendly format.
    static QString toString(Enum controlModeType) {
        QString string;

        switch (controlModeType) {
        case MANUAL:
            string = "Manual";
            break;
        case GO_TO_POSITION:
            string = "Go to position";
            break;
        case GO_STRAIGHT:
            string = "Go straight";
            break;
        case MODEL_BASED:
            string = "Model based";
            break;
        case TRAJECTORY:
            string = "Trajectory";
            break;
        case IDLE:
        default:
            string = "Idle";
            break;
        }
        return string;
    }
};

#endif // CATS2_CONTROL_MODE_TYPE_HPP

