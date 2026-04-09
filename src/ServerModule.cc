#include "ServerModule.h"

ServerModule::ServerModule()
{
    state = WAITING;
    deliveryTimer = nullptr;
    nextMailId = 1;
    clientSequenceNumber = 0;
    currentClientGateIndex = -1;
}

ServerModule::~ServerModule()
{
    cancelAndDelete(deliveryTimer);

    for (auto mail : highPriorityQueue)
    {
        delete mail;
    }
    for (auto mail : lowPriorityQueue)
    {
        delete mail;
    }
}

void ServerModule::initialize()
{
    serverName = par("serverName").stdstringValue();
    processingDelay = par("processingDelay");
    deliveryRate = par("deliveryRate");
    maxQueueSize = par("maxQueueSize");
    constrainedMode = par("constrainedMode");
    enableFiltering = par("enableFiltering");
    requireSubject = par("requireSubject");
    requireEncryption = par("requireEncryption");

    int seed = par("encryptionSeed");
    encryptionHelper.generateKeyPair(seed);

    EV << "Server public key: " << encryptionHelper.getPublicKeyString() << endl;

    mailReceivedSignal = registerSignal("mailReceived");
    mailAcceptedSignal = registerSignal("mailAccepted");
    mailRejectedSignal = registerSignal("mailRejected");
    mailDeliveredSignal = registerSignal("mailDelivered");
    highPriorityDeliveredSignal = registerSignal("highPriorityDelivered");
    lowPriorityDeliveredSignal = registerSignal("lowPriorityDelivered");
    queueSizeSignal = registerSignal("queueSize");
    highPriorityQueueSizeSignal = registerSignal("highPriorityQueueSize");
    lowPriorityQueueSizeSignal = registerSignal("lowPriorityQueueSize");
    deliveryLatencySignal = registerSignal("deliveryLatency");
    highPriorityLatencySignal = registerSignal("highPriorityLatency");
    lowPriorityLatencySignal = registerSignal("lowPriorityLatency");

    deliveryTimer = new cMessage("deliveryTimer");
    scheduleAt(simTime() + deliveryRate, deliveryTimer);

    state = WAITING;
    EV << "Server initialized: " << serverName << endl;
}

void ServerModule::handleMessage(cMessage *msg)
{
    if (msg == deliveryTimer)
    {
        processDeliveryQueue();
        scheduleAt(simTime() + deliveryRate, deliveryTimer);
    }
    else if (SMTPCommand *cmd = dynamic_cast<SMTPCommand *>(msg))
    {
        currentClientGateIndex = msg->getArrivalGate()->getIndex();

        switch (cmd->getCommandType())
        {
        case CMD_HELO:
            handleHeloCommand(cmd);
            break;
        case CMD_MAIL_FROM:
            handleMailFromCommand(cmd);
            break;
        case CMD_RCPT_TO:
            handleRcptToCommand(cmd);
            break;
        case CMD_QUIT:
            handleQuitCommand(cmd);
            break;
        default:
            sendResponse(RESP_500_ERROR, "Command not recognized");
            break;
        }
        delete msg;
    }
    else if (MailData *mailData = dynamic_cast<MailData *>(msg))
    {
        currentClientGateIndex = msg->getArrivalGate()->getIndex();

        handleDataCommand(mailData);
        delete msg;
    }
    else
    {
        delete msg;
    }
}

void ServerModule::handleHeloCommand(SMTPCommand *cmd)
{
    EV << "Received HELO from: " << cmd->getCommandData() << endl;

    state = CONNECTED;
    clientSequenceNumber = cmd->getSequenceNumber();

    // Send response with public key
    sendResponse(RESP_250_OK, (std::string("Hello ") + cmd->getCommandData()).c_str(),
                 encryptionHelper.getPublicKeyString().c_str());
}

void ServerModule::handleMailFromCommand(SMTPCommand *cmd)
{
    EV << "Received MAIL FROM: " << cmd->getCommandData() << endl;

    currentSender = cmd->getCommandData();
    state = MAIL_FROM_RECEIVED;

    sendResponse(RESP_250_OK, "Sender OK");
}

void ServerModule::handleRcptToCommand(SMTPCommand *cmd)
{
    EV << "Received RCPT TO: " << cmd->getCommandData() << endl;

    currentRecipient = cmd->getCommandData();
    state = RCPT_TO_RECEIVED;

    sendResponse(RESP_354_START_INPUT, "Start mail input; end with <CRLF>.<CRLF>");
}

void ServerModule::handleDataCommand(MailData *mailData)
{
    EV << "Received mail data from: " << mailData->getSender() << endl;

    emit(mailReceivedSignal, 1L);

    // Create queued mail
    QueuedMail *queuedMail = new QueuedMail();
    queuedMail->setSender(mailData->getSender());
    queuedMail->setRecipient(mailData->getRecipient());
    queuedMail->setSubject(mailData->getSubject());
    queuedMail->setPriority(mailData->getPriority());
    queuedMail->setArrivalTime(simTime());
    queuedMail->setMailId(nextMailId++);

    if (mailData->isEncrypted())
    {
        std::string decrypted = encryptionHelper.decrypt(mailData->getEncryptedBody());
        if (decrypted.empty())
        {
            EV << "Decryption failed!" << endl;
            delete queuedMail;
            emit(mailRejectedSignal, 1L);
            sendResponse(RESP_550_REJECTED, "Decryption failed");
            state = WAITING;
            return;
        }
        queuedMail->setDecryptedBody(decrypted.c_str());
        EV << "Mail decrypted successfully" << endl;
    }
    else
    {
        queuedMail->setDecryptedBody(mailData->getEncryptedBody());
    }

    std::string rejectReason;
    if (enableFiltering && !filterMail(queuedMail, rejectReason))
    {
        EV << "Mail rejected: " << rejectReason << endl;
        delete queuedMail;
        emit(mailRejectedSignal, 1L);
        sendResponse(RESP_550_REJECTED, rejectReason.c_str());
        state = WAITING;
        return;
    }

    int totalQueueSize = highPriorityQueue.size() + lowPriorityQueue.size();
    if (totalQueueSize >= maxQueueSize)
    {
        EV << "Queue full, mail rejected" << endl;
        delete queuedMail;
        emit(mailRejectedSignal, 1L);
        sendResponse(RESP_550_REJECTED, "Server queue full");
        state = WAITING;
        return;
    }

    enqueueMail(queuedMail);
    emit(mailAcceptedSignal, 1L);

    sendResponse(RESP_250_OK, "Mail accepted for delivery");
    state = WAITING;
}

void ServerModule::handleQuitCommand(SMTPCommand *cmd)
{
    EV << "Received QUIT" << endl;

    sendResponse(RESP_221_BYE, "Goodbye");
    state = WAITING;

    // Reset session
    currentSender = "";
    currentRecipient = "";
}

bool ServerModule::filterMail(QueuedMail *mail, std::string &rejectReason)
{
    if (requireSubject && strlen(mail->getSubject()) == 0)
    {
        rejectReason = "Subject header required";
        return false;
    }

    if (strlen(mail->getDecryptedBody()) == 0)
    {
        rejectReason = "Empty message body";
        return false;
    }

    if (mail->getPriority() != PRIORITY_HIGH && mail->getPriority() != PRIORITY_LOW)
    {
        rejectReason = "Invalid priority header";
        return false;
    }

    // All checks passed
    return true;
}

void ServerModule::enqueueMail(QueuedMail *mail)
{
    if (mail->getPriority() == PRIORITY_HIGH)
    {
        highPriorityQueue.push_back(mail);
        EV << "Mail enqueued to HIGH priority queue (size: " << highPriorityQueue.size() << ")" << endl;
    }
    else
    {
        lowPriorityQueue.push_back(mail);
        EV << "Mail enqueued to LOW priority queue (size: " << lowPriorityQueue.size() << ")" << endl;
    }

    updateQueueStatistics();
}

void ServerModule::processDeliveryQueue()
{
    QueuedMail *mailToDeliver = nullptr;

    // In constrained mode, prioritize high priority queue
    if (constrainedMode)
    {
        if (!highPriorityQueue.empty())
        {
            mailToDeliver = highPriorityQueue.front();
            highPriorityQueue.pop_front();
        }
        else if (!lowPriorityQueue.empty())
        {
            mailToDeliver = lowPriorityQueue.front();
            lowPriorityQueue.pop_front();
        }
    }
    else
    {
        // In normal mode, alternate between queues fairly
        if (!highPriorityQueue.empty())
        {
            mailToDeliver = highPriorityQueue.front();
            highPriorityQueue.pop_front();
        }
        else if (!lowPriorityQueue.empty())
        {
            mailToDeliver = lowPriorityQueue.front();
            lowPriorityQueue.pop_front();
        }
    }

    if (mailToDeliver != nullptr)
    {
        deliverMail(mailToDeliver);
    }

    updateQueueStatistics();
}

void ServerModule::deliverMail(QueuedMail *mail)
{
    simtime_t latency = simTime() - mail->getArrivalTime();

    EV << "Delivering mail ID " << mail->getMailId()
       << " (Priority: " << (mail->getPriority() == PRIORITY_HIGH ? "HIGH" : "LOW")
       << ", Latency: " << latency << ")" << endl;

    emit(mailDeliveredSignal, 1L);
    emit(deliveryLatencySignal, latency);

    if (mail->getPriority() == PRIORITY_HIGH)
    {
        emit(highPriorityDeliveredSignal, 1L);
        emit(highPriorityLatencySignal, latency);
    }
    else
    {
        emit(lowPriorityDeliveredSignal, 1L);
        emit(lowPriorityLatencySignal, latency);
    }

    delete mail;
}

void ServerModule::sendResponse(int responseCode, const char *message, const char *extraData)
{
    SMTPResponse *response = new SMTPResponse("SMTP_RESPONSE");
    response->setResponseCode(responseCode);
    response->setResponseMessage(message);

    if (extraData != nullptr)
    {
        response->setPublicKey(extraData);
    }

    response->setSequenceNumber(++clientSequenceNumber);

    if (responseCode == RESP_250_OK || responseCode == RESP_220_READY ||
        responseCode == RESP_354_START_INPUT || responseCode == RESP_221_BYE)
    {
        response->addPar("color") = "green";
        response->addPar("label") = "OK";
    }
    else
    {
        response->addPar("color") = "red";
        response->addPar("label") = "ERROR";
    }

    if (currentClientGateIndex >= 0)
    {
        send(response, "port$o", currentClientGateIndex);
    }
}

void ServerModule::updateQueueStatistics()
{
    int totalSize = highPriorityQueue.size() + lowPriorityQueue.size();
    emit(queueSizeSignal, (long)totalSize);
    emit(highPriorityQueueSizeSignal, (long)highPriorityQueue.size());
    emit(lowPriorityQueueSizeSignal, (long)lowPriorityQueue.size());
}

void ServerModule::finish()
{
    EV << "Server finishing" << endl;
    EV << "Remaining high priority mails: " << highPriorityQueue.size() << endl;
    EV << "Remaining low priority mails: " << lowPriorityQueue.size() << endl;
}
