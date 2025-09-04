#ifndef PHYSICS_H
#define PHYSICS_H

namespace RiverCore{

    //Class to get gravity
    class Physics{


        public:
            Physics(){};
            ~Physics(){};
            //Set gravity
            void setGravity(const float gravity) {gravityAmount = gravity;}
            //Get gravity
            float getGravity() {return gravityAmount;}

        private:
            //Gravity float amount
            float gravityAmount;
    };
}


#endif