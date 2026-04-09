//
// EMTP Network - Simple client-server topology
//
package emtp.simulations;

import emtp.ClientModule;
import emtp.ServerModule;

channel Channel extends ned.DatarateChannel
{
    parameters:
        datarate = default(1Mbps);
        delay = default(10ms);
        ber = default(0);  // bit error rate
        @display("ls=blue,3");
}

network EMTPNetwork
{
    parameters:
        int numClients = default(1);
        @display("bgb=1500,1000;bgi=background/terrain,s");

    submodules:
        client[numClients]: ClientModule {
            @display("p=899,368,ring,300;i=device/laptop");
        }

        server: ServerModule {
            @display("p=750,500;i=device/server2,gold;is=l");
            gates:
                port[parent.numClients];
        }

    connections:
        for i=0..numClients-1 {
            client[i].port <--> Channel <--> server.port[i];
        }
}
