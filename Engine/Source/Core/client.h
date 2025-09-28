#ifdef CLIENT.H
#define CLIENT.H
namespace RiverCore{
    class client {
        public:
            void initNetwork();
            void shutdownNetwork();

            // send my movement update
            void sendVel(int playerId, int velx, int vely);

            //Handle updates
            bool update(int& playerId, int& velx, int& vely);
    }
}


#endif
