#ifndef SERVERMODULE_H_
#define SERVERMODULE_H_

#include <omnetpp.h>
#include <queue>
#include <deque>
#include "SMTPMessage_m.h"
#include "EncryptionHelper.h"

using namespace omnetpp;

class ServerModule : public cSimpleModule
{
private:
    enum ServerState
    {
        WAITING,
        CONNECTED,
        MAIL_FROM_RECEIVED,
        RCPT_TO_RECEIVED,
        DATA_RECEIVED
    };

    ServerState state;

    std::string currentSender;
    std::string currentRecipient;
    int clientSequenceNumber;
    int currentClientGateIndex;

    std::string serverName;
    double processingDelay;
    double deliveryRate;
    int maxQueueSize;
    bool constrainedMode;
    bool enableFiltering;
    bool requireSubject;
    bool requireEncryption;

    EncryptionHelper encryptionHelper;

    std::deque<QueuedMail *> highPriorityQueue;
    std::deque<QueuedMail *> lowPriorityQueue;

    cMessage *deliveryTimer;
    int nextMailId;

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

    void handleHeloCommand(SMTPCommand *cmd);
    void handleMailFromCommand(SMTPCommand *cmd);
    void handleRcptToCommand(SMTPCommand *cmd);
    void handleDataCommand(MailData *mailData);
    void handleQuitCommand(SMTPCommand *cmd);

    bool filterMail(QueuedMail *mail, std::string &rejectReason);
    void enqueueMail(QueuedMail *mail);
    void processDeliveryQueue();
    void deliverMail(QueuedMail *mail);

    void sendResponse(int responseCode, const char *message, const char *extraData = nullptr);

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
