#ifndef PHYSICS_H
#define PHYSICS_H

namespace RiverCore{

    //Class to get gravity
    class Physics{
        public:
            //Set gravity
            void setPhysics(const float gravity) {gravityAmount = gravity;}
            //Get gravity
            float getPhysics() {return gravityAmount;}

        private:
            //Gravity float amount
            static float gravityAmount;
    };
}


#endif