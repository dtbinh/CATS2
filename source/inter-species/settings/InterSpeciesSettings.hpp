#ifndef CATS2_INTER_SPECIES_SETTINGS_HPP
#define CATS2_INTER_SPECIES_SETTINGS_HPP

#include <QtCore/QString>

/*!
 * Class-signleton that is used to store parameters of the inter-species communication.
 * Their values are loaded from the configuration file.
 * NOTE : All the settings are made as singltons to simplify the access to them;
 * the drawback is that initialization of many objects becomes obscure because of this.
 * NOTE : All the settings must be initialized on the program startup.
*/
class InterSpeciesSettings
{
public:
    //! The singleton getter.
    static InterSpeciesSettings& get();

    //! Initializes the parameters from the configuration file.
    bool init(QString configurationFileName,
              bool needPublisherAddress = false,
              bool needSubscriberAddress = false);

    // delete copy and move constructors and assign operators
    //! Copy constructor.
    InterSpeciesSettings(InterSpeciesSettings const&) = delete;
    //! Move constructor.
    InterSpeciesSettings(InterSpeciesSettings&&) = delete;
    //! Copy assignment.
    InterSpeciesSettings& operator=(InterSpeciesSettings const&) = delete;
    //! Move assignment.
    InterSpeciesSettings& operator=(InterSpeciesSettings &&) = delete;

public:
    //! Returns the address for the publisher.
    QString publisherAddress() const { return m_publisherAddress; }
    //! Returns the address for the subscriber.
    QString subscriberAddress() const { return m_subscriberAddress; }

private:
    //! Constructor. Defining it here prevents construction.
    InterSpeciesSettings() {}
    //! Destructor. Defining it here prevents unwanted destruction.
    ~InterSpeciesSettings() {}

private:
    //! Stores the address to publish the data.
    QString m_publisherAddress;
    //! Stores the address to receive the data.
    QString m_subscriberAddress;
};


#endif // CATS2_INTER_SPECIES_SETTINGS_HPP
