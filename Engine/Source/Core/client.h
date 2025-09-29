#ifndef CLIENT_H
#define CLIENT_H

namespace RiverCore {
    class Client {
        public:
            Client() = default;


            //Initalizes the network
            void initPorts();
            void shutdownPorts();

            // Send velocity update
            void sendVel(uint32_t playerId, float velx, float vely);

            //Handle updates
            bool update(uint32_t& playerId, float& velx, float& vely);
    };
}


#endif
