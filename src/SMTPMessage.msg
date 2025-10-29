enum SMTPCommandType {
    CMD_HELO = 1;
    CMD_MAIL_FROM = 2;
    CMD_RCPT_TO = 3;
    CMD_DATA = 4;
    CMD_QUIT = 5;
}

enum SMTPResponseCode {
    RESP_220_READY = 220;
    RESP_250_OK = 250;
    RESP_354_START_INPUT = 354;
    RESP_221_BYE = 221;
    RESP_500_ERROR = 500;
    RESP_550_REJECTED = 550;
}

enum MailPriority {
    PRIORITY_LOW = 0;
    PRIORITY_HIGH = 1;
}


packet SMTPMessage {
    int sequenceNumber;
}

packet SMTPCommand extends SMTPMessage {
    int commandType @enum(SMTPCommandType);
    string commandData;
}

packet SMTPResponse extends SMTPMessage {
    int responseCode @enum(SMTPResponseCode);
    string responseMessage;
    string publicKey;
}

packet MailData extends SMTPMessage {
    string sender;
    string recipient;
    string subject;
    int priority @enum(MailPriority);
    string encryptedBody;
    bool isEncrypted;
    simtime_t timestamp;
}

message QueuedMail {
    string sender;
    string recipient;
    string subject;
    int priority @enum(MailPriority);
    string decryptedBody;
    simtime_t arrivalTime;
    int mailId;
}
