package emtp;

simple ClientModule
{
    parameters:
        @display("i=device/laptop;i2=status/execute");
        
        // Client configuration
        string clientName = default("client1");
        string mailFrom = default("sender@example.com");
        string mailTo = default("receiver@example.com");
        string subject = default("Test Email");
        string messageBody = default("This is a test message.");
        
        // Priority configuration (HIGH or LOW)
        string priority = default("LOW");
        
        // Enable/disable encryption
        bool useEncryption = default(true);
        
        // Timing parameters
        double startTime @unit(s) = default(1s);
        
        // Statistics
        @signal[mailSent](type=long);
        @signal[mailAccepted](type=long);
        @signal[mailRejected](type=long);
        @signal[roundTripTime](type=simtime_t);
        
        @statistic[mailsSent](title="Mails Sent"; source=mailSent; record=count,sum);
        @statistic[mailsAccepted](title="Mails Accepted"; source=mailAccepted; record=count,sum);
        @statistic[mailsRejected](title="Mails Rejected"; source=mailRejected; record=count,sum);
        @statistic[rtt](title="Round Trip Time"; source=roundTripTime; record=vector,mean,max; unit=s);
        
    gates:
        inout port;
}
