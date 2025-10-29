package emtp;

simple ServerModule
{
    parameters:
        @display("i=device/server2;i2=status/execute");
        
        // Server configuration
        string serverName = default("mail.server.com");
        
        // Queue processing parameters
        double processingDelay @unit(s) = default(0.1s);  // Time to process one mail
        double deliveryRate @unit(s) = default(1s);  // How often to deliver mails from queue
        
        // Capacity constraints
        int maxQueueSize = default(100);  // Maximum combined queue size
        bool constrainedMode = default(false);  // Enable constrained/bandwidth-limited mode
        
        // Filtering rules
        bool enableFiltering = default(true);
        bool requireSubject = default(true);
        bool requireEncryption = default(false);
        
        // Encryption
        int encryptionSeed = default(12345);  // Seed for key generation
        
        // Statistics
        @signal[mailReceived](type=long);
        @signal[mailAccepted](type=long);
        @signal[mailRejected](type=long);
        @signal[mailDelivered](type=long);
        @signal[highPriorityDelivered](type=long);
        @signal[lowPriorityDelivered](type=long);
        @signal[queueSize](type=long);
        @signal[highPriorityQueueSize](type=long);
        @signal[lowPriorityQueueSize](type=long);
        @signal[deliveryLatency](type=simtime_t);
        @signal[highPriorityLatency](type=simtime_t);
        @signal[lowPriorityLatency](type=simtime_t);
        
        @statistic[mailsReceived](title="Mails Received"; source=mailReceived; record=count,sum);
        @statistic[mailsAccepted](title="Mails Accepted"; source=mailAccepted; record=count,sum);
        @statistic[mailsRejected](title="Mails Rejected"; source=mailRejected; record=count,sum);
        @statistic[mailsDelivered](title="Mails Delivered"; source=mailDelivered; record=count,sum);
        @statistic[highPriorityDelivered](title="High Priority Delivered"; source=highPriorityDelivered; record=count,sum);
        @statistic[lowPriorityDelivered](title="Low Priority Delivered"; source=lowPriorityDelivered; record=count,sum);
        @statistic[totalQueueSize](title="Total Queue Size"; source=queueSize; record=vector,mean,max);
        @statistic[highPriorityQueue](title="High Priority Queue Size"; source=highPriorityQueueSize; record=vector,mean,max);
        @statistic[lowPriorityQueue](title="Low Priority Queue Size"; source=lowPriorityQueueSize; record=vector,mean,max);
        @statistic[deliveryLatency](title="Delivery Latency"; source=deliveryLatency; record=vector,mean,max; unit=s);
        @statistic[highPriorityLatency](title="High Priority Latency"; source=highPriorityLatency; record=vector,mean,max; unit=s);
        @statistic[lowPriorityLatency](title="Low Priority Latency"; source=lowPriorityLatency; record=vector,mean,max; unit=s);
        
    gates:
        inout port[];
}
