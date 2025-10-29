#ifndef SERVERMODULE_H_
#define SERVERMODULE_H_

#include <omnetpp.h>
#include <queue>
#include <deque>
#include "SMTPMessage_m.h"
#include "EncryptionHelper.h"

using namespace omnetpp;

/**
 * SMTP Server Module
 * Implements server side with priority queues, encryption, and filtering
 */
class ServerModule : public cSimpleModule
{
private:
    // Server state
    enum ServerState
    {
        WAITING,
        CONNECTED,
        MAIL_FROM_RECEIVED,
        RCPT_TO_RECEIVED,
        DATA_RECEIVED
    };

    ServerState state;

    // Current session data
    std::string currentSender;
    std::string currentRecipient;
    int clientSequenceNumber;
    int currentClientGateIndex; // Track which client gate we're communicating with

    // Configuration
    std::string serverName;
    double processingDelay;
    double deliveryRate;
    int maxQueueSize;
    bool constrainedMode;
    bool enableFiltering;
    bool requireSubject;
    bool requireEncryption;

    // Encryption
    EncryptionHelper encryptionHelper;

    // Priority queues
    std::deque<QueuedMail *> highPriorityQueue;
    std::deque<QueuedMail *> lowPriorityQueue;

    // Queue management
    cMessage *deliveryTimer;
    int nextMailId;

    // Statistics signals
    simsignal_t mailReceivedSignal;
    simsignal_t mailAcceptedSignal;
    simsignal_t mailRejectedSignal;
    simsignal_t mailDeliveredSignal;
    simsignal_t highPriorityDeliveredSignal;
    simsignal_t lowPriorityDeliveredSignal;
    simsignal_t queueSizeSignal;
    simsignal_t highPriorityQueueSizeSignal;
    simsignal_t lowPriorityQueueSizeSignal;
    simsignal_t deliveryLatencySignal;
    simsignal_t highPriorityLatencySignal;
    simsignal_t lowPriorityLatencySignal;

    // Command handlers
    void handleHeloCommand(SMTPCommand *cmd);
    void handleMailFromCommand(SMTPCommand *cmd);
    void handleRcptToCommand(SMTPCommand *cmd);
    void handleDataCommand(MailData *mailData);
    void handleQuitCommand(SMTPCommand *cmd);

    // Mail processing
    bool filterMail(QueuedMail *mail, std::string &rejectReason);
    void enqueueMail(QueuedMail *mail);
    void processDeliveryQueue();
    void deliverMail(QueuedMail *mail);

    // Response sending
    void sendResponse(int responseCode, const char *message, const char *extraData = nullptr);

    // Queue statistics
    void updateQueueStatistics();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

public:
    ServerModule();
    virtual ~ServerModule();
};

Define_Module(ServerModule);

#endif /* SERVERMODULE_H_ */
