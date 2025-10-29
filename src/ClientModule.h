#ifndef CLIENTMODULE_H_
#define CLIENTMODULE_H_

#include <omnetpp.h>
#include "SMTPMessage_m.h"
#include "EncryptionHelper.h"

using namespace omnetpp;

/**
 * SMTP Client Module
 * Implements the client side of EMTP protocol with encryption and priority support
 */
class ClientModule : public cSimpleModule
{
private:
    // Client state
    enum ClientState
    {
        IDLE,
        WAIT_HELO_RESPONSE,
        WAIT_MAIL_FROM_RESPONSE,
        WAIT_RCPT_TO_RESPONSE,
        WAIT_DATA_RESPONSE,
        WAIT_QUIT_RESPONSE,
        DONE
    };

    ClientState state;
    int sequenceNumber;

    // Configuration parameters
    std::string clientName;
    std::string mailFrom;
    std::string mailTo;
    std::string subject;
    std::string messageBody;
    std::string priority;
    bool useEncryption;
    simtime_t startTime;

    // Encryption
    EncryptionHelper encryptionHelper;

    // Timing
    simtime_t sessionStartTime;

    // Statistics signals
    simsignal_t mailSentSignal;
    simsignal_t mailAcceptedSignal;
    simsignal_t mailRejectedSignal;
    simsignal_t roundTripTimeSignal;

    // Message sending methods
    void sendHelo();
    void sendMailFrom();
    void sendRcptTo();
    void sendData();
    void sendQuit();

    // Response handlers
    void handleHeloResponse(SMTPResponse *msg);
    void handleMailFromResponse(SMTPResponse *msg);
    void handleRcptToResponse(SMTPResponse *msg);
    void handleDataResponse(SMTPResponse *msg);
    void handleQuitResponse(SMTPResponse *msg);

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

public:
    ClientModule();
    virtual ~ClientModule();
};

Define_Module(ClientModule);

#endif /* CLIENTMODULE_H_ */
