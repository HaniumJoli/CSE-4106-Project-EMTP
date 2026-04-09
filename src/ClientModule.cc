#include "ClientModule.h"

ClientModule::ClientModule()
{
    state = IDLE;
    sequenceNumber = 0;
}

ClientModule::~ClientModule()
{
}

void ClientModule::initialize()
{
    clientName = par("clientName").stdstringValue();
    mailFrom = par("mailFrom").stdstringValue();
    mailTo = par("mailTo").stdstringValue();
    subject = par("subject").stdstringValue();
    messageBody = par("messageBody").stdstringValue();
    priority = par("priority").stdstringValue();
    useEncryption = par("useEncryption").boolValue();
    startTime = par("startTime");

    mailSentSignal = registerSignal("mailSent");
    mailAcceptedSignal = registerSignal("mailAccepted");
    mailRejectedSignal = registerSignal("mailRejected");
    roundTripTimeSignal = registerSignal("roundTripTime");

    cMessage *startMsg = new cMessage("start");
    scheduleAt(startTime, startMsg);

    EV << "Client initialized: " << clientName << endl;
}

void ClientModule::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        // Start communication
        if (strcmp(msg->getName(), "start") == 0)
        {
            state = WAIT_HELO_RESPONSE;
            sessionStartTime = simTime();
            sendHelo();
            delete msg;
        }
    }
    else
    {
        // Response from server
        if (SMTPResponse *response = dynamic_cast<SMTPResponse *>(msg))
        {
            switch (state)
            {
            case WAIT_HELO_RESPONSE:
                handleHeloResponse(response);
                break;
            case WAIT_MAIL_FROM_RESPONSE:
                handleMailFromResponse(response);
                break;
            case WAIT_RCPT_TO_RESPONSE:
                handleRcptToResponse(response);
                break;
            case WAIT_DATA_RESPONSE:
                handleDataResponse(response);
                break;
            case WAIT_QUIT_RESPONSE:
                handleQuitResponse(response);
                break;
            default:
                EV << "Unexpected response in state " << state << endl;
                break;
            }
            delete msg;
        }
        else
        {
            delete msg;
        }
    }
}

void ClientModule::sendHelo()
{
    EV << "Sending HELO command" << endl;

    SMTPCommand *cmd = new SMTPCommand("HELO");
    cmd->setCommandType(CMD_HELO);
    cmd->setCommandData(clientName.c_str());
    cmd->setSequenceNumber(++sequenceNumber);
    cmd->addPar("color") = "green";
    cmd->addPar("label") = "HELO";

    send(cmd, "port$o");
}

void ClientModule::handleHeloResponse(SMTPResponse *msg)
{
    EV << "Received HELO response: " << msg->getResponseCode() << " - "
       << msg->getResponseMessage() << endl;

    if (msg->getResponseCode() == RESP_250_OK)
    {
        // Get public key from server
        std::string pubKey = msg->getPublicKey();
        if (useEncryption && !pubKey.empty())
        {
            encryptionHelper.setPublicKeyFromString(pubKey);
            EV << "Received public key: " << pubKey << endl;
        }

        // Proceed to MAIL FROM
        state = WAIT_MAIL_FROM_RESPONSE;
        sendMailFrom();
    }
    else
    {
        EV << "HELO failed, terminating" << endl;
        state = DONE;
    }
}

void ClientModule::sendMailFrom()
{
    EV << "Sending MAIL FROM command" << endl;

    SMTPCommand *cmd = new SMTPCommand("MAIL_FROM");
    cmd->setCommandType(CMD_MAIL_FROM);
    cmd->setCommandData(mailFrom.c_str());
    cmd->setSequenceNumber(++sequenceNumber);
    cmd->addPar("color") = "blue";
    cmd->addPar("label") = "MAIL FROM";

    send(cmd, "port$o");
}

void ClientModule::handleMailFromResponse(SMTPResponse *msg)
{
    EV << "Received MAIL FROM response: " << msg->getResponseCode() << endl;

    if (msg->getResponseCode() == RESP_250_OK)
    {
        state = WAIT_RCPT_TO_RESPONSE;
        sendRcptTo();
    }
    else
    {
        EV << "MAIL FROM failed, terminating" << endl;
        state = DONE;
    }
}

void ClientModule::sendRcptTo()
{
    EV << "Sending RCPT TO command" << endl;

    SMTPCommand *cmd = new SMTPCommand("RCPT_TO");
    cmd->setCommandType(CMD_RCPT_TO);
    cmd->setCommandData(mailTo.c_str());
    cmd->setSequenceNumber(++sequenceNumber);
    cmd->addPar("color") = "cyan";
    cmd->addPar("label") = "RCPT TO";

    send(cmd, "port$o");
}

void ClientModule::handleRcptToResponse(SMTPResponse *msg)
{
    EV << "Received RCPT TO response: " << msg->getResponseCode() << endl;

    if (msg->getResponseCode() == RESP_250_OK || msg->getResponseCode() == RESP_354_START_INPUT)
    {
        state = WAIT_DATA_RESPONSE;
        sendData();
    }
    else
    {
        EV << "RCPT TO failed, terminating" << endl;
        state = DONE;
    }
}

void ClientModule::sendData()
{
    EV << "Sending DATA" << endl;

    MailData *mailMsg = new MailData("MAIL_DATA");
    mailMsg->setSender(mailFrom.c_str());
    mailMsg->setRecipient(mailTo.c_str());
    mailMsg->setSubject(subject.c_str());

    // Set priority
    if (priority == "HIGH")
    {
        mailMsg->setPriority(PRIORITY_HIGH);
    }
    else
    {
        mailMsg->setPriority(PRIORITY_LOW);
    }

    // Encrypt body if enabled
    if (useEncryption && encryptionHelper.isInitialized())
    {
        std::string encrypted = encryptionHelper.encrypt(messageBody);
        mailMsg->setEncryptedBody(encrypted.c_str());
        mailMsg->setIsEncrypted(true);
        EV << "Message encrypted" << endl;
    }
    else
    {
        mailMsg->setEncryptedBody(messageBody.c_str());
        mailMsg->setIsEncrypted(false);
    }

    mailMsg->setTimestamp(simTime());
    mailMsg->setSequenceNumber(++sequenceNumber);

    // Set color based on priority
    if (priority == "HIGH")
    {
        mailMsg->addPar("color") = "red";
        mailMsg->addPar("label") = "DATA (HIGH)";
    }
    else
    {
        mailMsg->addPar("color") = "yellow";
        mailMsg->addPar("label") = "DATA (LOW)";
    }

    emit(mailSentSignal, 1L);
    send(mailMsg, "port$o");
}

void ClientModule::handleDataResponse(SMTPResponse *msg)
{
    EV << "Received DATA response: " << msg->getResponseCode() << endl;

    if (msg->getResponseCode() == RESP_250_OK)
    {
        EV << "Mail accepted" << endl;
        emit(mailAcceptedSignal, 1L);
    }
    else
    {
        EV << "Mail rejected: " << msg->getResponseMessage() << endl;
        emit(mailRejectedSignal, 1L);
    }

    // Calculate round trip time
    simtime_t rtt = simTime() - sessionStartTime;
    emit(roundTripTimeSignal, rtt);

    // Send QUIT
    state = WAIT_QUIT_RESPONSE;
    sendQuit();
}

void ClientModule::sendQuit()
{
    EV << "Sending QUIT command" << endl;

    SMTPCommand *cmd = new SMTPCommand("QUIT");
    cmd->setCommandType(CMD_QUIT);
    cmd->setSequenceNumber(++sequenceNumber);
    cmd->addPar("color") = "gray";
    cmd->addPar("label") = "QUIT";

    send(cmd, "port$o");
}

void ClientModule::handleQuitResponse(SMTPResponse *msg)
{
    EV << "Received QUIT response: " << msg->getResponseCode() << endl;
    EV << "Session completed" << endl;
    state = DONE;
}

void ClientModule::finish()
{
    EV << "Client finishing" << endl;
}
