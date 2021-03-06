#include "Settings.hpp"

#include <settings/CommandLineParameters.hpp>
#include <settings/CalibrationSettings.hpp>
#include <settings/TrackingSetupSettings.hpp>
#include <settings/GrabberSettings.hpp>
#include <settings/ViewerSettings.hpp>
#include <settings/InterSpeciesSettings.hpp>
#include <settings/RobotControlSettings.hpp>
#include <RunTimer.hpp>

#include <QtCore/QDebug>

/*!
 * The singleton getter. Provides an instance of the settings.
 */
Settings& Settings::get()
{
    static Settings instance;
    return instance;
}

/*!
 * Initializes all the necessary parameters.
 */
bool Settings::init(int argc, char *argv[],
                    bool needConfigFile, bool needCalibration,
                    bool needMainCamera, bool needBelowCamera,
                    bool needPublisherAddress, bool needRobots)
{
    // initialize the run timer class
    RunTimer::get().init();

    // first check the command line parameters
    if (! CommandLineParameters::get().init(argc, argv, needConfigFile, needMainCamera, needBelowCamera)) {
        qDebug() << "Couldn't find necessary input arguments, finished";
        return false;
    }

    // check that when the main camera is available it is correctly set
    if (isAvailable(SetupType::MAIN_CAMERA)) {
        if (!CommandLineParameters::get().cameraDescriptor(SetupType::MAIN_CAMERA).isValid()) {
            qDebug() << "Main camera is not correctly set";
            return false;
        }
        if (!TrackingSetupSettings::init(CommandLineParameters::get().configurationFilePath(),
                                         SetupType::MAIN_CAMERA, needCalibration)) {
            qDebug() << "Could not initialize the main camera setup";
            return false;
        }
        if (!ViewerSettings::get().init(CommandLineParameters::get().configurationFilePath(),
                                        SetupType::MAIN_CAMERA)){
            qDebug() << "Could not initialize the main camera viewer";
            return false;
        }
    }

    // check that when the camera below is available it is correctly set
    if (isAvailable(SetupType::CAMERA_BELOW)) {
        if (!CommandLineParameters::get().cameraDescriptor(SetupType::CAMERA_BELOW).isValid()) {
            qDebug() << "The camera below is not correctly set";
            return false;
        }
        if (!TrackingSetupSettings::init(CommandLineParameters::get().configurationFilePath(),
                                         SetupType::CAMERA_BELOW, needCalibration)) {
            qDebug() << "Could not initialize the camera below setup";
            return false;
        }
        if (!ViewerSettings::get().init(CommandLineParameters::get().configurationFilePath(), SetupType::CAMERA_BELOW)){
            qDebug() << "Could not initialize the main camera viewer";
            return false;
        }
    }

    // initialize the inter-species module settings
    if (!InterSpeciesSettings::get().init(CommandLineParameters::get().configurationFilePath(), needPublisherAddress)) {
        qDebug() << "Could not initialize the inter-species module";
        return false;
    }

    // initialize the robot controller settings
    if ((!RobotControlSettings::get().init(CommandLineParameters::get().configurationFilePath())) && needRobots) {
        qDebug() << "Could not initialize the robot controllers";
        return false;
    }

    return true;
}

/*!
 * Returns the reference to the grabber settings.
 */
GrabberSettings& Settings::grabberSettings()
{
    return GrabberSettings::get();
}

/*!
 * Checks if the setup is set as available.
 */
bool Settings::isAvailable(SetupType::Enum setupType)
{
    return CommandLineParameters::get().availableSetups().contains(setupType);
}
